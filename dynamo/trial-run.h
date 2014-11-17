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
    
    /* stream identifiers. */
    static const int stdin = 0,
                     stdout = 1,
                     stderr = 2;
    
    /* constants for redirect mode. streams are split if these are in the mode. */                 
    static const int SPLIT_IN  = 1,
                     SPLIT_OUT = 2,
                     SPLIT_ERR = 4;
    
  private:
};

#endif
