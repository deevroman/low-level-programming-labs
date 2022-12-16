#ifndef LLP_INCLUDE_DATABASE_H_
#define LLP_INCLUDE_DATABASE_H_

#include <cassert>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <vector>

#include "FileInterface.h"
#include "Query.h"
#include "file_header.h"
#include "logger.h"
#include "page.h"
#include "schema_header_chunk.h"
#include "string_header_chunk.h"

class Database {
 public:
  FileInterface *file_;
  file_header master_header_;

  virtual ~Database() { delete file_; }

  bool Load() {
    if (!file_->read(&master_header_, sizeof(file_header), 0)) {
      return false;
    }
    if (master_header_.signature != kDatabaseFileSignature) {
      return false;
    }
    debug(master_header_.signature);
    debug(master_header_.file_end);
    debug(master_header_.schemas_first_page);
    debug(master_header_.nodes_first_page);
    debug(master_header_.strings_first_page);
    debug(master_header_.schemas_last_page);
    debug(master_header_.nodes_last_page);
    debug(master_header_.strings_last_page);
    return true;
  }

  void Init() {
    master_header_.signature = kDatabaseFileSignature;
    master_header_.file_end = sizeof(file_header);
    master_header_.data_summary_size = 0;

    // заголовок
    file_->write(&master_header_, sizeof(master_header_), 0);
    // страница под схемы
    master_header_.schemas_first_page = sizeof(file_header);
    master_header_.schemas_last_page = master_header_.schemas_first_page;
    AllocPage(MakeSchemasPageHeader());

    master_header_.nodes_first_page =
        sizeof(file_header) + sizeof(page_header) + kDefaultPageSize;
    master_header_.nodes_last_page = master_header_.nodes_first_page;
    AllocPage(MakeNodesPageHeader());

    master_header_.strings_first_page = sizeof(file_header) +
                                        sizeof(page_header) + kDefaultPageSize +
                                        sizeof(page_header) + kDefaultPageSize;
    master_header_.strings_last_page = master_header_.strings_first_page;
    AllocPage(MakeStringsPageHeader());

    file_->write(&master_header_, sizeof(master_header_), 0);
  }

  DbPtr AllocPage(page_header header, DbSize size = -1) {
    if (size == -1) {
      size = kDefaultPageSize;
    }
    size = std::max(size, kDefaultPageSize);
    file_->write(&header, sizeof(page_header), master_header_.file_end);

    void *zeros = calloc(size, 1);
    file_->write(zeros, size);
    free(zeros);

    auto re = master_header_.file_end;
    master_header_.file_end += sizeof(page_header) + size;
    return re;
  }

  template <class header_page_t>
  void insert_into_page(void *data, int size, header_page_t page_header,
                        DbPtr page_ptr) {
    if (page_header.ind_last_elem + size >
        page_ptr + sizeof(header_page_t) + page_header.size_) {
      AllocPage(page_header, size);
    }
  }

  DbPtr SaveString(const std::string &s) {
    DbPtr result = -1;
    page_header slp = page_header();
    this->file_->read(&slp, sizeof(page_header),
                      master_header_.strings_last_page);
    string_header_chunk to_save;
    if (slp.GetFreeSpace() < sizeof(string_header_chunk) + s.size() + 1) {
      // XXX
      auto last_string_page = master_header_.strings_last_page;
      master_header_.strings_last_page = AllocPage(MakeStringsPageHeader());
      slp.nxt_page = master_header_.strings_last_page;
      this->file_->write(&slp, sizeof(page_header), last_string_page);
      this->file_->read(&slp, sizeof(page_header),
                        master_header_.strings_last_page);
    }
    if (slp.GetFreeSpace() >= sizeof(string_header_chunk) + s.size() + 1) {
      to_save.is_chunk = false;
      to_save.size = s.size();
      to_save.nxt_chunk = 0;
      this->file_->write(&to_save, sizeof(string_header_chunk),
                         master_header_.strings_last_page +
                             sizeof(page_header) + slp.ind_last_elem);
      result = master_header_.strings_last_page + sizeof(page_header) +
               slp.ind_last_elem;
      this->file_->write((void *)s.c_str(), to_save.size + 1);
      slp.ind_last_elem += sizeof(string_header_chunk) + to_save.size + 1;
    } else {
      todo("oops");
    }
    this->file_->write(&slp, sizeof(page_header),
                       master_header_.strings_last_page);
    file_->write(&master_header_, sizeof(master_header_), 0);
    assert(result != -1);
    return result;
  }

