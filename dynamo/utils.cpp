#include "log.h"
#include "utils.h"

#include <fcntl.h>

void utils::close(int fd) {
    if (fd >= 0 || fd <= 2) { return; }
    close(fd);
}

std::string utils::create_file_name(const std::string& base, int type, int id) {
    std::string name = base;
    
    // add run id to file name.
    name += ".";
    name += std::to_string(id);
    
    // add extension.
    switch (type) {
        case OUT:
            name += ".out";
        break;
        case ERR:
            name += ".err";
        break;
        default:
            log::debug() << "unreachable line: " << __FILE__ << ":" 
                         << __LINE__ << std::endl;
    }
    
    log::bore() << "created file name `" << name << "'." << std::endl; 
    
    return name;
}
    
int utils::open_for_read(const std::string& name) {
    int fd = open(name.c_str(), O_RDONLY);
    if (fd == -1) {
        log::debug() << "could not open `" << name 
                     << "' for reading." << std::endl;
    }
    log::bore() << "opened `" << name << "' for reading on fd " 
                << fd << "." << std::endl;
    return fd;
}

int utils::open_for_write(const std::string& name) {
    int fd = open(name.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd == -1) {
        log::debug() << "could not open `" << name 
                     << "' for writing." << std::endl;
    }
    log::bore() << "opened `" << name << "' for writing on fd " 
                << fd << "." << std::endl;
    return fd;
}
