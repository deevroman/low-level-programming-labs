#ifndef LLP_INCLUDE_DATABASE_H_
#define LLP_INCLUDE_DATABASE_H_

#include <cassert>
#include <memory>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <vector>

#include "ElementHeader.h"
#include "FileInterface.h"
#include "PageChunk.h"
#include "Query.h"
#include "SchemaHeader.h"
#include "file_header.h"
#include "logger.h"
#include "page.h"
#include "raw_schema_header.h"
#include "string_header_chunk.h"

class Database {
 private:
  Database *self_ref_;

 public:
  FileInterface *file_;
  file_header master_header_;

 private:
  bool Load();

  void Init();

  template <uint64_t PageMarker>
  DbPtr AllocPage(FileChunkedList<PageMarker> &elements_list, DbSize size = -1);

  DbPtr SaveString(const std::string &s);

  [[nodiscard]] std::string ReadString(DbPtr ptr) const;

  void RemoveString(DbPtr ptr);

  DbPtr SaveSchema(const SchemaHeader &sh);

  //  DbPtr SaveElement(const ElementHeader &sh);
  template <uint64_t PageMarker>
  DbPtr WriteStruct(void *target_struct, DbSize size, FileChunkedList<PageMarker> &elements_list);

  Byte *ReadStruct(DbPtr target_struct) const;

  void RewriteStruct(void *target_struct, DbSize size, DbPtr start);

  template <uint64_t PageMarker>
  void CleanStruct(DbPtr target_struct, FileChunkedList<PageMarker> &elements_list);

 public:
  explicit Database(const std::string &file_path, bool overwrite = false);

  virtual ~Database() { delete file_; }
  
  void UpdateMasterHeader();
  
  Result CreateSchema(const create_schema_query &args);

  Result DeleteSchema(const delete_schema_query &name);

  [[nodiscard]] Result GetSchemas() const;

  [[nodiscard]] Result GetSchemaByName(const std::string &name) const;

  Result InsertElement(insert_query args) const;

  [[nodiscard]] Result GetElements() const;

  template <uint64_t PageMarker, class Element>
  class ListIterator {
    mutable Database *db_;
    FileChunkedList<PageMarker> head_;
    DbPtr prev_ptr_{};
    DbPtr current_ptr_{};
    int cur_index_ = 0;

   public:
    ListIterator(Database *db, FileChunkedList<PageMarker> head);
    //    SchemasPageIterator(Database *db, const SchemaWithPosition &sh)
    //        : db_(db),
    //          current_page_address_(sh.page_start_),
    //          current_page_offset_(sh.position_ - (sh.page_start_ + sizeof(page_header))),
    //          cur_(sh.index_) {}

    void Next();

    Element Read();

    void Remove();
  };
  SchemaHeader ReadSchema(const DbPtr ptr) const;
};

template <uint64_t PageMarker, class Element>
void Database::ListIterator<PageMarker, Element>::Remove() {
  debug("Удаляю схему номер", cur_index_);
  std::unique_ptr<Byte[]> buffer(this->db_->ReadStruct(current_ptr_));
  auto unpacked_schema = SchemaHeader(buffer.get());

  debug("Удаляю ссылки на строки в схеме");
  db_->RemoveString(unpacked_schema.name_);
  for (int i = 0; i < unpacked_schema.size_; i++) {
    db_->RemoveString(unpacked_schema.fields_[i].key);
  }

  if (prev_ptr_ != 0) {
    // прочесть предыдущую схему и изменить ссылку
    std::unique_ptr<Byte[]> buffer_2(this->db_->ReadStruct(prev_ptr_));
    auto unpacked_schema_2 = SchemaHeader(buffer_2.get());
    unpacked_schema_2.nxt_ = unpacked_schema.nxt_;
    // записать обратно предыдущую
    auto raw = std::unique_ptr<Byte[]>(unpacked_schema_2.MakePackedSchema());
    this->db_->RewriteStruct(raw.get(), unpacked_schema_2.GetOnFileSize(), prev_ptr_);
  } else {
    if (PageMarker == kSchemasPageMarker) {
      this->db_->master_header_.schemas.first_element = unpacked_schema.nxt_;
    } else {
    }
  }

  this->db_->CleanStruct(current_ptr_, this->db_->master_header_.schemas);
  this->db_->master_header_.schemas.count--;
  this->db_->UpdateMasterHeader();
}

