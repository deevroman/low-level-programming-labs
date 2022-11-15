#ifndef EXAMPLES_PAGE_H
#define EXAMPLES_PAGE_H


#include <cstdint>
#include "types.h"

// именно int64_t для поиска в файле
const int64_t SCHEMAS_PAGE_MARKER = 0xAA;
const int64_t NODES_PAGE_MARKER = 0xBB;
const int64_t STRINGS_PAGE_MARKER = 0xCC;

struct __attribute__((packed)) page_header {
    int64_t MAGIC_MARKER;
    db_ptr_t nxt_page;
    db_size_t size;
    db_ptr_t last_elem;

    explicit page_header(int64_t magicMarker) : MAGIC_MARKER(magicMarker) {}

    static page_header make_schemas_page_header() {
        return page_header(SCHEMAS_PAGE_MARKER);
    }

    static page_header make_nodes_page_header() {
        return page_header(NODES_PAGE_MARKER);
    }

    static page_header make_strings_page_header() {
        return page_header(STRINGS_PAGE_MARKER);
    }

};


#endif //EXAMPLES_PAGE_H
