#ifndef LLP_INCLUDE_PAGE_H_
#define LLP_INCLUDE_PAGE_H_

#include "types.h"

const DbSize kDefaultPageSize = 1 << 10;
const uint64_t kSchemasPageMarker = 0xAAAAAAAAAAAAAAAA;
const uint64_t kNodesPageMarker = 0xBBBBBBBBBBBBBBBB;
const uint64_t kStringsPageMarker = 0xCCCCCCCCCCCCCCCC;

#pragma pack(push, 1)
struct page_header {
  page_header() = default;

  uint64_t magic_marker{};
  DbPtr nxt_page{0}; // TODO
  DbSize size = kDefaultPageSize;
//  DbSize ind_last_elem{0};

  explicit page_header(uint64_t magic_marker) : magic_marker(magic_marker) {}

  //  [[nodiscard]] DbSize GetFreeSpace() const {
  // #ifdef DEBUG
  //    assert(size >= ind_last_elem);
  // #endif
  //    return size - ind_last_elem;
  //  }

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
