#include <pthread.h>
#include <string.h>
#include <iostream>
#include "producer_consumer.h"
using namespace std;

int main(int argc, char** argv) {
  bool debug = false;
  if (argc < 3) {
    cout << "more" << endl;
    return 1;
  }
  if (argc == 4 && !strcmp("-debug", argv[3])) {
    debug = true;
  }
  int threadsNumber = atoi(argv[1]);
  int sleepLimit = atoi(argv[2]);
  cout << run_threads(threadsNumber, sleepLimit, debug);
}
