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
#define PAD(type, value)                         \
  char name_##value[16] = #value;                \
  char pad_##value[CalcPadding(sizeof(type))]{}; \
  type value
#else
#define PAD(args) args
#endif

typedef int8_t Byte;
typedef int64_t DbPtr;
typedef int64_t DbSize;

// порядок связан с ElementData
enum DataItem { DB_INT_32 = 0, DB_DOUBLE, DB_STRING, DB_BOOL };

#endif  // LLP_INCLUDE_TYPES_H_
