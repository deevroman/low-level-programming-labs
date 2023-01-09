#ifndef LLP_INCLUDE_TYPES_H_
#define LLP_INCLUDE_TYPES_H_

#include <cstdint>

#ifdef __GNUC__
#define PACKED __attribute__((packed))
#else
#define PACKED
#endif

constexpr size_t CalcPadding(size_t x) {
  if (x >= 8) {
    return 16 - x;
  } else {
    return 32 - x;
  }
}

#ifdef DEBUG
#define PAD(type, value)                 \
  char name_##value[16] = #value;        \
  char pad_##value[CalcPadding(sizeof(type))]{}; \
  type value
#else
#define PAD(args) args
#endif

typedef int8_t Byte;
typedef int64_t DbPtr;
typedef int64_t DbSize;

// порядок связан с ElementData
enum PrimeTypes { DB_INT_32 = 'I', DB_DOUBLE = 'D', DB_STRING = 'S', DB_BOOL = 'B'};



#endif  // LLP_INCLUDE_TYPES_H_
