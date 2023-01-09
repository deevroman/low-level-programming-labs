#ifndef EXAMPLES_FILE_INTERFACE_H
#define EXAMPLES_FILE_INTERFACE_H

#include <string>
// #include <sys/fcntl.h>
// #include <sys/stat.h>
// #include <fcntl.h>
// #include <sys/types.h>
// #include <unistd.h>
#include <cstring>
// #include <csignal>
#include "logger.h"
#include "types.h"

#define __builtin_FILE() (strrchr(__FILE__, '/') ? strrchr(__builtin_FILE(), '/') + 1 : __builtin_FILE())

class AbstractStorage {
 public:
  virtual ~AbstractStorage() = default;

 private:
#ifdef DEBUG
  virtual void write(void *ptr, size_t size, const DbPtr position = -1, const char *caller = __builtin_FUNCTION(),
                     const char *file = __builtin_FILE(), int line = __builtin_LINE()) = 0;
#else
  virtual void write(void *ptr, size_t size, const DbPtr position = -1) = 0;
#endif

#ifdef DEBUG
  virtual void read(void *buffer, size_t size, DbPtr position, const char *caller = __builtin_FUNCTION(),
                    const char *file = __builtin_FILE(), int line = __builtin_LINE()) = 0;
#else
  virtual void read(void *buffer, size_t size, DbPtr position) = 0;
#endif
};

class FileInterface : public AbstractStorage {
 public:  // XXX
  FILE *fd_;

 public:
  ~FileInterface() override {
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
    fd_ = fopen(file_path.c_str(), "r+");
    if (fd_ == nullptr) {
      error("Файл с базой не открыт");
    }
    debug("Файл базы открыт", file_path);
  }

#ifdef DEBUG
  void write(void *ptr, size_t size, const DbPtr position = -1, const char *caller = __builtin_FUNCTION(),
             const char *file = __builtin_FILE(), int line = __builtin_LINE()) override {
    std::string from = std::string(file) + ":" + std::to_string(line);
    debug("write", position, size, from, caller);
    if (position == 0 && std::string(caller) != "UpdateMasterHeader"){
      error("0 position not from UpdateMasterHeader");
    }
#else
  void write(void *ptr, size_t size, const DbPtr position = -1) override {
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
  void read(void *buffer, size_t size, DbPtr position = -1, const char *caller = __builtin_FUNCTION(),
            const char *file = __builtin_FILE(), int line = __builtin_LINE()) override {
    std::string from = std::string(file) + ":" + std::to_string(line);
    debug("read", position, size, from, caller);
#else
  void read(void *buffer, size_t size, DbPtr position = -1) override {
#endif
    if (position != -1) {
      fseek(fd_, position, SEEK_SET);
    }
    if (fread(buffer, size, 1, fd_) != 1) {
      error("not read");
    }
  }
};

#endif  // EXAMPLES_FILE_INTERFACE_H
