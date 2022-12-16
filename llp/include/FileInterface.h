#ifndef EXAMPLES_FILE_INTERFACE_H
#define EXAMPLES_FILE_INTERFACE_H

#include <string>
//#include <sys/fcntl.h>
//#include <sys/stat.h>
//#include <fcntl.h>
//#include <sys/types.h>
//#include <unistd.h>
#include <cstring>
//#include <csignal>
#include "logger.h"
#include "types.h"

class AbstractStorage {
 public:
  virtual ~AbstractStorage() = default;

 private:
  virtual void write(void *ptr, size_t size, const DbPtr position = -1) = 0;

  virtual bool read(void *buffer, size_t size, DbPtr position) = 0;

};

class FileInterface : public AbstractStorage {
 public: // XXX
  FILE *fd_;
 public:
  ~FileInterface() override {
    if (fd_ != nullptr) {
      fclose(fd_);
    }
  }

  explicit FileInterface(const std::string &file_path, bool overwrite) { // XXX
    if (overwrite) {
      fd_ = fopen(file_path.c_str(), "w");
      if (fd_ != nullptr){
        fclose(fd_);
      }
    }
    fd_ = fopen(file_path.c_str(), "r+");
    if (fd_ == nullptr) {
      error("Файл с базой не открыт");
    }
    debug("Файл базы открыт:", file_path);
  }

  void write(void *ptr, size_t size, const DbPtr position = -1) override {
    if (position != -1) {
      fseek(fd_, position, SEEK_SET);
    }
    if (fwrite(ptr, size, 1, fd_) != 1) {
      debug("not wrote");
    }
#ifdef DEBUG
    fflush(fd_);
#endif
    debug("write", position, size);
  }

  bool read(void *buffer, size_t size, DbPtr position = -1) override {
    debug("read", position, size);
    if (position != -1) {
      fseek(fd_, position, SEEK_SET);
    }
    return fread(buffer, size, 1, fd_) == 1;
  }
};

#endif //EXAMPLES_FILE_INTERFACE_H
