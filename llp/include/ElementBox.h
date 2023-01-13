#ifndef LLP_INCLUDE_ELEMENTBOX_H_
#define LLP_INCLUDE_ELEMENTBOX_H_

#include "FileInterface.h"
#include "logger.h"
#include "raw_element_header.h"
#include "types.h"

class ElementBox {
 public:
  DbSize size_{};
  DbPtr schema_{};
  DbPtr parent_link_{};
  DbPtr child_link_{};
  DbPtr prev_brother_link_{};
  DbPtr brother_link_{};
  element_value *fields_{};

  ElementBox(DbSize size, DbPtr schema, DbPtr parent_link, DbPtr child_link)
      : size_(size), schema_(schema), parent_link_(parent_link), child_link_(child_link) {
    debug("Создаю SchemaHeader", size);
    fields_ = reinterpret_cast<element_value *>(new char[GetFlexibleElementSize()]{});
  }

  ElementBox(const ElementBox& e) {
    debug("Конструктор копирования ElementBox");
    schema_ = e.schema_;
    parent_link_ = e.parent_link_;
    child_link_ = e.child_link_;
    prev_brother_link_ = e.prev_brother_link_;
    brother_link_ = e.brother_link_;
    fields_ = reinterpret_cast<element_value *>(new char[GetFlexibleElementSize()]{});
    std::copy(e.fields_, e.fields_ + size_, fields_);
  }

  virtual ~ElementBox() { delete[] fields_; }

  [[nodiscard]] DbSize GetOnFileSize() const { return sizeof(raw_element_header) + GetFlexibleElementSize(); }

  [[nodiscard]] DbSize GetFlexibleElementSize() const { return size_ * sizeof(element_value); }

  [[nodiscard]] Byte *MakePackedElement() const {
    Byte *buffer = new Byte[GetOnFileSize()];
    *reinterpret_cast<raw_element_header *>(buffer) =
        raw_element_header(size_, schema_, parent_link_, child_link_, prev_brother_link_, brother_link_);
    std::copy(fields_, fields_ + size_, (element_value *)((buffer + sizeof(raw_element_header))));
    return buffer;
  }

  explicit ElementBox(Byte *buffer) {
    size_ = reinterpret_cast<raw_element_header *>(buffer)->size;
    schema_ = reinterpret_cast<raw_element_header *>(buffer)->schema;
    parent_link_ = reinterpret_cast<raw_element_header *>(buffer)->parent_link;
    child_link_ = reinterpret_cast<raw_element_header *>(buffer)->child_link;
    prev_brother_link_ = reinterpret_cast<raw_element_header *>(buffer)->pr_brother_link;
    brother_link_ = reinterpret_cast<raw_element_header *>(buffer)->brother_link;
    fields_ = reinterpret_cast<element_value *>(new char[GetFlexibleElementSize()]{});
    auto from = (element_value *)((buffer + sizeof(raw_element_header)));
    std::copy(from, from + size_, fields_);
  };
};

#endif  // LLP_INCLUDE_ELEMENTBOX_H_
