#ifndef LLP_INCLUDE_QUERY_H_
#define LLP_INCLUDE_QUERY_H_

#include <algorithm>
#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <variant>

#include "Element.h"
#include "Schema.h"

struct insert_query {
  DbSize parent_id;
  std::string type;
  std::map<std::string, DataItem> fields;
  insert_query(DbSize parent_id, std::string type, const std::map<std::string, DataItem> &fields)
      : parent_id(parent_id), type(std::move(type)), fields(fields) {}
};

struct select_query {
  std::string schema_name;
  std::vector<fields_conditional> conditionals;
  [[nodiscard]] bool CheckConditionals(const Element &e, int now_ind = 0) const {
    if (conditionals[now_ind].left_op == 0 and conditionals[now_ind].right_op == 0) {
      return e.CheckConditional(conditionals[now_ind]);
    } else if (conditionals[now_ind].right_op != 0) {
      if (conditionals[now_ind].op == OP_AND) {
        return CheckConditionals(e, conditionals[now_ind].left_op) &&
               CheckConditionals(e, conditionals[now_ind].right_op);
      } else if (conditionals[now_ind].op == OP_OR) {
        return CheckConditionals(e, conditionals[now_ind].left_op) ||
               CheckConditionals(e, conditionals[now_ind].right_op);
      } else {
        error("Invalid binary operator");
      }
    } else {
      return CheckConditionals(e, conditionals[now_ind].left_op);
    }
  }
};

struct update_query {
  select_query selector;
  std::map<std::string, DataItem> new_fields;
};

typedef Schema CreateSchemaQuery;
typedef std::string DeleteSchemaQuery;
typedef DbPtr DeleteQueryByIdQuery;

typedef std::variant<std::vector<Schema>, Schema, std::vector<Element>, Element, int64_t> ResultPayload;

class Result {
 public:
  bool ok_;
  std::string error_;
  ResultPayload payload_;

  Result(bool ok, std::string error) : ok_(ok), error_(std::move(error)) {}

  Result(bool ok, std::string error, ResultPayload payload)
      : ok_(ok), error_(std::move(error)), payload_(std::move(payload)) {}
};

#endif  // LLP_INCLUDE_QUERY_H_
