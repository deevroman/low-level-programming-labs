#ifndef EXAMPLES_STORAGE_LINK_H
#define EXAMPLES_STORAGE_LINK_H

#include <cstdio>

class storage_link{
public:
    FILE **file;
    long long offset;
};

#endif //EXAMPLES_STORAGE_LINK_H
