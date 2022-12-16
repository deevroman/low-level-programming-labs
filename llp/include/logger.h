#ifndef LLP_INCLUDE_LOGGER_H_
#define LLP_INCLUDE_LOGGER_H_

#include <iostream>
#include <exception>

// #define debug(x) std::cout << #x << ": " << x << endl;
template<class TH>
void _dbg(const char *sdbg, TH h) { std::cerr << sdbg << '=' << h << std::endl; }

template<class TH, class... TA>
void _dbg(const char *sdbg, TH h, TA... a) {
  while (*sdbg != ',') std::cerr << *sdbg++;
  std::cerr << '=' << h << ',';
  _dbg(sdbg + 1, a...);
}

#ifdef DEBUG
#define debug(...) _dbg("line,"#__VA_ARGS__, __LINE__, __VA_ARGS__)
#else
#define debug(...)
#endif

#ifdef DEBUG
#define todo(...) throw std::runtime_error(std::string("line:") + std::to_string(__LINE__) + "," + __VA_ARGS__)
#else
#define todo(...)
#endif

#include <string_view>

void info(std::string_view s) {
  std::cerr << s << std::endl;
}

#define error(...) throw std::runtime_error(std::string("line:") + std::to_string(__LINE__) + "," + __VA_ARGS__)

#endif //LLP_INCLUDE_LOGGER_H_
