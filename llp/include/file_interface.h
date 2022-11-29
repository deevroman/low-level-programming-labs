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

class abstract_storage {
public:
    virtual ~abstract_storage() = default;

private:
    virtual void write(void *ptr, size_t size, const db_ptr_t *position = reinterpret_cast<db_ptr_t *>(-1)) = 0;

    virtual bool read(void *buffer, size_t size, db_ptr_t *position) = 0;

};

class file_interface : public abstract_storage {
public: // XXX
    FILE *fd;
public:
    ~file_interface() override {
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
            throw "Файл с базой не открыт";
        }
        debug("Файл базы открыт:", file_path);
    }


    void write(void *ptr, size_t size, const db_ptr_t *position = reinterpret_cast<db_ptr_t *>(-1)) override {
        if (position != reinterpret_cast<db_ptr_t *>(-1)) {
            fseek(fd, (intptr_t) position, SEEK_SET);
        }
        if (fwrite(ptr, size, 1, fd) != 1) {
            debug("not wrote");
        }
        debug("write", (uintptr_t) position, size);
    }

    bool read(void *buffer, size_t size, db_ptr_t *position) override {
        debug("read", (uintptr_t) position, size);
        fseek(fd, (intptr_t) position, SEEK_SET);
        return fread(buffer, size, 1, fd) == 1;
    }
};


#endif //EXAMPLES_FILE_INTERFACE_H