template <uint64_t PageMarker, class Element>
Element Database::ListIterator<PageMarker, Element>::Read() {
  std::unique_ptr<Byte[]> buffer(this->db_->ReadStruct(current_ptr_));
  if (PageMarker == kSchemasPageMarker) {
    auto unpacked_schema = SchemaHeader(buffer.get());
    auto result = Element();
    result.name_ = db_->ReadString(unpacked_schema.name_);
    for (int i = 0; i < unpacked_schema.size_; i++) {
      result.fields_[db_->ReadString(unpacked_schema.fields_[i].key)] = unpacked_schema.fields_[i].value_type;
    }
    this->cur_index_++;
    return result;
  } else if (PageMarker == kNodesPageMarker) {
    // TODO
  }
}

template <uint64_t PageMarker, class Element>
void Database::ListIterator<PageMarker, Element>::Next() {
  std::unique_ptr<Byte[]> buffer(this->db_->ReadStruct(current_ptr_));
  if (PageMarker == kSchemasPageMarker) {
    prev_ptr_ = current_ptr_;
    current_ptr_ = SchemaHeader(buffer.get()).nxt_;
  } else if (PageMarker == kNodesPageMarker) {
    // TODO
  }
}

template <uint64_t PageMarker, class Element>
Database::ListIterator<PageMarker, Element>::ListIterator(Database *db, FileChunkedList<PageMarker> head) {
  db_ = db;
  head_ = head;
  current_ptr_ = head.first_element;
}

bool Database::Load() {
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
  debug(master_header_.schemas.first_element);
  debug(master_header_.schemas.first_free_element);
  debug(master_header_.schemas.count);
  debug(master_header_.nodes.first_element);
  debug(master_header_.nodes.first_free_element);
  debug(master_header_.nodes.count);
  debug(master_header_.strings.first_element);
  debug(master_header_.strings.first_free_element);
  debug(master_header_.strings.count);

  return true;
}

void Database::Init() {
  master_header_.signature = kDatabaseFileSignature;
  master_header_.file_end = sizeof(file_header);
  master_header_.data_summary_size = 0;

  // заголовок
  UpdateMasterHeader();
  // страница под схемы
  master_header_.schemas = {};
  AllocPage(master_header_.schemas);

  master_header_.nodes = {};
  AllocPage(master_header_.nodes);

  master_header_.strings = {};
  AllocPage(master_header_.strings);

  UpdateMasterHeader();
}

template <uint64_t PageMarker>
DbPtr Database::AllocPage(FileChunkedList<PageMarker> &elements_list, DbSize size) {
  debug("Выделяю новую страницу");
  if (size == -1) {
    size = kDefaultPageSize;
  }
  assert(size % sizeof(PageChunk) == 0);
  assert(size / sizeof(PageChunk) > 0);
  size = std::max(size, kDefaultPageSize);
  auto header = elements_list.MakePageHeader();  // нужны ли ссылки между страницами?
  file_->write(&header, sizeof(page_header), master_header_.file_end);

  auto chunk_cnt = size / sizeof(PageChunk);
  std::unique_ptr<PageChunk[]> buffer(new PageChunk[chunk_cnt]{});
  PageChunk cur_chunk;
  cur_chunk.nxt_chunk = master_header_.file_end + sizeof(page_header);
  for (DbSize i = 0; i < chunk_cnt; i++) {
    cur_chunk.nxt_chunk += sizeof(PageChunk);
    buffer.get()[i] = cur_chunk;
  }
  buffer.get()[chunk_cnt - 1].nxt_chunk = elements_list.first_free_element;
  file_->write(buffer.get(), size);

  auto re = master_header_.file_end;
  elements_list.first_free_element = master_header_.file_end + sizeof(page_header);
  master_header_.file_end += sizeof(page_header) + size;
  UpdateMasterHeader();
  return re;
}