  [[nodiscard]] std::string ReadString(const DbPtr ptr) const {
    auto result = std::string();
    // Читаем чанки. Вынести в универсальное?
    auto first_chunk = string_header_chunk();
    file_->read(&first_chunk, sizeof(string_header_chunk), ptr);
    if (first_chunk.is_chunk) {
      todo("XXX");
    } else {
      assert(first_chunk.size > 0);
      result.resize(first_chunk.size);
      file_->read(&result[0], first_chunk.size);
    }
    debug(ptr, result);
    return result;
  }

  class SchemaHeader {
   public:
    DbPtr name_{};
    DbSize size_{};
    schema_key_value *fields_{};
    SchemaHeader(DbPtr name, DbSize size, schema_key_value *fields)
        : name_(name), size_(size), fields_(fields) {}
    virtual ~SchemaHeader() { delete[] fields_; }
    DbSize GetOnFileSize() const {
      return sizeof(raw_schema_header) + GetFlexbleElementSize();
    }
    DbSize GetFlexbleElementSize() const {
      // округялем до размера заголовка, чтобы при наличии пустоты на странице в
      // неё всегда мог поместиться заголовок таким образом все схемы будут
      // лежать в памяти плотно
      return size_ * sizeof(schema_key_value) / sizeof(raw_schema_header) *
             sizeof(raw_schema_header);
    }
  };

  Result CreateSchema(const create_schema_query &args) {
    debug(args.name_);
    SchemaHeader sh{SaveString(args.name_),
                    static_cast<DbSize>(args.fields_.size()),
                    new schema_key_value[args.fields_.size()]};
    for (int i = 0; auto [k, v] : args.fields_) {
      sh.fields_[i] = {SaveString(k), v};
      i++;
    }
    SaveSchema(sh);

    return Result(true, "");
  }

  void AllocPagesForSize(page_header &start_page, DbSize requested_size) {
    page_header current_page = start_page;
    requested_size -= start_page.GetFreeSpace();

    while (requested_size > 0) {
      auto last_schemas_page = master_header_.schemas_last_page;

      auto new_page = MakeSchemasPageHeader();
      master_header_.schemas_last_page = AllocPage(new_page);
      requested_size -= kDefaultPageSize;
      current_page.nxt_page = master_header_.schemas_last_page;
      this->file_->write(&current_page, sizeof(page_header), last_schemas_page);

      current_page = new_page;
    }
  }

  DbPtr SaveSchema(const SchemaHeader &sh) {
    auto result = 0;

    auto current_page_address = master_header_.schemas_last_page;
    page_header current_page = page_header();
    this->file_->read(&current_page, sizeof(page_header), current_page_address);

    // Если не хватает места -- выделяем новые страницы
    if (current_page.GetFreeSpace() < sh.GetOnFileSize()) {
      AllocPagesForSize(current_page, sh.GetOnFileSize());
    }

    // начало записи
    if (current_page.GetFreeSpace() < sizeof(raw_schema_header)) {
      assert(current_page.GetFreeSpace() == 0);
      current_page_address = current_page.nxt_page;
      this->file_->read(&current_page, sizeof(page_header),
                        current_page_address);
    }

    // сохраняем заголовок
    raw_schema_header to_save{.size = sh.size_, .name = sh.name_};

    result =
        current_page_address + sizeof(page_header) + current_page.ind_last_elem;
    this->file_->write(&to_save, sizeof(raw_schema_header), result);
    current_page.ind_last_elem += sizeof(raw_schema_header);
    this->file_->write(&current_page, sizeof(page_header),
                       current_page_address);

    // сохраняем элементы
    auto saving_size = sh.GetFlexbleElementSize();
    while (saving_size > 0) {
      if (current_page.GetFreeSpace() == 0) {
        current_page_address = current_page.nxt_page;
        this->file_->read(&current_page, sizeof(page_header),
                          current_page_address);
      }
      auto writable_size = std::min(current_page.GetFreeSpace(), saving_size);
      this->file_->write(sh.fields_, writable_size);
      current_page.ind_last_elem += writable_size;
      this->file_->write(&current_page, sizeof(page_header),
                         current_page_address);
    }

    master_header_.schemas_count++;
    file_->write(&master_header_, sizeof(master_header_), 0);

    return result;
  }

