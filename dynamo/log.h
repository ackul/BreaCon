#ifndef LOG_H
#define LOG_H

#include <ostream>

/* class for logging output. all print to clog except for null, cout, dynamo. 
   all streams besides clog, cout, and null print a header describing their
   purpose when called if the log level is sufficiently high. if the level
   is too low, the streams do not print anything and return the null stream. */
class log {
  public: 
    /* stream for printing alerts. only prints if level is at least LART. */   
    static std::ostream& alert();
    
    /* standard log stream. */
    static std::ostream& clog();
    
    /* standard output stream. */
    static std::ostream& cout();
    
    /* stream for printing debug. only prints if level is at least LDBG. */
    static std::ostream& debug();
    
    /* stream for standard output. */
    static std::ostream& dynamo();
    
    /* stream for printing errors. only prints if level is at least LERR. */
    static std::ostream& error();
    
    /* stream for printing info. only prints if level is at least LINF. */
    static std::ostream& info();
    
    /* returns the current log level. */
    static inline int level();
    
    /* sets the current log level. */
    static void level(int);
    
    /* stream for discarding text. will not print anywhere. */
    static std::ostream& null();
    
    /* stream for printing warnings. only prints if level is at least LWRN. */
    static std::ostream& warn();
    
    /* different log levels with min, max, and default. */
    static const int LMIN = 0,
                     LNUL = 0,
                     LERR = 1,
                     LART = 2,
                     LDEF = 2,
                     LDBG = 3,
                     LWRN = 4,
                     LINF = 5,
                     LMAX = 5;
                     
    
  private:
    static int _level;
};

#endif
