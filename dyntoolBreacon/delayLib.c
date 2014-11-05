#include <stdio.h>
#include <time.h>
#include <stdlib.h>

/*this is the library*/
void breaconDelay()
{
        struct timespec tim, tim2;
        tim.tv_sec = 1;
        tim.tv_nsec = 5000000L;
        if(nanosleep(&tim , &tim2) < 0 )   
        {
            printf("Nano sleep system call failed \n");
        }
        printf("Nano sleep successfull \n");
}
