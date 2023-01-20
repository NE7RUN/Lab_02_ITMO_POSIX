#ifndef PRODUCERCONSUMER

#define PRODUCERCONSUMER

#include <pthread.h>

int run_threads(int, int, bool);

struct producerInfo {
  int* shared;
};

struct interruptorInfo {
  int workingConsumersAmount;

  pthread_t* consumers;
};

struct consumerInfo {
  int* shared;

  int sleepTime;

  int* global;

  consumerInfo(int* shared, int sleepTime, int* global)

      : shared(shared), sleepTime(sleepTime), global(global){};
};

#endif
