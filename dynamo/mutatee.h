#ifndef MUTATEE_H
#define MUTATEE_H

#include <string>

/* class for representing, running, and manipulating mutatee program. */
class mutatee {
  public:
    struct parameters {
        int delay_lower_bound;
        int delay_upper_bound;
        int delay_probability;
        int instrument_probability;
        int (*get_random_int)();
    };
    
    mutatee(char**, char**);
    
    bool create_process(int, int, int);
    
    bool destroy_process();
    
    int execute();
    
    bool find_delay_function(const std::string&, const std::string&);
    
    bool instrument_memory(const parameters&);
    
    bool instrument_mutex(const parameters&);
    
    bool load_runtime(const std::string&);
    
    // find module / delay fun
    static const int EXEC_FAIL = 65535;
    
  private:
    struct mdata;
    mdata* data;
};

#endif
