#ifndef LLP_INCLUDE_RAW_ELEMENT_HEADER_H_
#define LLP_INCLUDE_RAW_ELEMENT_HEADER_H_
#include "types.h"

#pragma pack(push, 1)
struct raw_element_header {
  PAD(DbSize, size);
  PAD(DbPtr, schema);
  PAD(DbPtr, parent_link);
  PAD(DbPtr, child_link);
  PAD(DbPtr, pr_brother_link);
  PAD(DbPtr, brother_link);
  raw_element_header(DbSize size, DbPtr schema, DbPtr parent_link, DbPtr child_link, DbPtr pr_brother_link,
                     DbPtr brother_link)
      : size(size),
        schema(schema),
        parent_link(parent_link),
        child_link(child_link),
        pr_brother_link(pr_brother_link),
        brother_link(brother_link) {}
} PACKED;
#pragma pack(pop)

// порядок связан с PrimTypes
#pragma pack(push, 1)
union ElementData {
  int32_t i_;
  double d_;
  DbPtr str_;
  bool b_;
  explicit ElementData(int32_t i) : i_(i) {}
  explicit ElementData(double d) : d_(d) {}
  explicit ElementData(DbPtr str) : str_(str) {}
  explicit ElementData(bool b) : b_(b) {}
} PACKED;
#pragma pack(pop)

#pragma pack(push, 1)
struct element_value {
  PAD(DbPtr, key);
  PAD(DataItemType, type);
  PAD(ElementData, data);
  element_value(DbPtr key, DataItemType type, const ElementData& data) : key(key), type(type), data(data) {}
} PACKED;
#pragma pack(pop)

#endif  // LLP_INCLUDE_RAW_ELEMENT_HEADER_H_
