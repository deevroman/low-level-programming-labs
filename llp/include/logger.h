#ifndef LLP_LOGGER_H
#define LLP_LOGGER_H

#include <iostream>

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
#define _GLIBCXX_DEBUG
#define debug(...) _dbg("line,"#__VA_ARGS__, __LINE__, __VA_ARGS__)
#else
#define debug(...)
#endif

#include <string_view>

void info(std::string_view s) {
    std::cerr << s << std::endl;
}

void error(std::string_view s) {
    std::cerr << s << std::endl;
}


#endif //LLP_LOGGER_H
