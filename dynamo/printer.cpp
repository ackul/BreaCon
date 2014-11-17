#include "printer.h"
#include "log.h"
#include "version.h"

void printer::help(char* prog) {
    log::cout() << "usage: " << prog << " [dynamo-arguments] mutatee [mutatee-arguments]"
                << std::endl << std::endl << "At least a mutatee must be specified."
                " mutatee-arguments will be passed thereto." << std::endl
                << "dynamo-arguments are optional and effect the performance of dynamo."
                << std::endl << "The following may be passed to dynamo:" << std::endl
                << std::endl
                << "  -h     prints this help message." << std::endl
                << "  -v     prints the version information of dynamo." << std::endl
                << "  -t  n  sets the number of trial runs to n. default is 10." << std::endl
                << "  -T  n  sets the time limit for each trial to n. default is 0." << std::endl
                << "  -m     enables instrumenting mutexes in mutatee." << std::endl
                << "  -a     enables instrumenting memory accesses in mutatee." << std::endl
                << "  -p  n  sets the probability of delaying to n. default is 75." << std::endl
                << "  -i  n  sets the probability of instrumenting to n. default is 100." << std::endl
                << "  -l  n  sets the lower bound for delay times to n. default is 1000." << std::endl
                << "  -u  n  sets the upper bound for delay times to n. default is 1000." << std::endl
                << "  -d  n  sets both bounds for delay times to n." << std::endl
                << "  -o  s  sets the root file name for mutatee output files to s. default is 'a'." << std::endl
                << "  -i  s  sets the input file for the mutatee to s. default is stdin." << std::endl
                << "  -1     merges the mutatee's output stream with dynamo's." << std::endl
                << "  -2     merges the mutatee's error stream with dynamo's." << std::endl
                << "  -D  n  sets the log level verbosity to n. range is [0-5]. default is 2." << std::endl
                << std::endl;
}
    
void printer::version() {
    log::cout() << "Dynamo Thread Fuzz Tester" << std::endl
                << "Achin Kulshrestha and Alex Morris" << std::endl
                << "Version " << version::major << "." << version::minor
                << "." << version::build << std::endl << std::endl;
}
