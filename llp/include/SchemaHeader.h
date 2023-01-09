#ifndef LLP_INCLUDE_SCHEMAHEADER_H_
#define LLP_INCLUDE_SCHEMAHEADER_H_

#include "logger.h"
#include "raw_schema_header.h"
#include "types.h"

class SchemaHeader {
 public:
  DbPtr name_{};
  DbSize size_{};
  DbPtr nxt_{};
  schema_key_value *fields_{};

  SchemaHeader(DbPtr name, DbSize size, DbPtr nxt) : name_(name), size_(size), nxt_(nxt) {
    debug("Создаю SchemaHeader", name, size);
    fields_ = reinterpret_cast<schema_key_value *>(new char[GetFlexibleElementSize()]{});
  }

  virtual ~SchemaHeader() { delete[] fields_; }

  [[nodiscard]] DbSize GetOnFileSize() const { return sizeof(raw_schema_header) + GetFlexibleElementSize(); }

  [[nodiscard]] DbSize GetFlexibleElementSize() const { return size_ * sizeof(schema_key_value); }

  Byte *MakePackedSchema() const {
    Byte *buffer = new Byte[GetOnFileSize()];
    *reinterpret_cast<raw_schema_header *>(buffer) = raw_schema_header(name_, size_, nxt_);
    std::copy(fields_, fields_ + size_, (schema_key_value *)((buffer + sizeof(raw_schema_header))));
    return buffer;
  }

  SchemaHeader(Byte *buffer) {
    name_ = reinterpret_cast<raw_schema_header *>(buffer)->name;
    size_ = reinterpret_cast<raw_schema_header *>(buffer)->size;
    nxt_ = reinterpret_cast<raw_schema_header *>(buffer)->nxt;
    fields_ = reinterpret_cast<schema_key_value *>(new char[GetFlexibleElementSize()]{});
    auto from = (schema_key_value *)((buffer + sizeof(raw_schema_header)));
    std::copy(from, from + size_, fields_);
  };
};

#endif  // LLP_INCLUDE_SCHEMAHEADER_H_
