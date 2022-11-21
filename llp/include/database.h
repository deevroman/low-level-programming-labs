#ifndef LLP_DATABASE_H
#define LLP_DATABASE_H

#include <typeinfo>
#include <type_traits>
#include <string>
#include <fcntl.h>
#include "logger.h"
#include "file_interface.h"
#include "file_header.h"
#include "query.h"
#include "page.h"

#ifndef DEBUG
template<class T_file_interface>
#endif

class database {
public:
#ifdef DEBUG
    file_in_memory_interface *file;
#else
    T_file_interface *file;
#endif
    file_header master_header;

    constexpr bool is_in_file() {
#ifdef DEBUG
        return false;
#else
        return std::is_same_v<T_file_interface, file_interface>;
#endif
    }

    virtual ~database() {
        delete file;
    }

    bool load() {
        if (!file->read(&master_header, sizeof(file_header), 0)) {
            return false;
        }
        if (master_header.SIGNATURE != DATABASE_FILE_SIGNATURE) {
            return false;
        }
        debug(master_header.SIGNATURE);
        debug(master_header.file_end);
        debug(master_header.schemas_first_page);
        debug(master_header.nodes_first_page);
        debug(master_header.strings_first_page);
        debug(master_header.schemas_last_page);
        debug(master_header.nodes_last_page);
        debug(master_header.strings_last_page);
        return true;
    }

    void init() {
        master_header.SIGNATURE = DATABASE_FILE_SIGNATURE;
        master_header.file_end =
                reinterpret_cast<db_ptr_t *>(sizeof(file_header));
        master_header.data_summary_size = 0;

        // заголовок
        file->write(&master_header, sizeof(master_header), 0);
        // страница под схемы
        master_header.schemas_first_page = reinterpret_cast<db_ptr_t *>(sizeof(file_header));
        master_header.schemas_last_page = master_header.schemas_first_page;
        alloc_page(make_schemas_page_header());


        master_header.nodes_first_page = reinterpret_cast<db_ptr_t *>(sizeof(file_header) + sizeof(page_header) +
                                                                      DEFAULT_PAGE_SIZE);
        master_header.nodes_last_page = master_header.nodes_first_page;
        alloc_page(make_nodes_page_header());

        master_header.strings_first_page =
                reinterpret_cast<db_ptr_t *>(sizeof(file_header) + sizeof(page_header) + DEFAULT_PAGE_SIZE +
                                             sizeof(page_header) + DEFAULT_PAGE_SIZE);
        master_header.strings_last_page = master_header.strings_first_page;
        alloc_page(make_strings_page_header());

        file->write(&master_header, sizeof(master_header), 0);
    }

    void alloc_page(page_header header, db_size_t size = -1) {
        if (size == -1) {
            size = DEFAULT_PAGE_SIZE;
        }
        size = std::max(size, DEFAULT_PAGE_SIZE);
        file->write(&header, sizeof(page_header), master_header.file_end);

        void *zeros = calloc(size, 1);
        file->write(zeros, size);
        free(zeros);

        master_header.file_end += sizeof(page_header) + size;
    }


    template<class header_page_t>
    void insert_into_page(void *data, int size, header_page_t page_header, db_ptr_t page_ptr) {
        if (page_header.ind_last_elem + size > page_ptr + sizeof(header_page_t) + page_header.size) {

            alloc_page(page_header, size);
        }
    }

//    void add_schema() {
//
//    }

//    page_header load_page_header(db_ptr_t *ptr) {
//    }

    struct __attribute__((packed)) string_header_chunk {
        bool is_chunk;
        db_size_t size;
        db_ptr_t *nxt_chunk;
    };

    struct string_chunk {
        bool is_chunk;
        db_size_t size;
        db_ptr_t *nxt_chunk;
        char *content;
    };

    db_ptr_t *save_string(const std::string &s) {
        auto result = master_header.strings_last_page;
        auto kek = page_header();
        page_header *slp = &kek;
        if (is_in_file()) {
            this->file->read(slp, sizeof(page_header), master_header.strings_last_page);
        } else {
            slp = reinterpret_cast<page_header *>(master_header.strings_last_page);
        }
        string_header_chunk to_save;
        if (slp->get_free_space() >= sizeof(string_header_chunk) + s.size()) {
            to_save.is_chunk = false;
            to_save.size = s.size();
            to_save.nxt_chunk = nullptr;
            this->file->write(&to_save, sizeof(string_chunk),
                              master_header.strings_last_page + sizeof(page_header) + slp->ind_last_elem);
            this->file->write((void *) s.c_str(), to_save.size);
            master_header.strings_last_page += sizeof(page_header) + slp->ind_last_elem + to_save.size;
        } else {
            // XXX
            debug("oops");
        }

        if (is_in_file()) {
            this->file->write(slp, sizeof(page_header), master_header.strings_last_page);
        }
        return result;
    }

    struct scheme_key_value {
        db_ptr_t *key;
        prime_types value_type;
    };

    struct __attribute__((packed)) scheme_header_chunk {
        bool is_chunk;
        db_ptr_t *nxt_chunk;
        db_ptr_t name;
        db_size_t size;
    };

    struct scheme_header {
        bool is_chunk;
        db_ptr_t *nxt_chunk;
        db_ptr_t *name;
        db_size_t size;
        scheme_key_value *fields;
    };

    result create_schema(const create_schema_query &args) {
        scheme_header sh;
        // сохранить имя
        sh.name = save_string(args.name);
        // сохранить названия полей
        sh.size = args.fields.size();
        for (int i = 0; auto [k, v]: args.fields) {
            sh.fields[i] = {save_string(k), v};
        }
        save_schema(sh);
    }

    db_ptr_t *save_schema(scheme_header sh) {
        auto result = master_header.schemas_last_page;
        auto kek = page_header();
        page_header *slp = &kek;
        if (is_in_file()) {
            this->file->read(slp, sizeof(page_header), master_header.schemas_last_page);
        } else {
            slp = reinterpret_cast<page_header *>(master_header.schemas_last_page);
        }
        scheme_header_chunk to_save;
        if (slp->get_free_space() >= sizeof(scheme_header_chunk) + sh.size * sizeof(scheme_key_value)) {
            to_save.is_chunk = false;
            to_save.size = sh.size;
            to_save.nxt_chunk = nullptr;
            this->file->write(&to_save, sizeof(string_chunk),
                              master_header.schemas_last_page + sizeof(page_header) + slp->ind_last_elem);
            this->file->write(sh.fields, to_save.size * sizeof(scheme_key_value));
            master_header.schemas_last_page += sizeof(page_header) + slp->ind_last_elem + to_save.size;
        } else {
            // XXX
            debug("oops");
        }

        if (is_in_file()) {
            this->file->write(slp, sizeof(page_header), master_header.schemas_last_page);
        }
        return result;
    }

    std::vector<schema> get_schemas() {
        std::vector<schema> result;
        auto kek = page_header();
        page_header *slp = &kek;
        if (is_in_file()) {
            this->file->read(slp, sizeof(page_header), master_header.schemas_first_page);
        } else {
            slp = reinterpret_cast<page_header *>(master_header.schemas_first_page);
        }

    }


public:
    explicit database(const std::string &file_path, bool overwrite = false) {
        debug("Открытие базы");
#ifdef DEBUG
        file = new file_in_memory_interface(file_path, overwrite);
#else
        file = new T_file_interface(file_path, overwrite);
#endif
        if (is_in_file()) {
            info("База находится на диске");
        } else {
            info("База находится в ОЗУ");
        }
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
