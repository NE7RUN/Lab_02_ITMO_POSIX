#include "producer_consumer.h"
#include <pthread.h>
#include <unistd.h>
#include <atomic>
#include <cstddef>
#include <iostream>
#include <memory>
#include <vector>
using namespace std;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t consCond = PTHREAD_COND_INITIALIZER;
pthread_cond_t prodCond = PTHREAD_COND_INITIALIZER;

bool done = false;
bool debug = false;
bool waiting = false;

int get_tid() {
  // 1 to 3+N thread ID
  static atomic<int> last{1};
  static thread_local int* tid = 0;
  if (tid == NULL) tid = new int(last++);
  return *tid;
}

void* producer_routine(void* arg) {
  auto* info = static_cast<producerInfo*>(arg);
  int input;
  while (cin >> input) {
    pthread_mutex_lock(&mutex);
    *info->shared = input;
    pthread_cond_signal(&consCond);

    waiting = true;
    while (waiting) {
      pthread_cond_wait(&prodCond, &mutex);
    }
    pthread_mutex_unlock(&mutex);
  }
  done = true;
  pthread_mutex_lock(&mutex);
  pthread_cond_broadcast(&consCond);
  pthread_mutex_unlock(&mutex);
  return 0;
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
    pthread_mutex_lock(&mutex);
    if (waiting) {
      localSum += *info->shared;
      waiting = false;
      if (debug) {
        writeDebug(localSum);
      }

      while (waiting == false && done == false) {
        pthread_cond_wait(&consCond, &mutex);
      }
    }
    pthread_cond_signal(&prodCond);
    pthread_mutex_unlock(&mutex);
    usleep(pickRandomSleepingTime(info->sleepTime));
  }
  pthread_mutex_lock(&mutex);
  *info->global += localSum;
  pthread_mutex_unlock(&mutex);
  delete (info);
  return 0;
}

int pickRandomConsumerForInterruptor(int workingConsumersAmount) {
  return rand() % workingConsumersAmount;
}

void* consumer_interruptor_routine(void* arg) {
  auto* info = static_cast<interruptorInfo*>(arg);
  while (!done) {
    int workingConsumersAmount = info->workingConsumersAmount;
    if (workingConsumersAmount > 0) {
      pthread_cancel(info->consumers[pickRandomConsumerForInterruptor(
          info->workingConsumersAmount)]);
    }
  }
  return nullptr;
}

// the declaration of run threads can be changed as you like
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
  pthread_t interruptor;
  auto iInfo = interruptorInfo{threadsNumber, consumersPointers};
  pthread_create(&interruptor, nullptr, consumer_interruptor_routine, &iInfo);
  pthread_join(producer, nullptr);
  for (int i = 0; i < threadsNumber; i++) {
    pthread_join(consumersPointers[i], nullptr);
  }
  pthread_join(interruptor, nullptr);
  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&consCond);
  pthread_cond_destroy(&prodCond);
  delete[] consumersPointers;
  delete (shared);
  return sum;
}
