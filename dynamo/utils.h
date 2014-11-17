#ifndef UTILS_H
#define UTILS_H

/* file utilities. */
class utils {
  public:
    /* closes the specified file. */
    static void close(int);
    
    /* creates a file name of the form <root>.<id>.<type>. */
    static std::string create_file_name(const std::string&, int, int);
    
    /* opens the provided file name for reading and returns the fd. */
    static int open_for_read(const std::string&);
    
    /* opens the provided file name for writing and returns the fd. */
    static int open_for_write(const std::string&);
    
    /* constants for output and error streams. */
    static const int OUT = 1,
                     ERR = 2;
};

#endif
