#ifndef LLP_INCLUDE_FILEINTERFACE_H_
#define LLP_INCLUDE_FILEINTERFACE_H_

#include <cstring>
#include <string>
#include <set>
#include <utility>


#include "logger.h"
#include "types.h"

#define __builtin_FILE() (strrchr(__FILE__, '/') ? strrchr(__builtin_FILE(), '/') + 1 : __builtin_FILE())

class FileInterface {
 public:
  FILE *fd_;
#ifdef DEBUG
  mutable std::set<std::pair<DbPtr, DbSize>> calls_;
  mutable std::set<DbPtr> calls_ptrs_;
#endif

 public:
  ~FileInterface() {
    if (fd_ != nullptr) {
      fclose(fd_);
    }
  }

  explicit FileInterface(const std::string &file_path, bool overwrite) {  // XXX
    if (overwrite) {
      fd_ = fopen(file_path.c_str(), "w");
      if (fd_ != nullptr) {
        fclose(fd_);
      }
    }
    fd_ = fopen(file_path.c_str(), "rb+");
    if (fd_ == nullptr) {
      error("Файл с базой не открыт");
    }
    debug("Файл базы открыт", file_path);
  }

#ifdef DEBUG
  void Write(void *ptr, size_t size, DbPtr position = -1, const char *caller = __builtin_FUNCTION(),
             const char *file = __builtin_FILE(), int line = __builtin_LINE()) const {
    std::string from = std::string(file) + ":" + std::to_string(line);
    debug("Write", position, size, from, caller);
    if (position == 0 && std::string(caller) != "UpdateMasterHeader") {
      error("0 position not from UpdateMasterHeader");
    }
    calls_.insert({position, size});
    calls_ptrs_.insert(position);
#else
  void Write(void *ptr, size_t size, const DbPtr position = -1) {
#endif
    if (position != -1) {
      fseek(fd_, position, SEEK_SET);
    }
    if (fwrite(ptr, size, 1, fd_) != 1) {
      error("not wrote");
    }
#ifdef DEBUG
    fflush(fd_);
#endif
  }

#ifdef DEBUG
  void Read(void *buffer, size_t size, DbPtr position = -1, const char *caller = __builtin_FUNCTION(),
            const char *file = __builtin_FILE(), int line = __builtin_LINE()) const {
    std::string from = std::string(file) + ":" + std::to_string(line);
    debug("Read", position, size, from, caller);
    debug_assert(position >= 0);
    if(!calls_.contains({position, size})){
      error("Bad read");
    }
#else
  void Read(void *buffer, size_t size, DbPtr position = -1) {
#endif
    if (position != -1) {
      fseek(fd_, position, SEEK_SET);
    }
    if (fread(buffer, size, 1, fd_) != 1) {
      error("not Read");
    }
  }
};

#endif  // LLP_INCLUDE_FILEINTERFACE_H_
