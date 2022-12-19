#ifndef LLP_INCLUDE_SCHEMA_HEADER_CHUNK_H_
#define LLP_INCLUDE_SCHEMA_HEADER_CHUNK_H_
#include "types.h"

#pragma pack(push, 1)
struct raw_schema_header {
  PAD(DbPtr, name);
  PAD(DbSize, size);
} PACKED;
#pragma pack(pop)

#pragma pack(push, 1)
struct schema_key_value {
  PAD(DbPtr, key);
  PAD(PrimeTypes, value_type);
} PACKED;
#pragma pack(pop)

#endif  // LLP_INCLUDE_SCHEMA_HEADER_CHUNK_H_
