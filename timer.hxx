#ifndef TIMER__HXX
#define TIMER__HXX

#include <sys/resource.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <stdio.h>

#include <cstdint>
#include <chrono>
#include <iostream>

using namespace std::chrono;

class Timer {
private:
  std::int64_t usertime_start, systime_start;
  time_point<steady_clock> walltime_start;

  std::int64_t usertime_end, systime_end;
  time_point<steady_clock> walltime_end;

  bool has_started = false;

public:
  Timer() = default;

  enum class PrintFmt { human, csv };

  void ensure_not_started() const {
    if (has_started) {
      throw std::runtime_error(
          "cannot get usage: end() has not been called yet");
    }
  }

  std::int64_t get_usertime() const {
    ensure_not_started();
    return usertime_end - usertime_start;
  }

  std::int64_t get_systime() const {
    ensure_not_started();
    return systime_end - systime_start;
  }

  std::int64_t get_walltime() const {
    ensure_not_started();
    return duration_cast<microseconds>(walltime_end - walltime_start).count();
  }

  void start() {
    has_started = true;
    struct rusage usage;
    int ret = getrusage(RUSAGE_SELF, &usage);

    if (ret != 0) {
      std::cerr << "rusage error: " << ret << std::endl;
      exit(ret);
    }

    usertime_start = usage.ru_utime.tv_sec * 1000000 + usage.ru_utime.tv_usec;
    systime_start = usage.ru_stime.tv_sec * 1000000 + usage.ru_stime.tv_usec;
    walltime_start = steady_clock::now();
  }

  void end() {
    has_started = false;
    struct rusage usage;
    int ret = getrusage(RUSAGE_SELF, &usage);

    if (ret != 0) {
      std::cerr << "rusage error: " << ret << std::endl;
      exit(ret);
    }

    usertime_end = usage.ru_utime.tv_sec * 1000000 + usage.ru_utime.tv_usec;
    systime_end = usage.ru_stime.tv_sec * 1000000 + usage.ru_stime.tv_usec;
    walltime_end = steady_clock::now();
  }

  void print(bool humanReadable = false) {
    if (humanReadable) {
      std::cout << "user: " << get_usertime() << ", sys: " << get_systime()
                << ", wall: " << get_walltime() << std::endl;
    } else {
      std::cout << get_usertime() << ", " << get_systime() << ", "
                << get_walltime() << std::endl;
    }
  }
};

#endif // TIMER__HXX
