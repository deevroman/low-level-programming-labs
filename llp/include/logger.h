#ifndef LLP_INCLUDE_LOGGER_H_
#define LLP_INCLUDE_LOGGER_H_

#include <exception>
#include <iostream>

#include "types.h"

#ifndef __FILE_NAME__
#define __FILE_NAME__ __FILE__
#endif

template <class TH>
void _dbg(const char *sdbg, TH h) {
  std::cerr << sdbg << '=' << h << std::endl;
}

void _dbg(const char *sdbg, const char *h) { std::cerr << h << std::endl; }

void _dbg(const char *sdbg, DbPtr h) {
  std::cerr << sdbg << '=' << std::hex << h << std::dec << "(" << h << ")" << std::endl;
}

template <class... TA>
void _dbg(const char *sdbg, const char *h, TA... a);

template <class TH, class... TA>
void _dbg(const char *sdbg, TH h, TA... a) {
  while (*sdbg != ',') std::cerr << *sdbg++;
  std::cerr << '=' << h << ',';
  _dbg(sdbg + 1, a...);
}

template <class... TA>
void _dbg(const char *sdbg, DbPtr h, TA... a) {
  while (*sdbg != ',') std::cerr << *sdbg++;
  std::cerr << '=' << std::hex << h << std::dec << "(" << h << ")" << ',';
  _dbg(sdbg + 1, a...);
}

template <class... TA>
void _dbg(const char *sdbg, const char *h, TA... a) {
  while (*sdbg != ',') sdbg++;
  std::cerr << h << ":";
  _dbg(sdbg + 1, a...);
}

#ifdef DEBUG
#define debug(...) _dbg(" ," #__VA_ARGS__, __FILE_NAME__ ":" + std::to_string(__LINE__), __VA_ARGS__)
#else
#define debug(...)
#endif

#ifdef DEBUG
#define debug_assert(...) assert(__VA_ARGS__)
#else
#define debug_assert(...)
#endif

#ifdef DEBUG
#define todo(...) throw std::runtime_error(std::string("line:") + std::to_string(__LINE__) + "," + __VA_ARGS__)
#else
#define todo(...)
#endif

#define error(...) \
  throw std::runtime_error(std::string("line:") + __FILE__ ":" + std::to_string(__LINE__) + "," + __VA_ARGS__)

#endif  // LLP_INCLUDE_LOGGER_H_
