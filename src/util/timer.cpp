#include "timer.hpp"

#include <iostream>

/*
Usage:

void function() {
  // Function scope.
  Timer timer("function()");
  ...
}

void function() {
  {
    // Nested scope to cause destruction earlier.
    Timer timer("thisThing()");
    thisThing();
  }
}
*/
Timer::Timer(const std::string& name) {
  this->name  = name;
  this->start = std::chrono::high_resolution_clock::now();
}

Timer::~Timer() {
  auto end = std::chrono::high_resolution_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  std::cout << this->name << " took: " << duration << std::endl;
}
