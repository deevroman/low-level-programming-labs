#ifndef LLP_INCLUDE_ELEMENT_H_
#define LLP_INCLUDE_ELEMENT_H_

#include <map>
#include <string>

#include "raw_element_header.h"
#include "types.h"

typedef std::variant<int32_t, double, std::string, bool> data_item;

class Element {
 public:
  DbSize id_{};
  std::map<std::string, data_item> fields_;

 public:
  Element() = default;

  Element(DbSize id, std::map<std::string, data_item> fields)
      : id_(id), fields_(std::move(fields)) {}
  void Print() {
    std::cout << id_ << "\n";
    for (auto [k, v] : fields_) {
      std::cout << k << ":";
      if (v.index() == DB_STRING) {
        std::cout << get<std::string>(v);
      } else if (v.index() == DB_INT_32) {
        std::cout << get<int32_t>(v);
      } else if (v.index() == DB_BOOL) {
        std::cout << get<bool>(v);
      } else if (v.index() == DB_DOUBLE) {
        std::cout << get<double>(v);
      }
      std::cout << "\n";
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
