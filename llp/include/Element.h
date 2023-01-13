#ifndef LLP_INCLUDE_ELEMENT_H_
#define LLP_INCLUDE_ELEMENT_H_

#include <map>
#include <string>
#include <variant>

#include "logger.h"
#include "raw_element_header.h"
#include "types.h"

typedef std::variant<int32_t, double, std::string, bool> DataItem;

enum Operator { OP_EQUAL, OP_NOT_EQUAL, OP_GREATER, OP_LESS };

template <typename T>
bool EvalOperator(Operator op, T a, T b) {
  switch (op) {
    case OP_EQUAL:
      return a == b;
    case OP_NOT_EQUAL:
      return a != b;
    case OP_LESS:
      return a < b;
    case OP_GREATER:
      return a > b;
    default:
      error("Unsupported operator");
  }
}

struct fields_conditional {
  std::string field_name;
  Operator op;
  DataItem value;
};

class Element {
 public:
  DbSize id_{};
  DbPtr schema_{};
  std::map<std::string, DataItem> fields_;

 public:
  Element() = default;

  Element(DbSize id, DbPtr schema, std::map<std::string, DataItem> fields)
      : id_(id), schema_(schema), fields_(std::move(fields)) {}

  [[nodiscard]] bool CheckConditional(fields_conditional cond) const {
    auto v = fields_.at(cond.field_name);
    if (v.index() == DB_STRING) {
      return EvalOperator(cond.op, get<std::string>(v), get<std::string>(cond.value));
    } else if (v.index() == DB_INT_32) {
      return EvalOperator(cond.op, get<int32_t>(v), get<int32_t>(cond.value));
    } else if (v.index() == DB_BOOL) {
      return EvalOperator(cond.op, get<bool>(v), get<bool>(cond.value));
    } else if (v.index() == DB_DOUBLE) {
      return EvalOperator(cond.op, get<double>(v), get<double>(cond.value));
    }
    error("Unknown type");
  }

  void Print() {
    std::cout << "id: " << id_ << "\n";
    std::cout << "schema: " << schema_ << "\n";
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
  DbPtr position_{};
};

#endif  // LLP_INCLUDE_ELEMENT_H_
