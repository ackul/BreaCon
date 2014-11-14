#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
int var = 0;

void* child_fn ( void* arg ) {
       var = 20; /* Unprotected relative to parent */ /* this is line 6 */
       return NULL;
}

int main ( void ) {
    pthread_t child;
    pthread_create(&child, NULL, child_fn, NULL);
    var = 10; /* Unprotected relative to child */ /* this is line 13 */
    pthread_join(child, NULL);
    printf("Value of var %d\n",var);
    if(var == 10)
    {
        printf("Race Warning: Child was made faster\n");
    }
    return 0;
}
