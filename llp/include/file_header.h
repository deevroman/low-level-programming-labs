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
  // позиции первых страниц заданного типа
  FileChunkedList<kSchemasPageMarker> schemas;
  FileChunkedList<kNodesPageMarker> elements;
  FileChunkedList<kStringsPageMarker> strings;
#ifdef DEBUG
  DbPtr pad{};
#endif
  file_header() = default;

} PACKED;
#pragma pack(pop)

#endif  // LLP_INCLUDE_FILE_HEADER_H_
