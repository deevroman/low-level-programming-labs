#ifndef LLP_NODES_PAGE_HEADER_H
#define LLP_NODES_PAGE_HEADER_H


#include "types.h"

struct __attribute__((packed)) nodes_page_header {
    int64_t MAGIC_MARKER = 0xAA;
    db_ptr_t nxt_page;
    db_size_t size;
    db_ptr_t last_elem;

    nodes_page_header(db_ptr_t nxtPage, db_size_t size, db_ptr_t lastElem) : nxt_page(nxtPage), size(size),
                                                                             last_elem(lastElem) {}
};

#endif //LLP_NODES_PAGE_HEADER_H
