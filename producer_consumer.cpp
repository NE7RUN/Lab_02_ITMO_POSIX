#include <pthread.h>
#include <unistd.h>
#include <atomic>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <vector>

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t condition_producer = PTHREAD_COND_INITIALIZER;
static pthread_cond_t condition_consumer = PTHREAD_COND_INITIALIZER;

bool debug_is_enabled = false;
bool end = false;
bool is_finished = false;

struct consumer {
  int sleep_time = -1;
  int* shared_variable;
  int* count;

  consumer(int sleeper, int* shared_variable, int* count)
      : sleep_time(sleeper), shared_variable(shared_variable), count(count){};
};

struct producer {
  int* shared_variable;

  producer(int* shared_variable) : shared_variable(shared_variable){};
};

struct interrupter {
  int size_consumers = -1;
  pthread_t* all_consumers;

  interrupter(int size_consumers, pthread_t* all_consumers)
      : size_consumers(size_consumers), all_consumers(all_consumers){};
};

int get_tid() {
  thread_local static int tid = 0;
  static std::atomic_int count = 0;

  if (tid == 0) {
    tid = ++count;
  }
  return tid;
}

void* producer_routine(void* arg) {
  auto* producer_struct = static_cast<producer*>(arg);

  std::vector<int> numbers;

  std::string s;
  getline(std::cin, s);

  std::string::size_type size = s.length();
  char* const buffer = new char[size + 1];

  strncpy(buffer, s.c_str(), size);

  char* p = strtok(buffer, " ");
  while (p) {
    numbers.push_back(std::atoi(p));
    p = strtok(NULL, " ");
  }

  for (auto num : numbers) {
    pthread_mutex_lock(&mutex);
    // assigning data to a shared variable
    *(producer_struct->shared_variable) = num;
    pthread_cond_signal(&condition_consumer);

    while (*(producer_struct->shared_variable) != 0) {
      pthread_cond_wait(&condition_producer, &mutex);
    }

    pthread_mutex_unlock(&mutex);
  }

  is_finished = true;
  pthread_mutex_lock(&mutex);
  pthread_cond_broadcast(&condition_consumer);
  pthread_mutex_unlock(&mutex);

  delete[] buffer;
  delete (producer_struct);
  return nullptr;
}

void* consumer_routine(void* arg) {
  (void)arg;
  // for every update issued by producer, read the value and add to sum
  // return pointer to result (for particular consumer)
  auto* consumer_struct = static_cast<consumer*>(arg);
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);

  // loop exit test
  while (!is_finished) {
    pthread_mutex_lock(&mutex);
    // waiting for notification from the producer
    while (!is_finished && *(consumer_struct->shared_variable) == 0) {
      pthread_cond_wait(&condition_consumer, &mutex);
    }

    // data reading
    if (*(consumer_struct->shared_variable) != 0) {
      *(consumer_struct->count) += *(consumer_struct->shared_variable);
      if (debug_is_enabled) {
        std::fprintf(stderr, "tid=%d psum=%d\n", get_tid(),
                     *(consumer_struct->count));
      }
      *(consumer_struct->shared_variable) = 0;
    }

    pthread_cond_signal(&condition_producer);
    pthread_mutex_unlock(&mutex);

    usleep(std::rand() % consumer_struct->sleep_time);
  }
  delete (consumer_struct);
  return nullptr;
}

void* consumer_interruptor_routine(void* arg) {
  // interrupt random consumer while producer is running
  auto* interrupter_struct = (interrupter*)arg;
  // interruptor tryhard
  while (!is_finished) {
    pthread_cancel(
        interrupter_struct
            ->all_consumers[std::rand() % interrupter_struct->size_consumers]);
  }
  delete (interrupter_struct);
  return nullptr;
}
// the declaration of run threads can be changed as you like
int run_threads(int consumers, int sleeper, bool debug) {
  // start N threads and wait until they're done
  // return aggregated sum of values
  debug_is_enabled = false;
  end = false;
  is_finished = false;

  debug_is_enabled = debug;
  int shared_variable = 0;
  int answer = 0;

  int* consumers_variable_count = new int[consumers];
  pthread_t* consumers_pointers = new pthread_t[consumers];
  pthread_t producer_pointer, interrupter_pointer;

  for (int i = 0; i < consumers; i++) {
    consumers_variable_count[i] = 0;
  }

  pthread_create(&producer_pointer, nullptr, producer_routine,
                 new producer(&shared_variable));
  for (int i = 0; i < consumers; i++) {
    pthread_create(&consumers_pointers[i], nullptr, consumer_routine,
                   new consumer(1000 * sleeper + 1, &shared_variable,
                                &consumers_variable_count[i]));
  }

  pthread_create(&interrupter_pointer, nullptr, consumer_interruptor_routine,
                 new interrupter(consumers, consumers_pointers));

  pthread_join(producer_pointer, nullptr);
  pthread_join(interrupter_pointer, nullptr);

  for (int i = 0; i < consumers; ++i) {
    pthread_join(consumers_pointers[i], nullptr);
    answer += consumers_variable_count[i];
  }
  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&condition_producer);
  pthread_cond_destroy(&condition_consumer);
  delete[] consumers_variable_count;
  delete[] consumers_pointers;
  return answer;
}
