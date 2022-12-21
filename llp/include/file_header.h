#ifndef LLP_INCLUDE_FILE_HEADER_H_
#define LLP_INCLUDE_FILE_HEADER_H_

#include "types.h"

const int64_t kDatabaseFileSignature = 0x4848484848484848;

#pragma pack(push, 1)
struct file_header {
  // сигнатура файла
  int64_t signature{};
  // позиция конца файла
  DbPtr file_end{};
  // количество данных в страницы. Без учёта заголовков
  DbSize data_summary_size{};
  // позиции первых страниц заданного типа
  DbPtr schemas_first_page{};
  DbPtr nodes_first_page{};
  DbPtr strings_first_page{};
  // позиции последних страниц заданного типа
  DbPtr schemas_last_page{};
  DbPtr nodes_last_page{};
  DbPtr strings_last_page{};
  DbSize schemas_count{};
  DbSize counter{};

  file_header(int64_t signature, DbPtr file_end, DbSize data_summary_size, DbPtr schemas_first_page,
              DbPtr nodes_first_page, DbPtr strings_first_page, DbPtr schemas_last_page, DbPtr nodes_last_page,
              DbPtr strings_last_page)
      : signature(signature),
        file_end(file_end),
        data_summary_size(data_summary_size),
        schemas_first_page(schemas_first_page),
        nodes_first_page(nodes_first_page),
        strings_first_page(strings_first_page),
        schemas_last_page(schemas_last_page),
        nodes_last_page(nodes_last_page),
        strings_last_page(strings_last_page) {}

  file_header() = default;

} PACKED;
#pragma pack(pop)

#endif  // LLP_INCLUDE_FILE_HEADER_H_
