#include <cstring>
#include <iostream>
#include "producer_consumer.h"

int main(int argc, char *argv[]) {
  int consumers, sleeper;
  bool debug = false;

  if (argc > 2) {
    consumers = std::atoi(argv[1]);
    sleeper = std::atoi(argv[2]);
    if (argc > 3) {
      debug = !std::strcmp(argv[3], "-debug");
    }
  } else {
    std::cout << "need more power";
    return 1;
  }

  std::cout << run_threads(consumers, sleeper, debug) << std::endl;
  return 0;
}
