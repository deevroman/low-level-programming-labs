#ifndef LLP_TYPES_H
#define LLP_TYPES_H


#include <cstdint>


#ifdef __GNUC__
#define PACKED __attribute__((packed))
#else
#define PACKED
#endif


typedef int8_t byte;
typedef byte db_ptr_t;
typedef int64_t db_size_t;

enum prime_types {
    DB_INT_32,
    DB_FLOAT,
    DB_STRING,
    DB_BOOL
};

#endif //LLP_TYPES_H
