
#include "log.h"
#include "mutatee.h"
#include "parse-args.h"
#include "printer.h"
#include "random.h"
#include "trial-run.h"

/* main function for dynamo fuzz tester. */
int main(int argc, char** argv, char** envp) {
    // parse the arguments. 
    parse args(argc, argv);
    log::info() << "parsed command-line arguments." << std::endl;
    
    // print version if requested.
    if (args.display_version()) { printer::version(); }
    
    // print help if requested.
    if (args.display_help()) { printer::help(argv[0]); }

    // stop if requested.
    if (!args.continue_run()) { return 0; }
    
    // create mutatee.
    mutatee prog(args.mutatee_arguments(), envp);
    log::info() << "created mutatee object." << std::endl;
    
    // initialize for instrumentation.
    mutatee::parameters mutatee_params {
        .delay_lower_bound = args.delay_lower(),
        .delay_upper_bound = args.delay_upper(),
        .delay_probability = args.delay_probability(),
        .instrument_probability = args.instrument_probability(),
        .get_random_int = random::uniform_probability
    };
    
    trial_run::parameters run_params {
        .output_root = args.output_root(),
        .input_file = args.input_file(),
        .timeout = args.timeout(),
        .redirect_mode = trial_run::mode(args.input_file(), 
                                         !args.merge_out(), !args.merge_err()),
        .instrument_memory = args.instrument_memory(),
        .instrument_mutex = args.instrument_mutex()
    };
    
    log::info() << "initialized mutatee and run parameters." << std::endl;
    
    // run each trial.
    for (int i = 0; i < args.number_trials(); ++i) {
        trial_run(i, prog, mutatee_params, run_params);
    }
    
    return 0;
}
