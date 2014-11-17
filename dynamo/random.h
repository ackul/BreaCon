#ifndef RANDOM_H
#define RANDOM_H

/* class for contain random number generators and sleepers. */
class random {
  public:
      
    /* sleep for a number of milliseconds. */
    static int sleep(int);
    
    /* a non-templated uniform generator for the range [A, B]. once used, the
       range is permanent, and the two int parameters are useless. */
    static int uniform_int(int, int);
    
    /* template for uniform distribution generators for the range [A, B]. may
       also only be used once, because it relies on the non-template version. */
    template <int A, int B> static int uniform_int() {
        return uniform_int(A, B);
    }
};

#endif
