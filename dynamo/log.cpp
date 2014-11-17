#include "log.h"
#include <iostream>

/* null stream: doesn't print anything. */
class nstream : public std::ostream {
  public:
    std::ostream& operator<< (bool v) { return *this; }
    std::ostream& operator<< (short v) { return *this; }
    std::ostream& operator<< (unsigned short v) { return *this; }
    std::ostream& operator<< (int v) { return *this; }
    std::ostream& operator<< (unsigned int v) { return *this; }
    std::ostream& operator<< (long v) { return *this; }
    std::ostream& operator<< (unsigned long v) { return *this; }
    std::ostream& operator<< (float v) { return *this; }
    std::ostream& operator<< (double v) { return *this; }
    std::ostream& operator<< (long double v) { return *this; }
    std::ostream& operator<< (void* v) { return *this; }
    std::ostream& operator<< (std::streambuf* v) { return *this; }
    std::ostream& operator<< (std::ostream& (*v)(std::ostream&)) { return *this; }
    std::ostream& operator<< (std::ios& (*v)(std::ios&)) { return *this; }
    std::ostream& operator<< (std::ios_base& (*v)(std::ios_base&)) { return *this; }
    
    static nstream& open() {
        static nstream* stream = new nstream();
        return *stream;
    }
  
  private:
    nstream() { }
};

int log::_level = LDEF;

std::ostream& log::alert() {
    // log if we have appropriate level.
    if (level() >= LART) {
        clog() << "[ALERT] ";
        return clog();
    }
    
    // otherwise, return null stream.
    return null();
}

std::ostream& log::clog() {
    return std::clog;
}

std::ostream& log::cout() {
    return std::cout;
}
    
std::ostream& log::debug() {
    // log if we have appropriate level.
    if (level() >= LDBG) {
        clog() << "[DEBUG] ";
        return clog();
    }
    
    // otherwise, return null stream.
    return null();
}

std::ostream& log::error() {
    // log if we have appropriate level.
    if (level() >= LERR) {
        clog() << "[ERROR] ";
        return clog();
    }
    
    // otherwise, return null stream.
    return null();
}

std::ostream& log::info() {
    // log if we have appropriate level.
    if (level() >= LINF) {
        clog() << "[INFO ] ";
        return clog();
    }
    
    // otherwise, return null stream.
    return null();
}

int log::level() {
    return _level;
}

void log::level(int l) {
    if (l > LMAX) { l = LMAX; }
    if (l < LMIN) { l = LMIN; }
    _level = l;
}

std::ostream& log::null() {
    return nstream::open();
}

std::ostream& log::warn() {
    // log if we have appropriate level.
    if (level() >= LWRN) {
        clog() << "[WARN ] ";
        return clog();
    }
    
    // otherwise, return null stream.
    return null();
}
