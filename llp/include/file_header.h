#ifndef LLP_FILE_HEADER_H
#define LLP_FILE_HEADER_H

#include <cstdint>
#include "types.h"

const int64_t DATABASE_FILE_SIGNATURE = 0x4848484848484848;

struct __attribute__((packed)) file_header {
    // сигнатура файла
    int64_t SIGNATURE{};
    // позиция конца файла
    db_ptr_t file_end{};
    // количество данных в страницы. Без учёта заголовков
    db_size_t data_summary_size{};
    // позиции первых страниц заданного типа
    db_ptr_t schemas_first_page{};
    db_ptr_t nodes_first_page{};
    db_ptr_t strings_first_page{};
    // позиции последних страниц заданного типа
    db_ptr_t schemas_last_page{};
    db_ptr_t nodes_last_page{};
    db_ptr_t strings_last_page{};

    file_header(const int8_t signature, db_ptr_t fileEnd, db_size_t dataSummarySize, db_ptr_t schemasFirstPage,
                db_ptr_t nodesFirstPage, db_ptr_t stringsFirstPage, db_ptr_t schemasLastPage, db_ptr_t nodesLastPage,
                db_ptr_t stringsLastPage) : SIGNATURE(signature), file_end(fileEnd), data_summary_size(dataSummarySize),
                                            schemas_first_page(schemasFirstPage), nodes_first_page(nodesFirstPage),
                                            strings_first_page(stringsFirstPage), schemas_last_page(schemasLastPage),
                                            nodes_last_page(nodesLastPage), strings_last_page(stringsLastPage) {}

    file_header() {}

};


#endif //LLP_FILE_HEADER_H
