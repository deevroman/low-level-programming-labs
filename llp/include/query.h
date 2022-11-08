#ifndef LLP_QUERY_H
#define LLP_QUERY_H

#include <utility>
#include <variant>
#include <string>
#include <cstdint>
#include <unordered_map>
#include "schemas_page_header.h"
#include "schema.h"


class insert_query {
    std::string path;
    std::string type;
    std::unordered_map<std::string, std::variant<int32_t, float, std::string, bool>> fields;
};

enum query_type {
    CREATE_SCHEMA = 0,
    SHOW_SCHEMAS,
    DELETE_SCHEMA,
    INSERT,
    PRINT,
    UPDATE,
    ERASE
};


typedef schema create_schema_query; // XXX Clion bug?
typedef std::string delete_schema_query;

typedef std::variant<create_schema_query, insert_query> query_payload;

class query {
public:
    query_type type;
    query_payload payload;

    query(query_type type, const query_payload &payload) : type(type), payload(payload) {}
};

class result {

};

#endif //LLP_QUERY_H
