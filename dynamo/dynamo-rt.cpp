#include "random.h"

#ifdef RT_DEBUG
#include "log.h"
#endif

/* default delay function. delays prob percent of the time for an amount 
   randomly in the range [min, max]. times are in milliseconds. */
void std_delay(int prob, int min, int max) {
    int cont = random::uniform_probability();
    int length;
    
    #ifdef RT_DEBUG
    log::level(log::LMAX);
    log::info() << "in runtime: ";
    #endif
    
    // abort if above probability.
    if (cont > prob) { 
    #ifdef RT_DEBUG
        log::clog() << "skipping." << std::endl; 
    #endif
        return; 
    }
    
    // generate length.
    if (min == max) { length = min; }
    else { length = random::uniform_int(min, max); }
    
    #ifdef RT_DEBUG
    log::clog() << "sleeping." << std::endl;
    #endif
    
    // sleep.
    random::sleep(length);
}
