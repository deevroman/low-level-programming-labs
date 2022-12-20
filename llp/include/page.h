#ifndef LLP_INCLUDE_PAGE_H_
#define LLP_INCLUDE_PAGE_H_

#include "types.h"

const DbSize kDefaultPageSize = 1 << 10;
const uint64_t kSchemasPageMarker = 0xAAAAAAAAAAAAAAAA;
const uint64_t kNodesPageMarker = 0xBBBBBBBBBBBBBBBB;
const uint64_t kStringsPageMarker = 0xCCCCCCCCCCCCCCCC;

struct page_header {
  page_header() = default;

  uint64_t magic_marker{};
  DbPtr nxt_page{0};
  DbSize size = kDefaultPageSize;
  DbSize ind_last_elem{0};

  explicit page_header(uint64_t magic_marker) : magic_marker(magic_marker) {}

  [[nodiscard]] DbSize GetFreeSpace() const {
#ifdef DEBUG
    assert(size >= ind_last_elem);
#endif
    return size - ind_last_elem;
  }

} PACKED;

static page_header MakeSchemasPageHeader() { return page_header(kSchemasPageMarker); }

static page_header MakeNodesPageHeader() { return page_header(kNodesPageMarker); }

static page_header MakeStringsPageHeader() { return page_header(kStringsPageMarker); }

#endif  // LLP_INCLUDE_PAGE_H_
