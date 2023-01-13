#ifndef LLP_INCLUDE_RAW_SCHEMA_HEADER_H_
#define LLP_INCLUDE_RAW_SCHEMA_HEADER_H_
#include "types.h"

#pragma pack(push, 1)
struct raw_schema_header {
  PAD(DbPtr, name);
  PAD(DbSize, size);
  PAD(DbSize, cnt_elements);
  PAD(DbPtr, nxt);
  raw_schema_header(DbPtr name, DbSize size, DbSize cnt_elements, DbPtr nxt)
      : name(name), size(size), cnt_elements(cnt_elements), nxt(nxt) {}
} PACKED;
#pragma pack(pop)

#pragma pack(push, 1)
struct schema_key_value {
  PAD(DbPtr, key);
  PAD(DataItemType, value_type);
} PACKED;
#pragma pack(pop)

#endif  // LLP_INCLUDE_RAW_SCHEMA_HEADER_H_
