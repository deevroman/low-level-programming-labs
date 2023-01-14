#ifndef LLP_INCLUDE_SCHEMA_H_
#define LLP_INCLUDE_SCHEMA_H_

#include <map>
#include <string>

#include "Query.h"
#include "types.h"

class Schema {
 public:
  std::string name_;
  int64_t cnt_elements_;
  std::map<std::string, DataItemType> fields_;

 public:
  Schema() = default;

  Schema(std::string name, std::map<std::string, DataItemType> fields)
      : name_(std::move(name)), fields_(std::move(fields)) {}
  void Print() {
    std::cout << name_ << "\n";
    for (auto [k, v] : fields_) {
      std::cout << k << ":" << v << "\n";
    }
  }
  DbSize GetElementPackedSize() { return sizeof(raw_element_header) + fields_.size() * sizeof(element_value); }
};

class SchemaWithPosition : public Schema {
 public:
  DbPtr position_{};
  bool Valid() { return position_; }
};

#endif  // LLP_INCLUDE_SCHEMA_H_