  [[nodiscard]] Result GetSchemas() const {
    std::vector<Schema> schemas;
    auto it = SchemasPageIterator(this);
    for (int i = 0; i < master_header_.schemas_count; i++) {
      schemas.push_back(it.ReadSchema());
    }
    return Result(true, "", result_payload(schemas));
  }

 public:
  explicit Database(const std::string &file_path, bool overwrite = false) {
    debug("Открытие базы");
    file_ = new FileInterface(file_path, overwrite);
    if (overwrite) {
      Init();
    } else {
      if (!Load()) {
        error("База повреждена");
      }
    }
  }

  Result Query(const Query &q) {
    switch (q.type) {
      case CREATE_SCHEMA:
        info("Запрос на создание новой схемы:");
        return CreateSchema(std::get<create_schema_query>(q.payload));
      case SHOW_SCHEMAS:
        return GetSchemas();
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
        debug("bred");  // XXX
    }
    return Result(false, "bred situation");
  }

  class SchemasPageIterator {
    const Database *db_;
    DbPtr current_page_addres_;
    DbPtr current_page_offset_{};
    int cur_ = 0;

   private:
    bool IsLastPage(const DbPtr ptr) {
      return db_->master_header_.schemas_last_page == ptr;
    }

   public:
    explicit SchemasPageIterator(const Database *db) {
      db_ = db;
      current_page_addres_ = db->master_header_.schemas_first_page;
      current_page_offset_ = 0;
    }

    Schema ReadSchema() {
      debug(cur_, current_page_addres_, current_page_offset_);

      if (cur_ >= db_->master_header_.schemas_count) {
        return {};
      }
      auto result = Schema();
      auto current_page = page_header();
      db_->file_->read(&current_page, sizeof(page_header),
                       current_page_addres_);

      // чтение хедера
      auto raw_header = raw_schema_header();
      db_->file_->read(
          &raw_header, sizeof(raw_schema_header),
          current_page_addres_ + sizeof(page_header) + current_page_offset_);
      result.name_ = db_->ReadString(raw_header.name);
      current_page_offset_ += sizeof(raw_schema_header);

      SchemaHeader header(raw_header.name, raw_header.size,
                          new schema_key_value[raw_header.size]);
      // чтение кусков
      auto *schema_key_value_buffer = header.fields_;
      auto need_read_elems = header.size_;
      while (need_read_elems > 0) {
        auto now_can_read = std::min(need_read_elems,
                                     (kDefaultPageSize - current_page_offset_) /
                                         (DbSize)sizeof(schema_key_value));
        schema_key_value_buffer += now_can_read;
        db_->file_->read(schema_key_value_buffer, now_can_read,
                         current_page_addres_ + sizeof(page_header) +
                             current_page_offset_ + sizeof(raw_schema_header));
        need_read_elems -= now_can_read;

        if (need_read_elems) {
          current_page_addres_ = current_page.nxt_page;
          db_->file_->read(&current_page, sizeof(page_header),
                           current_page_addres_);

          current_page_offset_ = 0;
        }
      }

      for (int i = 0; i < raw_header.size; i++) {
        result.fields_[db_->ReadString(schema_key_value_buffer[i].key)] =
            schema_key_value_buffer[i].value_type;
      }
      cur_++;
      return result;
    }
  };
};

#endif  // LLP_INCLUDE_DATABASE_H_
