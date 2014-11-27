#include "log.h"
#include "mutatee.h"
#include "trial-run.h"
#include "utils.h"

#include <iomanip>
#include <signal.h>
#include <unistd.h>

/* global variable to determine if signal handler has been set. */
static bool handler_initialized = false;

/* global variable containing mutatee pid. */
static int mutatee_pid;

/* global variable with the timeout length. */
static int timeout = 0;

/* global variable to contain state of trial. */
static int trial_state = trial_run::TBEG;

/* alarm handler. */
static void alarm_handler(int num) {
    switch (trial_state) {
        case trial_run::TRUN:
            log::info() << "trial timeout reached." << std::endl;
            kill(mutatee_pid, SIGTERM);
            trial_state = trial_run::TTRM;
            alarm(timeout);
        break;
        case trial_run::TTRM:
            log::info() << "killing non-responsive mutatee." << std::endl;
            kill(mutatee_pid, SIGKILL);
            trial_state = trial_run::TKIL;
        break;
        case trial_run::TKIL:
        case trial_run::TBEG:
        case trial_run::TEND:
            log::debug() << "unspecified alarm action in state " << trial_state 
                         << "." << std::endl;
    }
}

/* registers the signal handler for alarms. */
static bool register_alarm_handler() {
    struct sigaction handler;
    
    handler.sa_handler = alarm_handler;
    sigemptyset(&handler.sa_mask);
    handler.sa_flags = SA_RESTART;
    
    return sigaction(SIGALRM, &handler, nullptr) == 0 ? true : false;
}

/* sets the alarm for the proper timeout. */
static void set_timeout() {
    if (timeout == 0) { return; }
    alarm(timeout);
}

trial_run::trial_run(int id, mutatee& prog, mutatee::parameters& mut_params, 
                     trial_run::parameters& run_params) {
    int in  = stdin;
    int out = stdout;
    int err = stderr;
    int status;
    
    // initialize trial state.
    trial_state = TBEG;
    
    // register handler.
    if (!handler_initialized) {
        if (!register_alarm_handler()) {
            log::debug() << "could not register alarm handler." << std::endl;
            goto error;
        } else {
            handler_initialized = true;
            log::info() << "registered alarm handler." << std::endl;
        }
    }
    
    log::info() << "beginning trial " << id << "." << std::endl;
    
    // open input stream if splitting/redirecting prog input.
    if (run_params.redirect_mode & SPLIT_IN) {
        in = utils::open_for_read(run_params.input_file);
        if (in == -1) { goto error; }
    }
    
    // open output stream if splitting/redirecting prog output.
    if (run_params.redirect_mode & SPLIT_OUT) {
        std::string name = utils::create_file_name(run_params.output_root, 
                                                   utils::OUT, id);
        out = utils::open_for_write(name);
        if (out == -1) { goto error; }
    }
    
    // open error stream if splitting/redirecting prog error.
    if (run_params.redirect_mode & SPLIT_ERR) {
        std::string name = utils::create_file_name(run_params.output_root, 
                                                   utils::ERR, id);
        err = utils::open_for_write(name);
        if (err == -1) { goto error; }
    }
    
    // create new process with correct streams.
    if (!prog.create_process(in, out, err)) { goto error; }
    
    // load runtime library.
    if (!prog.load_runtime("libdynamo-rt.so")) { goto error; }
    
    // find delay function.
    if (!prog.find_delay_function("libdynamo-rt.so", "std_delay")) {
        goto error;
    }
    
    // perform memory instrumentation.
    if (run_params.instrument_memory) { 
        if (!prog.instrument_memory(mut_params)) { goto error; }
    }
    
    // perform mutex instrumentation.
    if (run_params.instrument_mutex) { 
        if (!prog.instrument_mutex(mut_params)) { goto error; }
    }
    
    // prepare trial for execution.
    mutatee_pid = prog.get_pid();
    trial_state = TRUN;
    timeout = run_params.timeout;
    set_timeout();
    
    // execute mutatee.
    status = prog.execute();
    
    // clear pending alarm if any.
    alarm(0);
    
    // check it succeeded.
    if (status == mutatee::EXEC_FAIL) { goto error; }
    
    // log result.
    switch (trial_state) {
        case TRUN:
            if (status > 128) {
                ++score_crash;
                log::dynamo() << "trial " << id << " crashed under signal ("
                            << status - 128 << ")." << std::endl;
            } else {
                ++score_success;
                log::dynamo() << "trial " << id << " completed successfully ("
                            << status << ")." << std::endl;
            }
        break;
        case TTRM:
            ++score_term;
            log::dynamo() << "trial " << id << " terminated due to timeout."
                        << std::endl;
        break;
        case TKIL:
            ++score_kill;
            log::dynamo() << "trial " << id << " killed due to no response."
                        << std::endl;
        break;
    }
    
    // clear state.
    trial_state = TEND;
                    
    // close open streams.
    if (run_params.redirect_mode & SPLIT_IN)  { utils::close(in); }
    if (run_params.redirect_mode & SPLIT_OUT) { utils::close(out); }
    if (run_params.redirect_mode & SPLIT_ERR) { utils::close(err); }
    
    return;
    
    // error handler.
    error:
    log::error() << "could not execute " 
                 << "trial " << id << "." << std::endl;
    if (run_params.redirect_mode & SPLIT_IN)  { utils::close(in); }
    if (run_params.redirect_mode & SPLIT_OUT) { utils::close(out); }
    if (run_params.redirect_mode & SPLIT_ERR) { utils::close(err); }
    prog.destroy_process();
}

int trial_run::mode(const std::string& in, bool out, bool err) {
    int m = 0;
    
    // add SPLIT_IN if in has not been set.
    if (in != "") { m += SPLIT_IN; }
    
    // add SPLIT_OUT if out is true.
    if (out) { m += SPLIT_OUT; }
    
    // add SPLIT_ERR if err is true.
    if (err) { m += SPLIT_ERR; }
    
    //log::info() << "mode for `" << in << "', " << out << ", " << err << ": " <<
    //                m << std::endl;
    
    return m;
}

void trial_run::report() {
    int total = score_success + score_term + score_crash + score_kill;
    
    log::dynamo() << "                ---  SCORE REPORT  ---  " << std::endl
        << "         " << std::setw(4) << std::left << score_success 
        << " successes                    " << std::setw(4) << score_crash 
        << " crashes" << std::endl
        << "         " << std::setw(4) << score_term    
        << " timeout terminations         " << std::setw(4) << score_kill
        << " non-responsive kills" << std::endl
        << "         " << std::setw(4) << total    
        << " total trials                 " << std::setw(4) << score_fail
        << " callback failures" << std::endl;
}

int trial_run::score_success = 0;
int trial_run::score_term = 0;
int trial_run::score_crash = 0;
int trial_run::score_kill = 0;
int trial_run::score_fail = 0;
