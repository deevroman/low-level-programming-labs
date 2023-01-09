#ifndef LLP_INCLUDE_ELEMENT_HEADER_CHUNK_H_
#define LLP_INCLUDE_ELEMENT_HEADER_CHUNK_H_
#include "types.h"

#pragma pack(push, 1)
struct raw_element_header {
  PAD(DbSize, id);
  PAD(DbPtr, child_link);
  PAD(DbPtr, brother_link);
} PACKED;
#pragma pack(pop)

// порядок связан с PrimTypes
union ElementData {
  int32_t i_;
  double d_;
  DbPtr str_;
  bool b_;
};

#pragma pack(push, 1)
struct element_value {
  PAD(ElementData, data);
} PACKED;
#pragma pack(pop)

#endif  // LLP_INCLUDE_ELEMENT_HEADER_CHUNK_H_