template <uint64_t PageMarker>
DbPtr Database::WriteStruct(void *target_struct, DbSize size, FileChunkedList<PageMarker> &elements_list) {
  assert(elements_list.first_free_element);
  assert(size != 0);
  DbPtr result_position = elements_list.first_free_element;

  DbPtr next_free = elements_list.first_free_element;
  Byte *byted_target_struct = static_cast<Byte *>(target_struct);
  PageChunk cur_chunk, next_chunk;
  int i = 0;
  for (; i + kChunkDataSize < size; i += kChunkDataSize) {
    this->file_->read(&next_chunk, sizeof(PageChunk), next_free);
    if (next_chunk.nxt_chunk == 0) {
      AllocPage(elements_list);
      next_chunk.nxt_chunk = elements_list.first_free_element;
    }
    cur_chunk.nxt_chunk = next_chunk.nxt_chunk;
    std::copy(byted_target_struct + i, byted_target_struct + i + kChunkDataSize, cur_chunk.data);
    this->file_->write(&cur_chunk, sizeof(PageChunk), next_free);
    next_free = next_chunk.nxt_chunk;
  }
  // последний чанк должен указывать на голову списка
  this->file_->read(&next_chunk, sizeof(PageChunk), next_free);
  cur_chunk.nxt_chunk = 0;
  std::fill(cur_chunk.data, cur_chunk.data + sizeof(cur_chunk.data), 0);
  std::copy(byted_target_struct + i, byted_target_struct + std::min(i + kChunkDataSize, size), cur_chunk.data);
  this->file_->write(&cur_chunk, sizeof(PageChunk), next_free);

  elements_list.first_element = result_position;
  elements_list.first_free_element = next_chunk.nxt_chunk;
  if (elements_list.first_free_element == 0) {
    AllocPage(elements_list);
  }
  UpdateMasterHeader();
  return result_position;
}

template <uint64_t PageMarker>
void Database::CleanStruct(DbPtr target_struct, FileChunkedList<PageMarker> &elements_list) {
  assert(elements_list.first_free_element);
  DbPtr old_free_start = elements_list.first_free_element;
  elements_list.first_free_element = target_struct;

  PageChunk cur_chunk;
  DbPtr next_chunk;
  while (true) {
    this->file_->read(&cur_chunk, sizeof(PageChunk), target_struct);
    next_chunk = cur_chunk.nxt_chunk;
    std::fill(cur_chunk.data, cur_chunk.data + sizeof(cur_chunk.data), 0);
    this->file_->write(&cur_chunk, sizeof(PageChunk), target_struct);
    if (next_chunk == 0) {
      break;
    }
    target_struct = next_chunk;
  }
  // последний чанк должен указывать на старый пустой чанк-кандидат
  cur_chunk.nxt_chunk = old_free_start;
  this->file_->write(&cur_chunk, sizeof(PageChunk), target_struct);
  UpdateMasterHeader();
}

void Database::RewriteStruct(void *target_struct, DbSize size, DbPtr start) {  // todo refactor copy paste
  assert(start);
  assert(size != 0);

  DbPtr next_free = start;
  Byte *byted_target_struct = static_cast<Byte *>(target_struct);
  PageChunk cur_chunk, next_chunk;
  int i = 0;
  for (; i + kChunkDataSize < size; i += kChunkDataSize) {
    this->file_->read(&next_chunk, sizeof(PageChunk), next_free);
    cur_chunk.nxt_chunk = next_chunk.nxt_chunk;
    std::copy(byted_target_struct + i, byted_target_struct + i + kChunkDataSize, cur_chunk.data);
    this->file_->write(&cur_chunk, sizeof(PageChunk), next_free);
    next_free = next_chunk.nxt_chunk;
    assert(next_free > 0);
  }
  // последний чанк должен указывать на голову списка
  this->file_->read(&next_chunk, sizeof(PageChunk), next_free);
  cur_chunk.nxt_chunk = 0;
  std::fill(cur_chunk.data, cur_chunk.data + sizeof(cur_chunk.data), 0);
  std::copy(byted_target_struct + i, byted_target_struct + std::min(i + kChunkDataSize, size), cur_chunk.data);
  this->file_->write(&cur_chunk, sizeof(PageChunk), next_free);
}

