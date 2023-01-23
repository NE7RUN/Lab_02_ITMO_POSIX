// #define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
// #include <doctest.h>
// #include <producer_consumer.h>
//
// TEST_CASE("just_example") { CHECK(4 == 4); }
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <producer_consumer.h>
#include <iostream>

TEST_CASE("test" * doctest::timeout(1)) {
  std::istringstream in("1 2 3 4 5");
  std::cin.rdbuf(in.rdbuf());
  CHECK(15 == run_threads(1, 0, false));
}
