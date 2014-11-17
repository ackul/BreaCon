#ifndef TRIAL_RUN_H
#define TRIAL_RUN_H

#include <string>

class mutatee;
class mutatee::parameters;

class trial_run {
  public:
    struct parameters {
        std::string output_root;
        std::string input_file;
        int timeout;
        int redirect_mode;
        bool instrument_memory;
        bool instrument_mutex;
    };
    
    trial_run(int, mutatee&, mutatee::parameters&, trial_run::parameters&);
    
    static int mode(const std::string&, bool, bool);
    
    static const int stdin = 0,
                     stdout = 1,
                     stderr = 2;
                     
    static const int SPLIT_IN  = 1,
                     SPLIT_OUT = 2,
                     SPLIT_ERR = 4;
    
  private:
};

#endif
