#ifndef LLP_INCLUDE_SCHEMA_H_
#define LLP_INCLUDE_SCHEMA_H_

#include <string>
#include <map>
#include "Query.h"

class Schema {
 public:
  std::string name_;
  std::map<std::string, PrimeTypes> fields_;
 public:
  Schema() = default;

  Schema(std::string name, std::map<std::string, PrimeTypes> fields) : name_(std::move(name)),
                                                                       fields_(std::move(fields)) {}
  void Print() {
    std::cout << name_ << "\n";
    for (auto [k, v] : fields_) {
      std::cout << k << ":" << v << "\n";
    }
  }
};

#endif //LLP_INCLUDE_SCHEMA_H_