Byte *Database::ReadStruct(DbPtr target_struct) const {
  std::vector<PageChunk> buffer;
  while (target_struct != 0) {
    buffer.push_back({});
    this->file_->read(&(buffer.back()), sizeof(PageChunk), target_struct);
    target_struct = buffer.back().nxt_chunk;
  }
  Byte *result = new Byte[buffer.size() * sizeof(PageChunk::data)];
  for (int res_ind = 0; auto now : buffer) {
    std::copy(now.data, now.data + sizeof(now.data), &(result[res_ind]));
    res_ind += +sizeof(now.data);
  }
  return result;
}

std::string Database::ReadString(const DbPtr ptr) const {
  assert(ptr > 0);
  std::unique_ptr<Byte[]> buffer(ReadStruct(ptr));
  auto result = std::string(reinterpret_cast<const char *>(buffer.get()));
  debug("Считана строка", result);
  return result;
}

SchemaHeader Database::ReadSchema(const DbPtr ptr) const {
  assert(ptr > 0);
  std::unique_ptr<Byte[]> buffer(ReadStruct(ptr));
  return {buffer.get()};
}

DbPtr Database::SaveString(const std::string &s) {
  debug("Сохраняю строку", s);
  assert(master_header_.strings.first_free_element != 0);
  return WriteStruct((void *)s.c_str(), s.size() + 1, master_header_.strings);
}
// DbPtr Database::SaveString(const std::string &s) {
//   debug("Сохраняю строку", s);
//   DbPtr result = -1;
//   assert(master_header_.strings_first_free_chunk != 0);
//
//   this->file_->read(&slp, sizeof(page_header), master_header_.strings_last_page);
//   string_header_chunk to_save;
//   if (slp.GetFreeSpace() < sizeof(string_header_chunk) + s.size() + 1) {
//     // XXX
//     auto last_string_page = master_header_.strings_last_page;
//     master_header_.strings_last_page = AllocPage(MakeStringsPageHeader(), master_header_.strings_first_free_chunk);
//     slp.nxt_page = master_header_.strings_last_page;
//     this->file_->write(&slp, sizeof(page_header), last_string_page);
//     this->file_->read(&slp, sizeof(page_header), master_header_.strings_last_page);
//   }
//   if (slp.GetFreeSpace() >= sizeof(string_header_chunk) + s.size() + 1) {
//     to_save.is_chunk = false;
//     to_save.size = s.size();
//     to_save.nxt_chunk = 0;
//     this->file_->write(&to_save, sizeof(string_header_chunk),
//                        master_header_.strings_last_page + sizeof(page_header) + slp.ind_last_elem);
//     result = master_header_.strings_last_page + sizeof(page_header) + slp.ind_last_elem;
//     this->file_->write((void *)s.c_str(), to_save.size + 1);
//     slp.ind_last_elem += sizeof(string_header_chunk) + to_save.size + 1;
//   } else {
//     todo("oops");
//   }
//   this->file_->write(&slp, sizeof(page_header), master_header_.strings_last_page);
//   file_->write(&master_header_, sizeof(master_header_), 0);
//   assert(result != -1);
//   debug("Строка сохранена");
//   return result;
// }

