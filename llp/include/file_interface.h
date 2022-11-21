#ifndef EXAMPLES_FILE_INTERFACE_H
#define EXAMPLES_FILE_INTERFACE_H


#include <string>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <sys/stat.h>
#include <string>
#include <csignal>
#include <vector>
#include "logger.h"
#include "types.h"


class file_interface {
public: // XXX
    FILE *fd;
public:
    virtual ~file_interface() {
        if (fd != nullptr) {
            fclose(fd);
        }
    }

    explicit file_interface(const std::string &file_path, bool overwrite) { // XXX
        if (overwrite) {
            fd = fopen(file_path.c_str(), "w");
            fclose(fd);
        }
        fd = fopen(file_path.c_str(), "r+");
        if (fd == nullptr) {
            // XXX
            debug(file_path, "не открыт", fd);
        }
        debug("Файл базы открыт:", file_path);
    }


    virtual void write(void *ptr, size_t size, long long position = -1) {
        if (position != -1) {
            fseek(fd, position, SEEK_SET);
        }
        if (fwrite(ptr, size, 1, fd) != 1) {
            debug("not wrote");
        }
        debug("write", position, size);
    }

    virtual bool read(void *buffer, size_t size, long long position) {
        debug("read", position, size);
        fseek(fd, position, SEEK_SET);
        return fread(buffer, size, 1, fd) == 1;
    }
};

class file_in_memory_interface : public file_interface {
public: // XXX
    byte *memory;
    long long seek{};
public:
    file_in_memory_interface(const std::string &filePath, bool overwrite) : file_interface(filePath, overwrite) {
        memory = static_cast<byte *>(calloc(10050048, sizeof(byte)));
    }

    void write(void *ptr, size_t size, const db_ptr_t * position = reinterpret_cast<db_ptr_t *>(-1)) {
        if (position == reinterpret_cast<db_ptr_t *>(-1)) {
            memcpy((byte *) memory + seek, ptr, size);
            seek += size;
        } else {
            seek = reinterpret_cast<long long int>(position);
            memcpy((byte *) memory + seek, ptr, size);
            seek += size;
        }
    }

    virtual bool read(void *buffer, size_t size, db_ptr_t *position) {
        if (position == reinterpret_cast<db_ptr_t *>(-1)) {
            debug("OH NO");
//            memcpy(buffer, memory + seek, size);
//            seek += size;
            return false;
        } else {
            seek = reinterpret_cast<long long int>(position);
            memcpy(buffer, (std::byte *) memory + seek, size);
            seek += size;
        }
        return true;
    }
};


#endif //EXAMPLES_FILE_INTERFACE_H
