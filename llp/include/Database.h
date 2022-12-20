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
 private:
  Database *self_ref_;

 public:
  FileInterface *file_;
  file_header master_header_;

  virtual ~Database() { delete file_; }

  bool Load() {
    try {
      file_->read(&master_header_, sizeof(file_header), 0);
    } catch (std::runtime_error &e) {
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

    master_header_.nodes_first_page = sizeof(file_header) + sizeof(page_header) + kDefaultPageSize;
    master_header_.nodes_last_page = master_header_.nodes_first_page;
    AllocPage(MakeNodesPageHeader());

    master_header_.strings_first_page =
        sizeof(file_header) + sizeof(page_header) + kDefaultPageSize + sizeof(page_header) + kDefaultPageSize;
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

  DbPtr SaveString(const std::string &s) {
    debug("Сохраняю строку", s);
    DbPtr result = -1;
    page_header slp = page_header();
    this->file_->read(&slp, sizeof(page_header), master_header_.strings_last_page);
    string_header_chunk to_save;
    if (slp.GetFreeSpace() < sizeof(string_header_chunk) + s.size() + 1) {
      // XXX
      auto last_string_page = master_header_.strings_last_page;
      master_header_.strings_last_page = AllocPage(MakeStringsPageHeader());
      slp.nxt_page = master_header_.strings_last_page;
      this->file_->write(&slp, sizeof(page_header), last_string_page);
      this->file_->read(&slp, sizeof(page_header), master_header_.strings_last_page);
    }
    if (slp.GetFreeSpace() >= sizeof(string_header_chunk) + s.size() + 1) {
      to_save.is_chunk = false;
      to_save.size = s.size();
      to_save.nxt_chunk = 0;
      this->file_->write(&to_save, sizeof(string_header_chunk),
                         master_header_.strings_last_page + sizeof(page_header) + slp.ind_last_elem);
      result = master_header_.strings_last_page + sizeof(page_header) + slp.ind_last_elem;
      this->file_->write((void *)s.c_str(), to_save.size + 1);
      slp.ind_last_elem += sizeof(string_header_chunk) + to_save.size + 1;
    } else {
      todo("oops");
    }
    this->file_->write(&slp, sizeof(page_header), master_header_.strings_last_page);
    file_->write(&master_header_, sizeof(master_header_), 0);
    assert(result != -1);
    debug("Строка сохранена");
    return result;
  }

  [[nodiscard]] std::string ReadString(const DbPtr ptr) const {
    assert(ptr > 0);
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

  void RemoveString(DbPtr ptr) const {
    debug("TODO");
    this->file_;
  }

  class SchemaHeader {
   public:
    DbPtr name_{};
    DbSize size_{};
    schema_key_value *fields_{};
    SchemaHeader(DbPtr name, DbSize size) : name_(name), size_(size) {
      debug("Создаю SchemaHeader", name, size);
      fields_ = reinterpret_cast<schema_key_value *>(new char[GetFlexibleElementSize()]{});
    }
    virtual ~SchemaHeader() { delete[] fields_; }
    [[nodiscard]] DbSize GetOnFileSize() const { return sizeof(raw_schema_header) + GetFlexibleElementSize(); }
    [[nodiscard]] DbSize GetFlexibleElementSize() const {
      // округялем вверх до размера заголовка, чтобы при наличии пустоты на странице в
      // неё всегда мог поместиться заголовок таким образом все схемы будут
      // лежать в памяти плотно
      return (size_ * sizeof(schema_key_value) + sizeof(raw_schema_header) - 1) / sizeof(raw_schema_header) *
             sizeof(raw_schema_header);
    }
    [[nodiscard]] DbSize GetFlexiblePadding() const {
      return size_ * sizeof(schema_key_value) % sizeof(raw_schema_header);
    }
  };

  Result CreateSchema(const create_schema_query &args) {
    debug(args.name_);
    if (GetSchemaByName(args.name_).ok_) {
      debug("Схема не создана, потому что уже существует схема с тем же именем");
      return Result(false, "Duplicated schema name");
    }
    SchemaHeader sh{SaveString(args.name_), static_cast<DbSize>(args.fields_.size())};
    for (int i = 0; auto [k, v] : args.fields_) {
      sh.fields_[i] = schema_key_value{.key = SaveString(k), .value_type = v};
      i++;
    }
    SaveSchema(sh);

    return Result(true, "");
  }

  Result DeleteSchema(const delete_schema_query &name) {
    debug(name);
    auto it = SchemasPageIterator(this);
    for (int i = 0; i < master_header_.schemas_count; i++) {
      auto cur = it.ReadSchema();
      if (cur.name_ == name) {
        debug("Схема для удаления найдена");
        SchemasPageIterator(this, cur).RemoveSchemaAndShift();
        return {true, ""};
      }
    }
    return {false, "Schema for delete not found"};
  }

  void AllocPagesForSize(page_header &start_page, DbSize requested_size) {
    debug("Создаю страницы под", requested_size);

    page_header current_page = start_page;
    requested_size -= start_page.GetFreeSpace();

    // для переиспользования освобождённых
    while (current_page.nxt_page != 0) {
      requested_size -= start_page.size;
      file_->read(&current_page, sizeof(page_header), current_page.nxt_page);
    }

    while (requested_size > 0) {
      auto last_schemas_page = master_header_.schemas_last_page;

      auto new_page = MakeSchemasPageHeader();
      master_header_.schemas_last_page = AllocPage(new_page);
      requested_size -= kDefaultPageSize;
      current_page.nxt_page = master_header_.schemas_last_page;
      this->file_->write(&current_page, sizeof(page_header), last_schemas_page);

      current_page = new_page;
    }

    debug("Страницы созданы");
  }

  DbPtr SaveSchema(const SchemaHeader &sh) {
    debug("Записываю схему");
    DbPtr result = 0;

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
      this->file_->read(&current_page, sizeof(page_header), current_page_address);
    }

    debug("Сохраняем заголовок");
    raw_schema_header to_save{.name = sh.name_, .size = sh.size_};

    result = current_page_address + sizeof(page_header) + current_page.ind_last_elem;
    this->file_->write(&to_save, sizeof(raw_schema_header), result);
    current_page.ind_last_elem += sizeof(raw_schema_header);
    this->file_->write(&current_page, sizeof(page_header), current_page_address);

    debug("Сохраняем элементы");
    auto saving_size = sh.GetFlexibleElementSize();
    while (saving_size > 0) {
      if (current_page.GetFreeSpace() == 0) {
        debug("На странице закончилось место. Переход на следующую");
        current_page_address = current_page.nxt_page;
        this->file_->read(&current_page, sizeof(page_header), current_page_address);
      }
      auto writable_size = std::min(current_page.GetFreeSpace(), saving_size);
      this->file_->write(sh.fields_, writable_size,
                         current_page_address + sizeof(page_header) + current_page.ind_last_elem);
      current_page.ind_last_elem += writable_size;
      saving_size -= writable_size;
      this->file_->write(&current_page, sizeof(page_header), current_page_address);
    }

    master_header_.schemas_count++;
    file_->write(&master_header_, sizeof(master_header_), 0);
    debug("Схема записана");
    return result;
  }

  [[nodiscard]] Result GetSchemas() const {
    std::vector<Schema> schemas;
    auto it = SchemasPageIterator(self_ref_);
    for (int i = 0; i < master_header_.schemas_count; i++) {
      schemas.push_back(it.ReadSchema());
    }
    return {true, "", result_payload(schemas)};
  }

  [[nodiscard]] Result GetSchemaByName(const std::string &name) const {
    debug("Поиск схемы с именем", name);
    auto it = SchemasPageIterator(self_ref_);
    for (int i = 0; i < master_header_.schemas_count; i++) {
      auto cur = it.ReadSchema();
      if (cur.name_ == name) {
        debug("Схема найдена");
        return Result(true, "", result_payload(cur));
      }
    }
    debug("Схема не найдена");
    return Result(false, "Not found");
  }

  Result InsertElement(insert_query args) const {
    debug("TODO");
    return Result(false, "Not found");
  }

  [[nodiscard]] Result GetElements() const {
    //    std::vector<Element> schemas;
    //    auto it = SchemasPageIterator(self_ref_);
    //    for (int i = 0; i < master_header_.schemas_count; i++) {
    //      schemas.push_back(it.ReadSchema());
    //    }
    //    return {false, "", result_payload(elements)};
    debug("TODO");
    return Result(false, "Not found");
  }

 public:
  explicit Database(const std::string &file_path, bool overwrite = false) {
    debug("Открытие базы");
    self_ref_ = this;
    file_ = new FileInterface(file_path, overwrite);
    if (overwrite) {
      debug("База создаётся с нуля");
      Init();
      debug("База создана");
    } else {
      debug("Загружаю существующую базу в файле");
      if (!Load()) {
        error("База повреждена");
      }
      debug("База из файла загружена");
    }
    debug("База открыта");
  }

  Result Query(const Query &q) {
    switch (q.type) {
      case CREATE_SCHEMA:
        info("Запрос на создание новой схемы:");
        return CreateSchema(std::get<create_schema_query>(q.payload));
      case SHOW_SCHEMAS:
        info("Запрос на получение всех схем");
        return GetSchemas();
      case DELETE_SCHEMA:
        info("Запрос на удаление схемы");
        return DeleteSchema(std::get<delete_schema_query>(q.payload));
      case INSERT:
        info("Запрос на вставку элемента");
        return InsertElement(std::get<insert_query>(q.payload));
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
    mutable Database *db_;
    DbPtr current_page_addres_;
    DbPtr current_page_offset_{};
    int cur_ = 0;
    page_header current_page_;

   private:
    bool IsLastPage(const DbPtr ptr) { return db_->master_header_.schemas_last_page == ptr; }

    SchemaHeader ReadSchemaHeader() {
      debug("Чтение хедера схемы");
      raw_schema_header raw_header;
      db_->file_->read(&raw_header, sizeof(raw_schema_header),
                       current_page_addres_ + sizeof(page_header) + current_page_offset_);
      current_page_offset_ += sizeof(raw_schema_header);

      debug("Чтение кусков полей схемы");
      SchemaHeader header(raw_header.name, raw_header.size);
      auto *schema_key_value_buffer = header.fields_;
      auto need_read_bytes = header.size_ * (DbSize)sizeof(schema_key_value);
      while (need_read_bytes > 0) {
        auto now_can_read_bytes = std::min(need_read_bytes, kDefaultPageSize - current_page_offset_);
        db_->file_->read(schema_key_value_buffer, now_can_read_bytes,
                         current_page_addres_ + sizeof(page_header) + current_page_offset_);
        schema_key_value_buffer += now_can_read_bytes / sizeof(schema_key_value);
        current_page_offset_ += now_can_read_bytes;
        need_read_bytes -= now_can_read_bytes;

        if (need_read_bytes) {
          current_page_addres_ = current_page_.nxt_page;
          db_->file_->read(&current_page_, sizeof(page_header), current_page_addres_);

          current_page_offset_ = 0;
        }
      }
      current_page_offset_ += header.GetFlexiblePadding();
      return header;
    }

   public:
    explicit SchemasPageIterator(Database *db) {
      db_ = db;
      current_page_addres_ = db->master_header_.schemas_first_page;
      current_page_offset_ = 0;
    }
    SchemasPageIterator(Database *db, const SchemaWithPosition &sh)
        : db_(db),
          current_page_addres_(sh.page_start_),
          current_page_offset_(sh.position_ - (sh.page_start_ + sizeof(page_header))),
          cur_(sh.index_) {}

    SchemaWithPosition ReadSchema() {
      debug("Читаю схему номер", cur_, current_page_addres_, current_page_offset_);

      if (cur_ >= db_->master_header_.schemas_count) {
        debug("Итератор вышел за границу");
        return {};
      }
      db_->file_->read(&current_page_, sizeof(page_header), current_page_addres_);

      SchemaWithPosition result;
      result.page_start_ = current_page_addres_;
      result.position_ = current_page_addres_ + sizeof(page_header) + current_page_offset_;

      auto header = ReadSchemaHeader();

      debug("Чтение названия и названий ключей схемы");
      result.name_ = db_->ReadString(header.name_);
      for (int i = 0; i < header.size_; i++) {
        result.fields_[db_->ReadString(header.fields_[i].key)] = header.fields_[i].value_type;
      }
      cur_++;
      debug("Схема прочитана", result.name_);
      return result;
    }

    void RemoveSchemaAndShift() {
      debug("Удаляю схему номер", cur_, current_page_addres_, current_page_offset_);

      if (cur_ >= db_->master_header_.schemas_count) {
        debug("Итератор вышел за границу");
        return;
      }
      db_->file_->read(&current_page_, sizeof(page_header), current_page_addres_);

      auto current_page_addres_1 = current_page_addres_;
      auto current_page_offset_1 = current_page_offset_;
      auto current_page_1 = current_page_;

      auto header = ReadSchemaHeader();
      auto current_page_addres_2 = current_page_addres_;
      auto current_page_offset_2 = current_page_offset_;
      auto current_page_2 = current_page_;

      debug("Удаляю ссылки на строки в схеме");
      db_->RemoveString(header.name_);
      for (int i = 0; i < header.size_; i++) {
        db_->RemoveString(header.fields_[i].key);
      }

      (db_->master_header_.schemas_count)--;
      db_->file_->write(&(db_->master_header_), sizeof(master_header_), 0);

      debug("Сдвигаю элементы схемы");
      DbSize shift_step = sizeof(raw_schema_header);
      std::unique_ptr<char[]> buffer(new char[shift_step]{});
      while (current_page_2.nxt_page != 0 || current_page_offset_2 != current_page_2.ind_last_elem) {
        if (current_page_offset_1 == current_page_1.size) {
          current_page_addres_1 = current_page_1.nxt_page;
          db_->file_->read(&current_page_1, sizeof(page_header), current_page_addres_1);
          current_page_offset_1 = 0;
        }

        if (current_page_offset_2 == current_page_2.size) {
          current_page_addres_2 = current_page_2.nxt_page;
          db_->file_->read(&current_page_2, sizeof(page_header), current_page_addres_2);
          current_page_offset_2 = 0;
        }

        db_->file_->read(buffer.get(), shift_step, current_page_addres_2 + sizeof(page_header) + current_page_offset_2);
        db_->file_->write(buffer.get(), shift_step,
                          current_page_addres_1 + sizeof(page_header) + current_page_offset_1);

        current_page_offset_1 += shift_step;
        current_page_offset_2 += shift_step;
      }
      current_page_1.ind_last_elem = current_page_offset_1;
      db_->file_->write(&current_page_1, sizeof(page_header), current_page_addres_1);

      debug("Обновляю последнюю страницу");
      db_->master_header_.schemas_last_page = current_page_addres_1;
      db_->file_->write(&(db_->master_header_), sizeof(master_header_), 0);

      debug("Затираю освободившееся место");
      while (current_page_addres_1 != 0) {
        std::unique_ptr<char[]> zeros(new char[current_page_1.size - current_page_offset_1]{});
        db_->file_->write(zeros.get(), current_page_1.size - current_page_offset_1,
                          current_page_addres_1 + sizeof(page_header) + current_page_offset_1);
        if (current_page_addres_1 == current_page_addres_2) {
          break;
        }
        current_page_addres_1 = current_page_1.nxt_page;
        db_->file_->read(&current_page_1, sizeof(page_header), current_page_addres_1);
        current_page_offset_1 = 0;
      }
      current_page_1.ind_last_elem = current_page_offset_1;
      db_->file_->write(&current_page_1, sizeof(page_header), current_page_addres_1);

      debug("Схемы сдвинуты");
    }
  };
};

#endif  // LLP_INCLUDE_DATABASE_H_
