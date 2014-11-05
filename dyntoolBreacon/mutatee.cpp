#include <stdio.h>
#include <pthread.h>

pthread_mutex_t lock;

int a,b;

void *para(void *t){

  pthread_mutex_lock(&lock);
  printf("child thread acquire the lock\n");
  a=11;
  pthread_mutex_unlock(&lock);
  printf("child thread release the lock\n");
  b=11;
  printf("child finish\n");
}


int main(int argn, char **argv){
  pthread_t pid;
  pthread_mutex_init(&lock, NULL);

  pthread_create(&pid, NULL, para , (void*)pid);
  pthread_mutex_lock(&lock);
  printf("main thread acquire the lock\n");
  a = 10;
  pthread_mutex_unlock(&lock);
  printf("main thread release the lock\n");
  b = 10;
  printf("main before join\n");
  pthread_join(pid, NULL);
  printf("main finish\n");
  return 0;
}

