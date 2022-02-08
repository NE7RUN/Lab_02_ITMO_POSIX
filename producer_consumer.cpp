#include <pthread.h>
#include <iostream>

int get_tid() {
  // 1 to 3+N thread ID
  return 0;
}

void* producer_routine(void* arg) {
  (void)arg;
  // read data, loop through each value and update the value, notify consumer,
  // wait for consumer to process
  return nullptr;
}

void* consumer_routine(void* arg) {
  (void)arg;
  // for every update issued by producer, read the value and add to sum
  // return pointer to result (for particular consumer)
  return nullptr;
}

void* consumer_interruptor_routine(void* arg) {
  (void)arg;
  // interrupt random consumer while producer is running
  return nullptr;
}

// the declaration of run threads can be changed as you like
int run_threads() {
  // start N threads and wait until they're done
  // return aggregated sum of values

  return 0;
}
