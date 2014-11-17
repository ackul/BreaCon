#ifndef UTILS_H
#define UTILS_H

class utils {
  public:
    static void close(int);
  
    static std::string create_file_name(const std::string&, int, int);
    
    static int open_for_read(const std::string&);
    
    static int open_for_write(const std::string&);
    
    static const int OUT = 1,
                     ERR = 2;
};

#endif
