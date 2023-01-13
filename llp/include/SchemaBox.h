#ifndef LLP_INCLUDE_SCHEMABOX_H_
#define LLP_INCLUDE_SCHEMABOX_H_

#include "logger.h"
#include "raw_schema_header.h"
#include "types.h"

class SchemaBox {
 public:
  DbPtr name_{};
  DbSize size_{};
  DbSize cnt_elements_{};
  DbPtr nxt_{};
  schema_key_value *fields_{};

  SchemaBox(DbPtr name, DbSize size, DbPtr nxt) : name_(name), size_(size), nxt_(nxt) {
    debug("Создаю SchemaHeader", name, size);
    fields_ = reinterpret_cast<schema_key_value *>(new char[GetFlexibleElementSize()]{});
  }

  SchemaBox(const SchemaBox &s) {
    debug("Конструктор копирования SchemaBox");
    name_ = s.name_;
    size_ = s.size_;
    cnt_elements_ = s.cnt_elements_;
    nxt_ = s.nxt_;
    fields_ = reinterpret_cast<schema_key_value *>(new char[GetFlexibleElementSize()]{});
    std::copy(s.fields_, s.fields_ + size_, fields_);
  }

  virtual ~SchemaBox() { delete[] fields_; }

  [[nodiscard]] DbSize GetOnFileSize() const { return sizeof(raw_schema_header) + GetFlexibleElementSize(); }

  [[nodiscard]] DbSize GetFlexibleElementSize() const { return size_ * sizeof(schema_key_value); }

  [[nodiscard]] Byte *MakePackedSchema() const {
    Byte *buffer = new Byte[GetOnFileSize()];
    *reinterpret_cast<raw_schema_header *>(buffer) = raw_schema_header(name_, size_, cnt_elements_, nxt_);
    std::copy(fields_, fields_ + size_, (schema_key_value *)((buffer + sizeof(raw_schema_header))));
    return buffer;
  }

  explicit SchemaBox(Byte *buffer) {
    name_ = reinterpret_cast<raw_schema_header *>(buffer)->name;
    size_ = reinterpret_cast<raw_schema_header *>(buffer)->size;
    cnt_elements_ = reinterpret_cast<raw_schema_header *>(buffer)->cnt_elements;
    nxt_ = reinterpret_cast<raw_schema_header *>(buffer)->nxt;
    fields_ = reinterpret_cast<schema_key_value *>(new char[GetFlexibleElementSize()]{});
    auto from = (schema_key_value *)((buffer + sizeof(raw_schema_header)));
    std::copy(from, from + size_, fields_);
  };

#ifdef DEBUG
  bool Validate(FileInterface &f) const {
    for (int i = 0; i < size_; i++) {
      if (!f.calls_ptrs_.contains(fields_[i].key)) {
        debug("Invalid", fields_[i].key, i);
        return false;
      }
    }
    if (f.calls_ptrs_.contains(name_) && f.calls_ptrs_.contains(nxt_)) {
      return true;
    } else {
      debug("Invalid", f.calls_ptrs_.contains(name_), f.calls_ptrs_.contains(nxt_));
      return false;
    }
  }
#endif
};

#endif  // LLP_INCLUDE_SCHEMABOX_H_
