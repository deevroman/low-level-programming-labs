#ifndef LLP_INCLUDE_FILE_HEADER_H_
#define LLP_INCLUDE_FILE_HEADER_H_

#include "PageChunk.h"
#include "types.h"

const int64_t kDatabaseFileSignature = 0x4848484848484848;

#pragma pack(push, 1)
struct file_header {
  // сигнатура файла
  int64_t signature{};
  // позиция конца файла
  DbPtr file_end{};
  // количество данных в страницы без учёта заголовков
  DbSize data_summary_size{};
  // позиции первых страниц заданного типа
  FileChunkedList<kSchemasPageMarker> schemas;
  FileChunkedList<kNodesPageMarker> nodes;
  FileChunkedList<kStringsPageMarker> strings;
  DbSize id_counter{1};

  //  DbPtr schemas_first_chunk;
  //  DbPtr schemas_first_free_chunk;
  //  DbPtr nodes_first_chunk;
  //  DbPtr nodes_first_free_chunk;
  //  DbPtr strings_first_chunk;
  //  DbPtr strings_first_free_chunk;

  //  DbPtr schemas_first_page{};  // xxx
  //
  //  DbPtr nodes_first_page{};
  //  DbPtr strings_first_page{};
  // позиции последних страниц заданного типа
  //  DbPtr schemas_last_page{};
  //  DbPtr nodes_last_page{};
  //  DbPtr strings_last_page{};

  file_header() = default;

} PACKED;
#pragma pack(pop)

#endif  // LLP_INCLUDE_FILE_HEADER_H_
