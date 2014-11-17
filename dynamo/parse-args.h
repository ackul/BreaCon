#ifndef PARSE_ARGS_H
#define PARSE_ARGS_H

#include <string>

/* class to handle parsing args. */
class parse {
  public:
    /* constructs an immutable object by parsing the arguments. */
    parse(int, char**);
    
    /* returns true if the program should continue running after parsing. */
    bool continue_run() const;
    
    /* returns the log level. */
    int debug_level() const;
    
    /* returns true if help should be displayed. */
    bool display_help() const;
    
    /* returns true if the version should be displayed. */
    bool display_version() const;
    
    /* returns the lower bound for delay times. */
    int delay_lower() const;
    
    /* returns the probability of delaying. */
    int delay_probability() const;
    
    /* returns the upper bound on delay times. */
    int delay_upper() const;
    
    /* returns the input file name for the mutatee. */
    std::string input_file() const;
    
    /* returns true if memory accesses should be instrumented. */
    bool instrument_memory() const;
    
    /* returns true if mutexes should be instrumented. */
    bool instrument_mutex() const;
    
    /* returns the probability of instrumenting a point. */
    int instrument_probability() const;
    
    /* return true if the mutatee's error stream should be merged. */
    bool merge_err() const;
    
    /* return true if the mutatee's output stream should be merged. */
    bool merge_out() const;
    
    /* returns the arguments to pass to the mutatee. */
    char** mutatee_arguments() const;
    
    /* returns the number of trials to run the mutatee. */
    int number_trials() const;
    
    /* returns the root file name for mutatee output files. */
    std::string output_root() const;
    
    /* returns the timeout to run mutatee. */
    int timeout() const;
    
    /* different arguments that may be passed to the program. */
    static const char HELP = 'h',
                      VERS = 'v',
                      LOWR = 'l',
                      UPPR = 'u',
                      DELY = 'd',
                      PPRB = 'p',
                      IPRB = 'i',
                      IMEM = 'a',
                      IMUT = 'm',
                      IFIL = 'I',
                      OUTR = 'o',
                      MOUT = '1',
                      MERR = '2',
                      TIME = 'T',
                      NTRL = 't',
                      DEBG = 'D';
    
    
  private:
    int _debug;
    bool _cont;
    bool _help, _version;
    int _lower, _dprob, _upper;
    std::string _in;
    bool _mem, _mutex;
    int _iprob;
    bool _merr, _mout;
    char** _args;
    int _trials;
    std::string _out;
    int _time;
};

#endif
