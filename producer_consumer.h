#ifndef PRODUCERCONSUMER
#define PRODUCERCONSUMER
#include <pthread.h>

int run_threads(int, int, bool);

struct producerInfo {
  int* shared;
};

struct interrupterInfo {
  int workingConsumersAmount;
  pthread_t* consumers;
};

struct consumerInfo {
  int* shared;
  int sleepTime;
  int* global;
  consumerInfo(int* shared, int sleepingTime, int* global)
      : shared(shared), sleepingTime(sleepingTime), global(global){};
};

#endif
