#ifndef TRIAL_RUN_H
#define TRIAL_RUN_H

#include <string>

class mutatee;
//class mutatee::parameters;

/* class for representing each run of a mutatee. */
class trial_run {
  public:
    /* parameters that are passed to the run. */
    struct parameters {
        std::string output_root;
        std::string input_file;
        int timeout;
        int redirect_mode;
        bool instrument_memory;
        bool instrument_mutex;
    };
    
    /* constructor. performs the actual run. */
    trial_run(int, mutatee&, mutatee::parameters&, trial_run::parameters&);
    
    /* helper function that forms the redirect mode setting for streams. */
    static int mode(const std::string&, bool, bool);
    
    /* reports the results of all trials up to this point. */
    static void report();

    /* stream identifiers. */
    static const int stdin = 0,
                     stdout = 1,
                     stderr = 2;
    
    /* constants for redirect mode. streams are split if these are in the mode. */                 
    static const int SPLIT_IN  = 1,
                     SPLIT_OUT = 2,
                     SPLIT_ERR = 4;
    
    /* constants for trial state. */
    static const int TBEG = 0,
                     TRUN = 1,
                     TTRM = 2,
                     TKIL = 3,
                     TEND = 4;
    
  private:
    /* score card variables. */
    static int score_success,
               score_term,
               score_crash,
               score_kill,
               score_fail;
};

#endif
