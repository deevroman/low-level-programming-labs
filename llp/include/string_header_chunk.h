#ifndef LLP_INCLUDE_STRING_HEADER_CHUNK_H_
#define LLP_INCLUDE_STRING_HEADER_CHUNK_H_
#include "types.h"

#pragma pack(push, 1)
struct string_header_chunk {
  PAD(bool, is_chunk);
  PAD(DbSize, size);
  PAD(DbPtr, nxt_chunk);
} PACKED;
#pragma pack(pop)

#endif //LLP_INCLUDE_STRING_HEADER_CHUNK_H_
