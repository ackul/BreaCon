#include "parse-args.h"
#include "log.h"

parse::parse(int argc, char** argv) : _debug(log::LDEF), _cont(true), 
                                      _help(false), _version(false), _lower(1000),
                                      _dprob(75), _upper(1000), _in(""), _mem(false), 
                                      _mutex(false), _iprob(100), _merr(false), 
                                      _mout(false), _args(nullptr), _out("a"), 
                                      _trials(10), _time(0) {
    char* curr_arg = argv[1];
    char** curr_arg_ptr = argv + 1;
    bool expect_opt = false;
    bool reached_mutatee = false;
    char arg;
    
    // jump to end if there are not enough arguments.
    if (argc < 2) {
        goto mutatee_check;
    }
    
    // loop through all arguments.
    for (int i = 1; i < argc; ++i) {
        // are we expecting an option?
        if (expect_opt) {
            // set back to false;
            expect_opt = false;
            
            // parse option based on argument.
            switch (arg) {
                case DEBG:
                    _debug = std::stoi(curr_arg);
                    log::level(_debug);
                break;
                case LOWR:
                    _lower = std::stoi(curr_arg);
                break;
                case UPPR:
                    _upper = std::stoi(curr_arg);
                break;
                case DELY:
                    _upper = std::stoi(curr_arg);
                    _lower = _upper;
                break;
                case OUTR:
                    _out = std::string(curr_arg);
                break;
                case NTRL:
                    _trials = std::stoi(curr_arg);
                break;
                case IFIL:
                    _in = std::string(curr_arg);
                break;
                case IPRB:
                    _iprob = std::stoi(curr_arg);
                break;
                case PPRB:
                    _dprob = std::stoi(curr_arg);
                break;
                case TIME:
                    _time = std::stoi(curr_arg);
                break;
                default:
                    log::debug() << "unreachable line: " << __FILE__ << ":" 
                                 << __LINE__ << std::endl;
            }
        }
        
        // if we are not expecting an option:
        else {
            // next argument begins with a dash:  
            if (curr_arg[0] == '-') {
                // parse argument.
                switch (curr_arg[1]) {
                    case HELP:
                        _help = true;
                        _cont = false;
                    break;
                    case VERS:
                        _version = true;
                        _cont = false;
                    break;
                    case DEBG:
                    case LOWR:
                    case UPPR:
                    case DELY:
                    case OUTR:
                    case IFIL:
                    case PPRB:
                    case IPRB:
                    case TIME:
                    case NTRL:
                       arg = curr_arg[1];
                        expect_opt = true;
                    break;
                    case IMEM:
                        _mem = true;
                    break;
                    case IMUT:
                        _mutex = true;
                    break;
                    case MERR:
                        _merr = true;
                    break;
                    case MOUT:
                        _mout = true;
                    break;
                    default:
                        log::alert() << argv[0] << ": unrecognized argument, `"
                                  << curr_arg << "'." << std::endl;
                }
            }
            
            // otherwise assume we have reached the mutatee.
            else {
                // set the beginning of mutatee's args and stop parsing.
                _args = &argv[i];
                reached_mutatee = true;
                break;
            }
        }
        
        // update argument pointers.
        ++curr_arg_ptr;
        curr_arg = *curr_arg_ptr;
    }
    
    // make sure the user has specified a mutatee.
    mutatee_check:
    if (!reached_mutatee) {
        if (_cont) {
            log::error() << argv[0] << ": must specify a mutatee!" << std::endl;
        }
        _cont = false;
    }
}

bool parse::continue_run() const {
    return _cont;
}

int parse::debug_level() const {
    return _debug;
}
    
bool parse::display_help() const {
    return _help;
}

bool parse::display_version() const {
    return _version;
}

int parse::delay_lower() const {
    return _lower;
}

int parse::delay_probability() const {
    return _dprob;
}

int parse::delay_upper() const {
    return _upper;
}

std::string parse::input_file() const {
    return _in;
}

bool parse::instrument_memory() const {
    return _mem;
}

bool parse::instrument_mutex() const {
    return _mutex;
}

int parse::instrument_probability() const {
    return _iprob;
}

bool parse::merge_err() const {
    return _merr;
}

bool parse::merge_out() const {
    return _mout;
}

char** parse::mutatee_arguments() const {
    return _args;
}

std::string parse::output_root() const {
    return _out;
}

int parse::number_trials() const {
    return _trials;
}

int parse::timeout() const {
    return _time;
}
