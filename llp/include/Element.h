#ifndef LLP_INCLUDE_ELEMENT_H_
#define LLP_INCLUDE_ELEMENT_H_

#include <map>
#include <string>

#include "raw_element_header.h"
#include "types.h"

class Element {
 public:
  DbSize id_{};
  std::map<std::string, PrimeTypes> fields_;

 public:
  Element() = default;

  Element(DbSize id, std::map<std::string, PrimeTypes> fields)
      : id_(id), fields_(std::move(fields)) {}
  void Print() {
    std::cout << id_ << "\n";
    for (auto [k, v] : fields_) {
      std::cout << k << ":" << v << "\n";
    }
  }
};

class ElementWithPosition : public Element {
 public:
  DbPtr page_start_{};
  DbPtr position_{};
  DbSize index_{};
};

#endif  // LLP_INCLUDE_ELEMENT_H_
