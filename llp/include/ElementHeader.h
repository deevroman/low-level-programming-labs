#ifndef LLP_INCLUDE_ELEMENTHEADER_H_
#define LLP_INCLUDE_ELEMENTHEADER_H_

#include "logger.h"
#include "raw_element_header.h"
#include "raw_schema_header.h"
#include "types.h"
//
//class ElementHeader {
// public:
//  DbSize id_{};
//  DbSize size_{};
//  DbPtr brother_link{};
//  element_value *fields_{};
//  ElementHeader(DbSize id, DbSize size) : id_(id), size_(size) {
//    debug("Создаю SchemaHeader", id, size);
//    fields_ = reinterpret_cast<element_value *>(new char[GetFlexibleElementSize()]{});
//  }
//  virtual ~ElementHeader() { delete[] fields_; }
//  [[nodiscard]] DbSize GetOnFileSize() const { return sizeof(raw_schema_header) + GetFlexibleElementSize(); }
//  [[nodiscard]] DbSize GetFlexibleElementSize() const {
//    // округялем вверх до размера заголовка, чтобы при наличии пустоты на странице в
//    // неё всегда мог поместиться заголовок таким образом все схемы будут
//    // лежать в памяти плотно
//    return (size_ * sizeof(element_value) + sizeof(raw_schema_header) - 1) / sizeof(raw_schema_header) *
//           sizeof(raw_schema_header);
//  }
//  [[nodiscard]] DbSize GetFlexiblePadding() const {
//    return size_ * sizeof(element_value) % sizeof(raw_schema_header);
//  }
//};

#endif  // LLP_INCLUDE_ELEMENTHEADER_H_
