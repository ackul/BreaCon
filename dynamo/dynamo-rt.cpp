#include "random.h"
#define RT_DEBUG
#ifdef RT_DEBUG
#include "log.h"
#endif

/* default delay function. delays prob percent of the time for an amount 
   randomly in the range [min, max]. times are in milliseconds. */
void std_delay(int prob, int min, int max) {
    int cont = random::uniform_int<1, 100>();
    int length;
    
    log::level(log::LMAX);
    log::info() << "in runtime: ";
    
    // abort if above probability.
    if (cont > prob) { log::info() << "skipping." << std::endl; return; }
    
    // generate length.
    if (min == max) { length = min; }
    else { length = random::uniform_int(min, max); }
    
    #ifdef RT_DEBUG
    log::level(log::LMAX);
    log::info() << "sleeping." << std::endl;
    #endif
    
    // sleep.
    random::sleep(length);
}
