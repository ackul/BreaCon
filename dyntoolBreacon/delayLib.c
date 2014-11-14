#include <stdio.h>
#include <time.h>
#include <stdlib.h>
int i=0;
/*breaconDelay - 75% chance of the sleep being executed
 * else do nothing
 * */
void breaconDelay(unsigned int random)
{
   unsigned int seed;
   if(i==0){
   FILE *fp = fopen("/dev/urandom", "r");
   fread((unsigned int*)(&seed),sizeof(seed), 1, fp);
   printf("The RT library generated seed is %u\n",seed);
   i=1;
   srand(seed);
   }
   int temp = rand()%100;
   if(temp < random) {
       printf("The intermediate rand %d\n",temp);
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
        printf("The probability is %d and it is less than the generated random number temp - %d\n",random,temp);
    }
}
