#include "producer_consumer.h"
#include <pthread.h>
#include <unistd.h>
#include <atomic>
#include <cstddef>
#include <iostream>
#include <memory>
#include <vector>

using namespace std;

bool done = false;
bool debug = false;
bool waiting = false;

pthread_mutex_t globalMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t consumerCond = PTHREAD_COND_INITIALIZER;
pthread_cond_t producerCond = PTHREAD_COND_INITIALIZER;

int get_tid() {
  static atomic<int> last{1};
  static thread_local int* tid = nullptr;
  if (tid == NULL) tid = new int(last++);
  return *tid;
}

void* producer_routine(void* arg) {
  auto* info = static_cast<producerInfo*>(arg);
  int input;
  while (cin >> input) {
    pthread_mutex_lock(&globalMutex);
    *info->shared = input;
    pthread_cond_signal(&consumerCond);
    waiting = true;
    while (waiting) {
      pthread_cond_wait(&producerCond, &globalMutex);
    }
    pthread_mutex_unlock(&globalMutex);
  }
  done = true;
  pthread_mutex_lock(&globalMutex);
  pthread_cond_broadcast(&consumerCond);
  pthread_mutex_unlock(&globalMutex);
  // delete (info);
  return nullptr;
}

void writeDebug(int sum) { cout << "(" << get_tid() << ", " << sum << ")\n"; }

int pickRandomSleepingTime(int sleep) {
  if (sleep) {
    return (rand() % sleep + 1) * 1000;
  } else {
    return 0;
  }
}

void* consumer_routine(void* arg) {
  auto* info = static_cast<consumerInfo*>(arg);
  thread_local int localSum = 0;
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
  while (done == false) {
    pthread_mutex_lock(&globalMutex);
    while (waiting == false && done == false) {
      pthread_cond_wait(&consumerCond, &globalMutex);
    }
    if (waiting) {
      localSum += *info->shared;
      waiting = false;
      if (debug) {
        writeDebug(localSum);
      }
    }
    pthread_cond_signal(&producerCond);
    pthread_mutex_unlock(&globalMutex);
    usleep(pickRandomSleepingTime(info->sleepingTime));
  }
  pthread_mutex_lock(&globalMutex);
  *info->global += localSum;
  pthread_mutex_unlock(&globalMutex);
  delete (info);
  return nullptr;
}

int pickRandomConsumerForInterrupter(int workingConsumersAmount) {
  return rand() % workingConsumersAmount;
}

void* consumer_interrupter_routine(void* arg) {
  auto* info = static_cast<interrupterInfo*>(arg);
  while (!done) {
    int workingConsumersAmount = info->workingConsumersAmount;
    if (workingConsumersAmount > 0) {
      pthread_cancel(info->consumers[pickRandomConsumerForInterrupter(
          info->workingConsumersAmount)]);
    }
  }
  // delete (info);
  return nullptr;
}

int run_threads(int threadsNumber, int sleepLimit, bool debug) {
  int* shared = new int;
  int sum = 0;
  if (debug) {
    ::debug = true;
  }
  pthread_t producer;
  auto pInfo = producerInfo{shared};
  pthread_create(&producer, nullptr, producer_routine, &pInfo);
  pthread_t* consumersPointers = new pthread_t[threadsNumber];
  for (int i = 0; i < threadsNumber; i++) {
    pthread_create(&consumersPointers[i], nullptr, consumer_routine,
                   new consumerInfo(shared, sleepLimit, &sum));
  }
  pthread_t interrupter;
  auto iInfo = interrupterInfo{threadsNumber, consumersPointers};
  pthread_create(&interrupter, nullptr, consumer_interrupter_routine, &iInfo);
  pthread_join(producer, nullptr);
  for (int i = 0; i < threadsNumber; i++) {
    pthread_join(consumersPointers[i], nullptr);
  }
  pthread_join(interrupter, nullptr);
  pthread_mutex_destroy(&globalMutex);
  pthread_cond_destroy(&consumerCond);
  pthread_cond_destroy(&producerCond);
  delete[] consumersPointers;
  delete (shared);
  return sum;
}