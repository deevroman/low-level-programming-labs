#ifndef LLP_INCLUDE_RAW_ELEMENT_HEADER_H_
#define LLP_INCLUDE_RAW_ELEMENT_HEADER_H_
#include "types.h"

#pragma pack(push, 1)
struct raw_element_header {
  PAD(DbSize, size);
  PAD(DbPtr, parent_link);
  PAD(DbPtr, child_link);
  PAD(DbPtr, brother_link);
  raw_element_header(DbSize size, DbPtr parent_link, DbPtr child_link, DbPtr brother_link)
      : size(size), parent_link(parent_link), child_link(child_link), brother_link(brother_link) {}
} PACKED;
#pragma pack(pop)

// порядок связан с PrimTypes
union ElementData {
  int32_t i_;
  double d_;
  DbPtr str_;
  bool b_;
  explicit ElementData(int32_t i) : i_(i) {}
  explicit ElementData(double d) : d_(d) {}
  explicit ElementData(DbPtr str) : str_(str) {}
  explicit ElementData(bool b) : b_(b) {}
};

#pragma pack(push, 1)
struct element_value {
  PAD(DbPtr, key);
  PAD(DataItem, type);
  PAD(ElementData, data);
  element_value(DbPtr key, DataItem type, const ElementData& data) : key(key), type(type), data(data) {}
} PACKED;
#pragma pack(pop)

#endif  // LLP_INCLUDE_RAW_ELEMENT_HEADER_H_
