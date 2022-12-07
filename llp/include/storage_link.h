#ifndef LLP_STORAGE_LINK_H
#define LLP_STORAGE_LINK_H

#include "types.h"

class storage_link {
 public:
  DbPtr address;
  // maybe [page, offset] for page moving  
};

#endif //LLP_STORAGE_LINK_H
