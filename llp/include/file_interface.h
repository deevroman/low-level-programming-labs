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


class file_interface {
public: // XXX
    FILE *file;
public:
    virtual ~file_interface() {
        if (file != nullptr) {
            fclose(file);
        }
    }

    explicit file_interface(const std::string &file_path, bool overwrite) { // XXX
        if (overwrite) {
            file = fopen(file_path.c_str(), "w");
            fclose(file);
        }
        file = fopen(file_path.c_str(), "r+");
        if (file == nullptr) {
            // XXX
            debug(file_path, "не открыт", file);
        }
        debug("Файл базы открыт:", file_path);
    }


    void write(void *ptr, size_t size, long long position = -1) {
        if (position != -1) {
            fseek(file, position, SEEK_SET);
        }
        if (fwrite(ptr, size, 1, file) != 1) {
            debug("not wrote");
        }
        debug("write", position, size);
    }

    bool read(void *buffer, size_t size, long long position) {
        debug("read", position, size);
        fseek(file, position, SEEK_SET);
        return fread(buffer, size, 1, file) == 1;
    }
};


#endif //EXAMPLES_FILE_INTERFACE_H
