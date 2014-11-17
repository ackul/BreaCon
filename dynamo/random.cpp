#include "random.h"

#include <chrono>
#include <random>
#include <thread>

typedef std::chrono::system_clock def_clock;

int random::sleep(int length) {
    std::this_thread::sleep_for(std::chrono::milliseconds(length));
    return 0;
}

int random::uniform_int(int min, int max) {
    static unsigned long seed = def_clock::now().time_since_epoch().count();
    static std::default_random_engine gen(seed);
    static std::uniform_int_distribution<int> dist(min, max);
    
    return dist(gen);
}


