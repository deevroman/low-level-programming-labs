#ifndef LLP_DATABASE_H
#define LLP_DATABASE_H

#include <string>
#include <fcntl.h>
#include "logger.h"
#include "file_interface.h"
#include "file_header.h"
#include "schemas_page_header.h"
#include "nodes_page_header.h"
#include "strings_page_header.h"
#include "query.h"

const db_size_t DEFAULT_SCHEMAS_PAGE_SIZE = 1 << 6;
const db_size_t DEFAULT_NODES_PAGE_SIZE = 1 << 6;
const db_size_t DEFAULT_STRINGS_PAGE_SIZE = 1 << 6;

class database {
public: // XXX
    file_interface *file;
    file_header header;


    virtual ~database() {
        delete file;
    }

    bool load() {
        if (!file->read(&header, sizeof(file_header), 0)) {
            return false;
        }
        if (header.SIGNATURE != DATABASE_FILE_SIGNATURE) {
            return false;
        }
        debug(header.SIGNATURE);
        debug(header.file_end);
        debug(header.schemas_first_page);
        debug(header.nodes_first_page);
        debug(header.strings_first_page);
        debug(header.schemas_last_page);
        debug(header.nodes_last_page);
        debug(header.strings_last_page);
        return true;
    }

    void init() {
        header.SIGNATURE = DATABASE_FILE_SIGNATURE;
        header.file_end =
                sizeof(file_header);
        header.data_summary_size = 0;

        // заголовок
        file->write(&header, sizeof(header), 0);
        // страница под схемы
        header.schemas_first_page = sizeof(file_header);
        header.schemas_last_page = header.schemas_first_page;
        alloc_schemas_page();


        header.nodes_first_page = sizeof(file_header) + sizeof(schemas_page_header) + DEFAULT_SCHEMAS_PAGE_SIZE;
        header.nodes_last_page = header.nodes_first_page;
        alloc_nodes_page();

        header.strings_first_page =
                sizeof(file_header) + sizeof(schemas_page_header) + DEFAULT_SCHEMAS_PAGE_SIZE +
                sizeof(nodes_page_header) + DEFAULT_NODES_PAGE_SIZE;
        header.strings_last_page = header.strings_first_page;
        alloc_strings_page();

        file->write(&header, sizeof(header), 0);
    }

    void alloc_schemas_page(db_size_t size = -1) {
        if (size == -1) {
            size = DEFAULT_SCHEMAS_PAGE_SIZE;
        }
        schemas_page_header h1 = schemas_page_header(0, size, header.file_end);
        file->write(&h1, sizeof(schemas_page_header), header.file_end);

        void *zeros = calloc(size, 1);
        file->write(zeros, size);
        free(zeros);

        header.file_end += sizeof(schemas_page_header) + size;
    }

    void alloc_nodes_page(db_size_t size = -1) {
        if (size == -1) {
            size = DEFAULT_NODES_PAGE_SIZE;
        }
        nodes_page_header h1 = nodes_page_header(0, size, header.file_end);
        file->write(&h1, sizeof(nodes_page_header), header.file_end);

        void *zeros = calloc(size, 1);
        file->write(zeros, size);
        free(zeros);

        header.file_end += sizeof(nodes_page_header) + size;
    }

    void alloc_strings_page(db_size_t size = -1) {
        if (size == -1) {
            size = DEFAULT_STRINGS_PAGE_SIZE;
        }
        strings_page_header h1 = strings_page_header(0, size, header.file_end);
        file->write(&h1, sizeof(strings_page_header), header.file_end);

        void *zeros = calloc(size, 1);
        file->write(zeros, size);
        free(zeros);

        header.file_end += sizeof(strings_page_header) + size;
    }

    result create_schema(const create_schema_query args) {

    }

public:
    explicit database(const std::string &file_path, bool overwrite = false) {
        file = new file_interface(file_path, overwrite);
        if (overwrite) {
            init();
        } else {
            if (!load()) {
                debug("База повреждена");
                return; // XXX
            }
        }
    }

    result query(const query &q) {
        switch (q.type) {
            case CREATE_SCHEMA:
                return create_schema(std::get<create_schema_query>(q.payload));
            case SHOW_SCHEMAS:
                break;
            case DELETE_SCHEMA:
                break;
            case INSERT:
                break;
            case PRINT:
                break;
            case UPDATE:
                break;
            case ERASE:
                break;
            default:
                debug("bred"); // XXX
        }
        debug("bred");
    }
};


#endif //LLP_DATABASE_H
