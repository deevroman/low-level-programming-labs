#ifndef LLP_INCLUDE_PAGE_H_
#define LLP_INCLUDE_PAGE_H_

#include "types.h"

#ifdef DEBUG
const DbSize kDefaultPageSize = 1 << 10;
#else
const DbSize kDefaultPageSize = 1 << 12;
#endif
const uint64_t kSchemasPageMarker = 0xAAAAAAAAAAAAAAAA;
const uint64_t kNodesPageMarker = 0xBBBBBBBBBBBBBBBB;
const uint64_t kStringsPageMarker = 0xCCCCCCCCCCCCCCCC;

#pragma pack(push, 1)
struct page_header {
  uint64_t magic_marker{};
  DbSize size = kDefaultPageSize;

  page_header() = default;
  explicit page_header(uint64_t magic_marker) : magic_marker(magic_marker) {}
} PACKED;
#pragma pack(pop)

#pragma pack(push, 1)
template <uint64_t PageMarker>
struct FileChunkedList {
  DbPtr first_element{};
  DbPtr first_free_element{};
  DbSize count{};

  page_header MakePageHeader() { return page_header(PageMarker); }
} PACKED;

#pragma pack(pop)

#endif  // LLP_INCLUDE_PAGE_H_
