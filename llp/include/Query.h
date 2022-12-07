#ifndef LLP_QUERY_H
#define LLP_QUERY_H

#include <variant>
#include <string>
#include <cstdint>
#include <map>
#include "Schema.h"

class insert_query {
  std::string path;
  std::string type;
  std::map<std::string, std::variant<int32_t, float, std::string, bool>> fields;
};

enum QueryType {
  CREATE_SCHEMA = 0,
  SHOW_SCHEMAS,
  DELETE_SCHEMA,
  INSERT,
  PRINT,
  UPDATE,
  ERASE
};

typedef Schema create_schema_query; // XXX Clion bug?
typedef std::string delete_schema_query;

typedef std::variant<create_schema_query, insert_query> query_payload;

class Query {
 public:
  QueryType type;
  query_payload payload;

  Query(QueryType type, const query_payload &payload) : type(type), payload(payload) {}
};

typedef std::variant<std::vector<Schema>> result_payload;

class Result {
 public:
  bool ok_;
  std::string error_;
  result_payload payload_;

  Result(bool ok, const std::string &error) : ok_(ok), error_(error) {}

  Result(bool ok, const std::string &error, const result_payload &payload)
      : ok_(ok), error_(error), payload_(payload) {}
};

#endif //LLP_QUERY_H
