#ifndef LLP_INCLUDE_QUERY_H_
#define LLP_INCLUDE_QUERY_H_

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
  std::map<std::string, data_item> fields;
  insert_query(DbSize parent_id, std::string type, const std::map<std::string, data_item> &fields)
      : parent_id(parent_id), type(std::move(type)), fields(fields) {}
};


typedef Schema create_schema_query;  // XXX Clion bug?
typedef std::string delete_schema_query;
typedef DbPtr delete_query_by_id_query;

typedef std::variant<std::vector<Schema>, Schema, std::vector<Element>, Element, int64_t> result_payload;

class Result {
 public:
  bool ok_;
  std::string error_;
  result_payload payload_;

  Result(bool ok, std::string error) : ok_(ok), error_(std::move(error)) {}

  Result(bool ok, std::string error, result_payload payload)
      : ok_(ok), error_(std::move(error)), payload_(std::move(payload)) {}
};

#endif  // LLP_INCLUDE_QUERY_H_
