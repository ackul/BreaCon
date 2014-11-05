#include <stdio.h>
#include <time.h>
#include <stdlib.h>

/*this is the library*/
void breaconDelay(unsigned int random)
{
    if(random < 75) {
        struct timespec tim, tim2;
        tim.tv_sec = 1;
        tim.tv_nsec = 5000000L;
        if(nanosleep(&tim , &tim2) < 0 )   
        {
            printf("Nano sleep system call failed \n");
        }
        printf("Nano sleep successfull \n");
    }
    else {
        printf("Random > 75\n");
    }
}
