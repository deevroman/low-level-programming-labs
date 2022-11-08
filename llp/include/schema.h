#ifndef EXAMPLES_SCHEMA_H
#define EXAMPLES_SCHEMA_H

#include <string>
#include <unordered_map>
#include "query.h"

class schema {
public:
    std::string name;
    std::unordered_map<std::string, prime_types> fields;
public:
    schema(std::string name, std::unordered_map<std::string, prime_types> fields) : name(std::move(
            name)), fields(std::move(fields)) {}
};


#endif //EXAMPLES_SCHEMA_H
