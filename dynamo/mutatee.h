#ifndef MUTATEE_H
#define MUTATEE_H

#include <string>

/* class for representing, running, and manipulating mutatee program. */
class mutatee {
  public:
    /* parameters to this class. */
    struct parameters {
        int delay_lower_bound;
        int delay_upper_bound;
        int delay_probability;
        int instrument_probability;
        int (*get_random_int)();
    };
    
    /* constructs a mutatee with argv and envp variables. */
    mutatee(char**, char**);
    
    /* creates a new process for the mutatee with specified streams in dynamo
       as the child's standard streams. */
    bool create_process(int, int, int);
    
    /* destroys the mutatee process. */
    bool destroy_process();
    
    /* executes the mutatee process and returns the exit status. */
    int execute();
    
    /* finds a given delay function in the mutatee process. */
    bool find_delay_function(const std::string&, const std::string&);
    
    /* instruments the memory accesses in the mutatee process. */
    bool instrument_memory(const parameters&);
    
    /* instruments the mutexes (unlock) in the mutatee process. */
    bool instrument_mutex(const parameters&);
    
    /* loads the runtime library into the mutatee process. */
    bool load_runtime(const std::string&);
    
    /* constant returned by execute it it failed. */
    static const int EXEC_FAIL = 65535;
    
  private:
    struct mdata;
    mdata* data;
};

#endif
