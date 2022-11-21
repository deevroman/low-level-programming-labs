#ifndef EXAMPLES_PAGE_H
#define EXAMPLES_PAGE_H


#include "types.h"

const db_size_t DEFAULT_PAGE_SIZE = 1 << 8;
// именно int64_t для поиска в файле
const int64_t SCHEMAS_PAGE_MARKER = 0xAA;
const int64_t NODES_PAGE_MARKER = 0xBB;
const int64_t STRINGS_PAGE_MARKER = 0xCC;

struct __attribute__((packed)) page_header {
    page_header() {

    }

    int64_t MAGIC_MARKER;
    db_ptr_t nxt_page;
    db_size_t size;
    db_size_t ind_last_elem;

    explicit page_header(int64_t magicMarker) : MAGIC_MARKER(magicMarker) {}

    page_header(int64_t magicMarker, db_ptr_t nxtPage, db_size_t size, db_ptr_t lastElem) : MAGIC_MARKER(magicMarker),
                                                                                            nxt_page(nxtPage),
                                                                                            size(size),
                                                                                            ind_last_elem(lastElem) {}

    db_size_t get_free_space() {
        return size - ind_last_elem;
    }
};

static page_header make_schemas_page_header() {
    return page_header(SCHEMAS_PAGE_MARKER);
}

static page_header make_nodes_page_header() {
    return page_header(NODES_PAGE_MARKER);
}

static page_header make_strings_page_header() {
    return page_header(STRINGS_PAGE_MARKER);
}

#endif //EXAMPLES_PAGE_H