Database::Database(const std::string &file_path, bool overwrite) {
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

Result Database::GetElements() const {
  //    std::vector<Element> schemas;
  //    auto it = SchemasPageIterator(self_ref_);
  //    for (int i = 0; i < master_header_.schemas_count; i++) {
  //      schemas.push_back(it.ReadSchema());
  //    }
  //    return {false, "", result_payload(elements)};
  debug("TODO");
  return Result(false, "Not found");
}

Result Database::InsertElement(insert_query args) const {
  info("Запрос на вставку элемента");

  debug("Проверка наличия схемы");
  auto target_schema = GetSchemaByName(args.type);
  if (!target_schema.ok_) {
    return Result(false, "Schema for element not created");
  }
  if (std::get<Schema>(target_schema.payload_).fields_.size() != args.fields.size()) {
    return Result(false, "Element not match with schema by fields count");
  }
  for (auto [k, v] : std::get<Schema>(target_schema.payload_).fields_) {
    if (args.fields.find(k) == args.fields.end() || args.fields[k].index() != v) {
      return Result(false, "Element not match with schema");
    }
  }

  //  SaveElement();

  return Result(true, "Done");  // TODO id
}

Result Database::GetSchemas() const {
  info("Запрос на получение всех схем");
  std::vector<Schema> schemas;
  ListIterator<kSchemasPageMarker, Schema> it{self_ref_, master_header_.schemas};
  for (int i = 0; i < master_header_.schemas.count; i++) {
    schemas.push_back(it.Read());
    it.Next();
  }
  return {true, "", result_payload(schemas)};
}

DbPtr Database::SaveSchema(const SchemaHeader &sh) {
  debug("Записываю схему");
  auto raw = std::unique_ptr<Byte[]>(sh.MakePackedSchema());
  DbPtr result = WriteStruct(raw.get(), sh.GetOnFileSize(), master_header_.schemas);

  master_header_.schemas.count++;
  UpdateMasterHeader();
  debug("Схема записана");
  return result;
}

Result Database::DeleteSchema(const delete_schema_query &name) {
  info("Запрос на удаление схемы");
  debug(name);
  auto it = ListIterator<kSchemasPageMarker, Schema>(this, master_header_.schemas);
  for (int i = 0; i < master_header_.schemas.count; i++) {
    auto cur = it.Read();
    if (cur.name_ == name) {
      debug("Схема для удаления найдена");
      it.Remove();
      return {true, ""};
    }
    it.Next();
  }
  return {false, "Schema for delete not found"};
}

Result Database::CreateSchema(const create_schema_query &args) {
  info("Запрос на создание новой схемы:");
  debug(args.name_);
  if (GetSchemaByName(args.name_).ok_) {
    debug("Схема не создана, потому что уже существует схема с тем же именем");
    return Result(false, "Duplicated schema name");
  }
  SchemaHeader sh{SaveString(args.name_), static_cast<DbSize>(args.fields_.size()),
                  master_header_.schemas.first_element};
  for (int i = 0; auto [k, v] : args.fields_) {
    sh.fields_[i] = schema_key_value{.key = SaveString(k), .value_type = v};
    i++;
  }
  SaveSchema(sh);

  return Result(true, "");
}

// DbPtr Database::SaveElement(const ElementHeader &sh) {
//   debug("Записываю схему");
//   DbPtr result = 0;
//
//   auto current_page_address = master_header_.schemas_last_page;
//   page_header current_page = page_header();
//   this->file_->read(&current_page, sizeof(page_header), current_page_address);
//
//   // Если не хватает места -- выделяем новые страницы
//   if (current_page.GetFreeSpace() < sh.GetOnFileSize()) {
//     AllocPagesForSize(current_page, sh.GetOnFileSize());
//   }
//
//   // начало записи
//   if (current_page.GetFreeSpace() < sizeof(raw_schema_header)) {
//     assert(current_page.GetFreeSpace() == 0);
//     current_page_address = current_page.nxt_page;
//     this->file_->read(&current_page, sizeof(page_header), current_page_address);
//   }
//
//   debug("Сохраняем заголовок");
//   raw_schema_header to_save{.name = sh.name_, .size = sh.size_};
//
//   result = current_page_address + sizeof(page_header) + current_page.ind_last_elem;
//   this->file_->write(&to_save, sizeof(raw_schema_header), result);
//   current_page.ind_last_elem += sizeof(raw_schema_header);
//   this->file_->write(&current_page, sizeof(page_header), current_page_address);
//
//   debug("Сохраняем элементы");
//   auto saving_size = sh.GetFlexibleElementSize();
//   while (saving_size > 0) {
//     if (current_page.GetFreeSpace() == 0) {
//       debug("На странице закончилось место. Переход на следующую");
//       current_page_address = current_page.nxt_page;
//       this->file_->read(&current_page, sizeof(page_header), current_page_address);
//     }
//     auto writable_size = std::min(current_page.GetFreeSpace(), saving_size);
//     this->file_->write(sh.fields_, writable_size,
//                        current_page_address + sizeof(page_header) + current_page.ind_last_elem);
//     current_page.ind_last_elem += writable_size;
//     saving_size -= writable_size;
//     this->file_->write(&current_page, sizeof(page_header), current_page_address);
//   }
//
//   master_header_.schemas_count++;
//   file_->write(&master_header_, sizeof(master_header_), 0);
//   debug("Схема записана");
//   return result;
// }

void Database::RemoveString(DbPtr ptr) {
  debug("Удаляю строку начиная с", ptr);
  CleanStruct<kStringsPageMarker>(ptr, master_header_.strings);
}

// void Database::AllocPagesForSize(page_header &start_page, DbSize requested_size) {
//   debug("Создаю страницы под", requested_size);
//
//   page_header current_page = start_page;
//   requested_size -= start_page.GetFreeSpace();
//
//   // для переиспользования освобождённых
//   while (current_page.nxt_page != 0) {
//     requested_size -= start_page.size;
//     file_->read(&current_page, sizeof(page_header), current_page.nxt_page);
//   }
//
//   while (requested_size > 0) {
//     auto last_schemas_page = master_header_.schemas_last_page;
//
//     auto new_page = MakeSchemasPageHeader();
//     master_header_.schemas_last_page = AllocPage(new_page, master_header_.schemas_first_free_chunk);
//     requested_size -= kDefaultPageSize;
//     current_page.nxt_page = master_header_.schemas_last_page;
//     this->file_->write(&current_page, sizeof(page_header), last_schemas_page);
//
//     current_page = new_page;
//   }
//
//   debug("Страницы созданы");
// }

Result Database::GetSchemaByName(const std::string &name) const {
  debug("Поиск схемы с именем", name);
  ListIterator<kSchemasPageMarker, Schema> it{self_ref_, master_header_.schemas};
  for (int i = 0; i < master_header_.schemas.count; i++) {
    auto cur = it.Read();
    if (cur.name_ == name) {
      debug("Схема найдена");
      return Result(true, "", result_payload(cur));
    }
    it.Next();
  }
  debug("Схема не найдена");
  return Result(false, "Not found");
}

void Database::UpdateMasterHeader() {
  this->file_->write(&master_header_, sizeof(master_header_), 0);
}

// SchemaHeader Database::SchemasPageIterator::ReadSchemaHeader() {
////  debug("Чтение хедера схемы");
////  raw_schema_header raw_header;
////  db_->file_->read(&raw_header, sizeof(raw_schema_header),
////                   current_page_address_ + sizeof(page_header) + current_page_offset_);
////  current_page_offset_ += sizeof(raw_schema_header);
////
////  debug("Чтение кусков полей схемы");
////  SchemaHeader header(raw_header.name, raw_header.size);
////  auto *schema_key_value_buffer = header.fields_;
////  auto need_read_bytes = header.size_ * (DbSize)sizeof(schema_key_value);
////  while (need_read_bytes > 0) {
////    auto now_can_read_bytes = std::min(need_read_bytes, kDefaultPageSize - current_page_offset_);
////    db_->file_->read(schema_key_value_buffer, now_can_read_bytes,
////                     current_page_address_ + sizeof(page_header) + current_page_offset_);
////    schema_key_value_buffer += now_can_read_bytes / sizeof(schema_key_value);
////    current_page_offset_ += now_can_read_bytes;
////    need_read_bytes -= now_can_read_bytes;
////
////    if (need_read_bytes) {
////      current_page_address_ = current_page_.nxt_page;
////      db_->file_->read(&current_page_, sizeof(page_header), current_page_address_);
////
////      current_page_offset_ = 0;
////    }
////  }
////  current_page_offset_ += header.GetFlexiblePadding();
////  return header;
//}

// SchemaWithPosition Database::SchemasPageIterator::ReadSchema() {
//   debug("Читаю схему номер", cur_, current_page_address_, current_page_offset_);
//
//   if (cur_ >= db_->master_header_.schemas.count) {
//     debug("Итератор вышел за границу");
//     return {};
//   }
//   db_->file_->read(&current_page_, sizeof(page_header), current_page_address_);
//
//   SchemaWithPosition result;
//   result.page_start_ = current_page_address_;
//   result.position_ = current_page_address_ + sizeof(page_header) + current_page_offset_;
//
//   auto header = ReadSchemaHeader();
//
//   debug("Чтение названия и названий ключей схемы");
//   result.name_ = db_->ReadString(header.name_);
//   for (int i = 0; i < header.size_; i++) {
//     result.fields_[db_->ReadString(header.fields_[i].key)] = header.fields_[i].value_type;
//   }
//   cur_++;
//   debug("Схема прочитана", result.name_);
//   return result;
// }

// void Database::SchemasPageIterator::RemoveSchemaAndShift() {
//   //  debug("Удаляю схему номер", cur_, current_page_address_, current_page_offset_);
//   //
//   //  if (cur_ >= db_->master_header_.schemas_count) {
//   //    debug("Итератор вышел за границу");
//   //    return;
//   //  }
//   //  db_->file_->read(&current_page_, sizeof(page_header), current_page_address_);
//   //
//   //  auto current_page_address_1 = current_page_address_;
//   //  auto current_page_offset_1 = current_page_offset_;
//   //  auto current_page_1 = current_page_;
//   //
//   //  auto header = ReadSchemaHeader();
//   //  auto current_page_address_2 = current_page_address_;
//   //  auto current_page_offset_2 = current_page_offset_;
//   //  auto current_page_2 = current_page_;
//   //
//   //  debug("Удаляю ссылки на строки в схеме");
//   //  db_->RemoveString(header.name_);
//   //  for (int i = 0; i < header.size_; i++) {
//   //    db_->RemoveString(header.fields_[i].key);
//   //  }
//   //
//   //  (db_->master_header_.schemas_count)--;
//   //  db_->file_->write(&(db_->master_header_), sizeof(master_header_), 0);
//   //
//   //  debug("Сдвигаю элементы схемы");
//   //  DbSize shift_step = sizeof(raw_schema_header);
//   //  std::unique_ptr<char[]> buffer(new char[shift_step]{});
//   //  while (current_page_2.nxt_page != 0 || current_page_offset_2 != current_page_2.ind_last_elem) {
//   //    if (current_page_offset_1 == current_page_1.size) {
//   //      current_page_address_1 = current_page_1.nxt_page;
//   //      db_->file_->read(&current_page_1, sizeof(page_header), current_page_address_1);
//   //      current_page_offset_1 = 0;
//   //    }
//   //
//   //    if (current_page_offset_2 == current_page_2.size) {
//   //      current_page_address_2 = current_page_2.nxt_page;
//   //      db_->file_->read(&current_page_2, sizeof(page_header), current_page_address_2);
//   //      current_page_offset_2 = 0;
//   //    }
//   //
//   //    db_->file_->read(buffer.get(), shift_step, current_page_address_2 + sizeof(page_header) +
//   //    current_page_offset_2); db_->file_->write(buffer.get(), shift_step, current_page_address_1 +
//   sizeof(page_header)
//   //    + current_page_offset_1);
//   //
//   //    current_page_offset_1 += shift_step;
//   //    current_page_offset_2 += shift_step;
//   //  }
//   //  current_page_1.ind_last_elem = current_page_offset_1;
//   //  db_->file_->write(&current_page_1, sizeof(page_header), current_page_address_1);
//   //
//   //  debug("Обновляю последнюю страницу");
//   //  db_->master_header_.schemas_last_page = current_page_address_1;
//   //  db_->file_->write(&(db_->master_header_), sizeof(master_header_), 0);
//   //
//   //  debug("Затираю освободившееся место");
//   //  while (current_page_address_1 != 0) {
//   //    std::unique_ptr<char[]> zeros(new char[current_page_1.size - current_page_offset_1]{});
//   //    db_->file_->write(zeros.get(), current_page_1.size - current_page_offset_1,
//   //                      current_page_address_1 + sizeof(page_header) + current_page_offset_1);
//   //    if (current_page_address_1 == current_page_address_2) {
//   //      break;
//   //    }
//   //    current_page_address_1 = current_page_1.nxt_page;
//   //    db_->file_->read(&current_page_1, sizeof(page_header), current_page_address_1);
//   //    current_page_offset_1 = 0;
//   //  }
//   //  current_page_1.ind_last_elem = current_page_offset_1;
//   //  db_->file_->write(&current_page_1, sizeof(page_header), current_page_address_1);
//   //
//   //  debug("Схемы сдвинуты");
// }

#endif  // LLP_INCLUDE_DATABASE_H_
