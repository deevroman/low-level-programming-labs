#ifndef LLP_INCLUDE_SCHEMAHEADER_H_
#define LLP_INCLUDE_SCHEMAHEADER_H_

#include "logger.h"
#include "schema_header_chunk.h"
#include "types.h"

class SchemaHeader {
 public:
  DbPtr name_{};
  DbSize size_{};
  schema_key_value *fields_{};
  SchemaHeader(DbPtr name, DbSize size) : name_(name), size_(size) {
    debug("Создаю SchemaHeader", name, size);
    fields_ = reinterpret_cast<schema_key_value *>(new char[GetFlexibleElementSize()]{});
  }
  virtual ~SchemaHeader() { delete[] fields_; }
  [[nodiscard]] DbSize GetOnFileSize() const { return sizeof(raw_schema_header) + GetFlexibleElementSize(); }
  [[nodiscard]] DbSize GetFlexibleElementSize() const {
    // округялем вверх до размера заголовка, чтобы при наличии пустоты на странице в
    // неё всегда мог поместиться заголовок таким образом все схемы будут
    // лежать в памяти плотно
    return (size_ * sizeof(schema_key_value) + sizeof(raw_schema_header) - 1) / sizeof(raw_schema_header) *
           sizeof(raw_schema_header);
  }
  [[nodiscard]] DbSize GetFlexiblePadding() const {
    return size_ * sizeof(schema_key_value) % sizeof(raw_schema_header);
  }
};

#endif  // LLP_INCLUDE_SCHEMAHEADER_H_
