#ifndef LLP_INCLUDE_PAGECHUNK_H_
#define LLP_INCLUDE_PAGECHUNK_H_

#include "page.h"
#include "types.h"

const DbSize kDefaultChunkSize = sizeof(DbPtr) * 16;
#ifdef DEBUG
const DbSize kChunkDataSize = kDefaultChunkSize - CalcPadding(sizeof(DbPtr)) - sizeof(DbPtr) - 16;
#else
const DbSize kChunkDataSize = sizeof(DbPtr);
#endif

#pragma pack(push, 1)
struct PageChunk {
  PAD(DbPtr, nxt_chunk){};
  Byte data[kChunkDataSize]{};
#ifdef DEBUG
  bool validate(FileInterface &f){
    return f.calls_ptrs_.contains(nxt_chunk);
  }
#endif
} PACKED;
#pragma pack(pop)

static_assert(kDefaultPageSize % sizeof(PageChunk) == 0);

#endif  // LLP_INCLUDE_PAGECHUNK_H_
