#include "log.h"
#include "mutatee.h"
#include "trial-run.h"
#include "utils.h"

trial_run::trial_run(int id, mutatee& prog, mutatee::parameters& mut_params, 
                     trial_run::parameters& run_params) {
    int in  = stdin;
    int out = stdout;
    int err = stderr;
    int status;
    
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
    
    // execute mutatee.
    status = prog.execute();
    
    // check it succeeded.
    if (status == mutatee::EXEC_FAIL) { goto error; }
    
    // log result.
    log::alert() << "run " << id << " exited with "
                    "status " << status << "." << std::endl;
                    
    // close open streams.
    if (run_params.redirect_mode & SPLIT_IN)  { utils::close(in); }
    if (run_params.redirect_mode & SPLIT_OUT) { utils::close(out); }
    if (run_params.redirect_mode & SPLIT_ERR) { utils::close(err); }
    
    return;
    
    // error handler.
    error:
    log::error() << "could not execute " 
                 << "run " << id << "." << std::endl;
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
