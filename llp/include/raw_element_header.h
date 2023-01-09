#ifndef LLP_INCLUDE_RAW_ELEMENT_HEADER_H_
#define LLP_INCLUDE_RAW_ELEMENT_HEADER_H_
#include "types.h"

#pragma pack(push, 1)
struct raw_element_header {
  PAD(DbSize, id);
  PAD(DbSize, size);
  PAD(DbPtr, parent_link);
  PAD(DbPtr, child_link);
  PAD(DbPtr, brother_link);
  raw_element_header(DbSize id, DbSize size, DbPtr parent_link, DbPtr child_link, DbPtr brother_link)
      : id(id), size(size), parent_link(parent_link), child_link(child_link), brother_link(brother_link) {}
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
  PAD(PrimeTypes, type);
  PAD(ElementData, data);
} PACKED;
#pragma pack(pop)

#endif  // LLP_INCLUDE_RAW_ELEMENT_HEADER_H_
