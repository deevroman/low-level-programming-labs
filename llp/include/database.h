#ifndef LLP_DATABASE_H
#define LLP_DATABASE_H

#include <typeinfo>
#include <type_traits>
#include <string>
#include <vector>
#include "logger.h"
#include "file_interface.h"
#include "file_header.h"
#include "query.h"
#include "page.h"

class database {
public:
    file_interface *file;
    file_header master_header;

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

#pragma pack(push, 1)
    struct string_header_chunk {
        bool is_chunk;
        db_size_t size;
        db_ptr_t *nxt_chunk;
    } PACKED;
#pragma pack(pop)

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
        this->file->read(slp, sizeof(page_header), master_header.strings_last_page);
        string_header_chunk to_save;
        if (slp->get_free_space() >= sizeof(string_header_chunk) + s.size() + 1) {
            to_save.is_chunk = false;
            to_save.size = s.size();
            to_save.nxt_chunk = nullptr;
            this->file->write(&to_save, sizeof(string_chunk),
                              master_header.strings_last_page + sizeof(page_header) + slp->ind_last_elem);
            this->file->write((void *) s.c_str(), to_save.size + 1);
            slp->ind_last_elem += sizeof(string_chunk) + to_save.size + 1;
        } else {
            // XXX
            debug("oops");
        }
        this->file->write(slp, sizeof(page_header), master_header.strings_last_page);
        file->write(&master_header, sizeof(master_header), 0);
        return result;
    }

    struct scheme_key_value {
        db_ptr_t *key;
        prime_types value_type;
    };

#pragma pack(push, 1)
    struct scheme_header_chunk {
        bool is_chunk;
        db_ptr_t *nxt_chunk;
        db_ptr_t name;
        db_size_t size;
    } PACKED;
#pragma pack(pop)


    class scheme_header {
    public:
        bool is_chunk;
        db_ptr_t *nxt_chunk;
        db_ptr_t *name;
        db_size_t size;
        scheme_key_value *fields;

        virtual ~scheme_header() {
            delete[] fields;
        }
    };

    result create_schema(const create_schema_query &args) {
        scheme_header sh{};
        // сохранить имя
        debug(args.name);
        sh.name = save_string(args.name);
        // сохранить названия полей
        sh.size = args.fields.size();
        sh.fields = new scheme_key_value[sh.size];
        for (int i = 0; auto [k, v]: args.fields) {
            sh.fields[i] = {save_string(k), v};
        }
        save_schema(sh);

        return result(true, "");
    }

    db_ptr_t *save_schema(const scheme_header &sh) {
        auto result = master_header.schemas_last_page;
        auto kek = page_header();
        page_header *slp = &kek;
        this->file->read(slp, sizeof(page_header), master_header.schemas_last_page);
        scheme_header_chunk to_save{};
        if (slp->get_free_space() >= sizeof(scheme_header_chunk) + sh.size * sizeof(scheme_key_value)) {
            to_save.is_chunk = false;
            to_save.size = sh.size;
            to_save.nxt_chunk = nullptr;
            this->file->write(&to_save, sizeof(scheme_header_chunk),
                              master_header.schemas_last_page + sizeof(page_header) + slp->ind_last_elem);
            this->file->write(sh.fields, to_save.size * sizeof(scheme_key_value));
            slp->ind_last_elem += sizeof(string_chunk) + to_save.size;
        } else {
            // XXX
            debug("oops");
        }

        this->file->write(slp, sizeof(page_header), master_header.schemas_last_page);
        file->write(&master_header, sizeof(master_header), 0);
        return result;
    }

    result get_schemas() const {
        std::vector<schema> schemas;
        auto kek = page_header();
        page_header *slp = &kek;
        db_ptr_t *current_page = master_header.schemas_first_page;
        this->file->read(slp, sizeof(page_header), current_page);
        db_size_t current_schema_offset = 0; 
        while (current_schema_offset < slp->size) {
            auto lol = scheme_header_chunk();
            scheme_header_chunk *shc = &lol;
            this->file->read(shc, sizeof(scheme_header_chunk), current_page+current_schema_offset);
//            todo
        }
        
        return result(true, "", result_payload(
                schemas
        ));
    }


public:
    explicit database(const std::string &file_path, bool overwrite = false) {
        debug("Открытие базы");
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
                info("Запрос на создание новой схемы:");
                return create_schema(std::get<create_schema_query>(q.payload));
            case SHOW_SCHEMAS:
                return get_schemas();
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
        return result(false, "bred situation");
    }
};


#endif //LLP_DATABASE_H
