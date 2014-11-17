#include <fstream>
#include <iostream>

/* reads in a file and updates the build numbers. */
int main(int argc, char** argv) {
    std::fstream file(argv[1]);
    int major;
    int minor;
    int build;
    std::string tok;
    
    // read in  class    version {      public:    
    file     >> tok   >> tok >>  tok >> tok;
    
    // read in  static const  int    major  =      VALUE    ;
    file     >> tok >> tok >> tok >> tok >> tok >> major >> tok;
    
    // read in  static const  int    minor  =      VALUE    ;
    file     >> tok >> tok >> tok >> tok >> tok >> minor >> tok;
    
    // read in  static const  int    build  =      VALUE    ;
    file     >> tok >> tok >> tok >> tok >> tok >> build >> tok;
    
    // increment build.
    ++build;
    
    // rewind stream.
    file.seekp(0);
    
    // write it back out.
    file << "class version {" << std::endl
         << "  public:" << std::endl
         << "    static const int major = " << major << ";" << std::endl
         << "    static const int minor = " << minor << ";" << std::endl
         << "    static const int build = " << build << ";" << std::endl
         << "};";
    
    file.close();
    
    return 0;
}
