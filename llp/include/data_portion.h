#ifndef EXAMPLES_DATA_PORTION_H
#define EXAMPLES_DATA_PORTION_H


#include <cstdio>
#include "data_item.h"

class data_portion {
public:
    size_t size;
    size_t capacity;
    data_item *item;

    data_portion() = default;

    explicit data_portion(size_t size, size_t capacity, data_item *item = nullptr) : size(size),
                                                                                     capacity(capacity),
                                                                                     item(item) {};


};


#endif //EXAMPLES_DATA_PORTION_H
