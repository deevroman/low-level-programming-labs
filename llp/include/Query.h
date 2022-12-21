#ifndef LLP_INCLUDE_QUERY_H_
#define LLP_INCLUDE_QUERY_H_

#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <variant>

#include "Schema.h"

typedef std::variant<int32_t, double, std::string, bool> data_item;

struct Element {
  long long id;
  DbPtr child_link;
  DbPtr next_brother_link;
};

struct insert_query {
  DbSize parent_id;
  std::string type;
  std::map<std::string, data_item> fields;
  insert_query(DbSize parent_id, std::string type, const std::map<std::string, data_item> &fields)
      : parent_id(parent_id), type(std::move(type)), fields(fields) {}
};

enum QueryType { CREATE_SCHEMA = 0, SHOW_SCHEMAS, DELETE_SCHEMA, INSERT, PRINT, UPDATE, ERASE };

typedef Schema create_schema_query;  // XXX Clion bug?
typedef std::string delete_schema_query;

typedef std::variant<create_schema_query, delete_schema_query, insert_query> query_payload;

class Query {
 public:
  QueryType type;
  query_payload payload;

  Query(QueryType type, const query_payload &payload) : type(type), payload(payload) {}
};

typedef std::variant<std::vector<Schema>, Schema> result_payload;

class Result {
 public:
  bool ok_;
  std::string error_;
  result_payload payload_;

  Result(bool ok, const std::string &error) : ok_(ok), error_(error) {}

  Result(bool ok, const std::string &error, const result_payload &payload)
      : ok_(ok), error_(error), payload_(payload) {}
};

#endif  // LLP_INCLUDE_QUERY_H_
