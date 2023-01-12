#ifndef LLP_INCLUDE_SCHEMA_H_
#define LLP_INCLUDE_SCHEMA_H_

#include <map>
#include <string>

#include "Query.h"
#include "types.h"

class Schema {
 public:
  std::string name_;
  std::map<std::string, DataItem> fields_;

 public:
  Schema() = default;

  Schema(std::string name, std::map<std::string, DataItem> fields)
      : name_(std::move(name)), fields_(std::move(fields)) {}
  void Print() {
    std::cout << name_ << "\n";
    for (auto [k, v] : fields_) {
      std::cout << k << ":" << v << "\n";
    }
  }
};

class SchemaWithPosition : public Schema {
 public:
  DbPtr position_{};
  bool Valid(){
    return position_;
  }
};

#endif  // LLP_INCLUDE_SCHEMA_H_
