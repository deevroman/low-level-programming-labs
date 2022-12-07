#ifndef LLP_INCLUDE_SCHEMA_HEADER_CHUNK_H_
#define LLP_INCLUDE_SCHEMA_HEADER_CHUNK_H_
#include "types.h"

#pragma pack(push, 1)
struct schema_header_chunk {
  PAD(bool, is_chunk);
  PAD(DbPtr, nxt_chunk);
  PAD(DbPtr, name);
  PAD(DbSize, size);
} PACKED;
#pragma pack(pop)

#pragma pack(push, 1)
struct schema_key_value {
  DbPtr key;
  PrimeTypes value_type;
} PACKED;
#pragma pack(pop)

#endif //LLP_INCLUDE_SCHEMA_HEADER_CHUNK_H_
