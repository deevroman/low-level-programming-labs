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

  template <uint64_t PageMarker>
  DbPtr WriteStruct(void *target_struct, DbSize size, FileChunkedList<PageMarker> &elements_list);

  [[nodiscard]] Byte *ReadStruct(DbPtr target_struct) const;

  void RewriteStruct(void *target_struct, DbSize size, DbPtr start) const;

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

  [[nodiscard]] Result InsertElement(insert_query args) const;

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

    void Next();

    Element Read();

    void Remove();
  };
};

template <uint64_t PageMarker, class Element>
void Database::ListIterator<PageMarker, Element>::Remove() {
  debug("Удаляю схему номер", cur_index_);
  std::unique_ptr<Byte[]> buffer(this->db_->ReadStruct(current_ptr_));
  auto unpacked_schema = SchemaHeader(buffer.get());
  debug_assert(unpacked_schema.Validate(*this->db_->file_));
  

  debug("Удаляю ссылки на строки в схеме");
  db_->RemoveString(unpacked_schema.name_);
  for (int i = 0; i < unpacked_schema.size_; i++) {
    db_->RemoveString(unpacked_schema.fields_[i].key);
  }

  if (prev_ptr_ != 0) {
    // прочесть предыдущую схему и изменить ссылку
    std::unique_ptr<Byte[]> buffer_2(this->db_->ReadStruct(prev_ptr_));
    auto unpacked_schema_2 = SchemaHeader(buffer_2.get());
    debug_assert(unpacked_schema_2.Validate(*this->db_->file_));
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
    debug_assert(unpacked_schema.Validate(*this->db_->file_));
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
    file_->Read(&master_header_, sizeof(file_header), 0);
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
  file_->Write(&header, sizeof(page_header), master_header_.file_end);

  auto chunk_cnt = size / sizeof(PageChunk);
  std::unique_ptr<PageChunk[]> buffer(new PageChunk[chunk_cnt]{});
  PageChunk cur_chunk;
  cur_chunk.nxt_chunk = master_header_.file_end + sizeof(page_header);
  for (DbSize i = 0; i < chunk_cnt; i++) {
    cur_chunk.nxt_chunk += sizeof(PageChunk);
#ifdef DEBUG
    file_->calls_.insert({master_header_.file_end + sizeof(page_header) + sizeof(PageChunk) * i, sizeof(PageChunk)});
    file_->calls_ptrs_.insert(master_header_.file_end + sizeof(page_header) + sizeof(PageChunk) * i);
#endif
    buffer.get()[i] = cur_chunk;
  }
  buffer.get()[chunk_cnt - 1].nxt_chunk = elements_list.first_free_element;
  file_->Write(buffer.get(), size);

  auto re = master_header_.file_end;
  elements_list.first_free_element = master_header_.file_end + sizeof(page_header);
  master_header_.file_end += sizeof(page_header) + size;
  UpdateMasterHeader();
  return re;
}

template <uint64_t PageMarker>
DbPtr Database::WriteStruct(void *target_struct, DbSize size, FileChunkedList<PageMarker> &elements_list) {
  debug("Записываю в файл структуру", size, elements_list.first_element, elements_list.first_free_element);
  assert(elements_list.first_free_element);
  assert(size != 0);
  DbPtr result_position = elements_list.first_free_element;

  DbPtr next_free = elements_list.first_free_element;
  Byte *byted_target_struct = static_cast<Byte *>(target_struct);
  PageChunk cur_chunk, next_chunk;
  int i = 0;
  for (; i + kChunkDataSize < size; i += kChunkDataSize) {
    this->file_->Read(&next_chunk, sizeof(PageChunk), next_free);
    if (next_chunk.nxt_chunk == 0) {
      AllocPage(elements_list);
      next_chunk.nxt_chunk = elements_list.first_free_element;
    }
    cur_chunk.nxt_chunk = next_chunk.nxt_chunk;
    std::copy(byted_target_struct + i, byted_target_struct + i + kChunkDataSize, cur_chunk.data);
    debug("Занимаю чанк с", next_free, next_chunk.nxt_chunk);
    this->file_->Write(&cur_chunk, sizeof(PageChunk), next_free);
    next_free = next_chunk.nxt_chunk;
  }
  // последний чанк должен указывать на голову списка
  this->file_->Read(&next_chunk, sizeof(PageChunk), next_free);
  cur_chunk.nxt_chunk = 0;
  std::fill(cur_chunk.data, cur_chunk.data + sizeof(cur_chunk.data), 0);
  std::copy(byted_target_struct + i, byted_target_struct + std::min(i + kChunkDataSize, size), cur_chunk.data);
  debug("Занимаю последний чанк структуры", next_free, next_chunk.nxt_chunk);
  this->file_->Write(&cur_chunk, sizeof(PageChunk), next_free);

  elements_list.first_element = result_position;
  elements_list.first_free_element = next_chunk.nxt_chunk;
  debug(elements_list.first_element, elements_list.first_free_element);
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
    this->file_->Read(&cur_chunk, sizeof(PageChunk), target_struct);
    next_chunk = cur_chunk.nxt_chunk;
    std::fill(cur_chunk.data, cur_chunk.data + sizeof(cur_chunk.data), 0);
    this->file_->Write(&cur_chunk, sizeof(PageChunk), target_struct);
    if (next_chunk == 0) {
      break;
    }
    target_struct = next_chunk;
  }
  // последний чанк должен указывать на старый пустой чанк-кандидат
  cur_chunk.nxt_chunk = old_free_start;
  this->file_->Write(&cur_chunk, sizeof(PageChunk), target_struct);
  UpdateMasterHeader();
}

void Database::RewriteStruct(void *target_struct, DbSize size, DbPtr start) const {  // todo refactor copy paste
  assert(start);
  assert(size != 0);

  DbPtr next_free = start;
  Byte *byted_target_struct = static_cast<Byte *>(target_struct);
  PageChunk cur_chunk, next_chunk;
  int i = 0;
  for (; i + kChunkDataSize < size; i += kChunkDataSize) {
    this->file_->Read(&next_chunk, sizeof(PageChunk), next_free);
    cur_chunk.nxt_chunk = next_chunk.nxt_chunk;
    std::copy(byted_target_struct + i, byted_target_struct + i + kChunkDataSize, cur_chunk.data);
    this->file_->Write(&cur_chunk, sizeof(PageChunk), next_free);
    next_free = next_chunk.nxt_chunk;
    assert(next_free > 0);
  }
  // последний чанк должен указывать на голову списка
  this->file_->Read(&next_chunk, sizeof(PageChunk), next_free);
  cur_chunk.nxt_chunk = 0;
  std::fill(cur_chunk.data, cur_chunk.data + sizeof(cur_chunk.data), 0);
  std::copy(byted_target_struct + i, byted_target_struct + std::min(i + kChunkDataSize, size), cur_chunk.data);
  this->file_->Write(&cur_chunk, sizeof(PageChunk), next_free);
}

Byte *Database::ReadStruct(DbPtr target_struct) const {
  std::vector<PageChunk> buffer;
  while (target_struct != 0) {
    buffer.emplace_back();
    this->file_->Read(&(buffer.back()), sizeof(PageChunk), target_struct);
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

DbPtr Database::SaveString(const std::string &s) {
  debug("Сохраняю строку", s);
  assert(master_header_.strings.first_free_element != 0);
  return WriteStruct((void *)s.c_str(), s.size() + 1, master_header_.strings);
}

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
  return {false, "Not found"};
}

Result Database::InsertElement(insert_query args) const {
  Info("Запрос на вставку элемента");

  debug("Проверка наличия схемы");
  auto target_schema = GetSchemaByName(args.type);
  if (!target_schema.ok_) {
    return {false, "Schema for element not created"};
  }
  if (std::get<Schema>(target_schema.payload_).fields_.size() != args.fields.size()) {
    return {false, "Element not match with schema by fields count"};
  }
  for (auto [k, v] : std::get<Schema>(target_schema.payload_).fields_) {
    if (args.fields.find(k) == args.fields.end() || args.fields[k].index() != v) {
      return {false, "Element not match with schema"};
    }
  }

  //  SaveElement();

  return {true, "Done"};  // TODO id
}

Result Database::GetSchemas() const {
  Info("Запрос на получение всех схем");
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
  Info("Запрос на удаление схемы");
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
  Info("Запрос на создание новой схемы:");
  debug(args.name_);
  if (GetSchemaByName(args.name_).ok_) {
    debug("Схема не создана, потому что уже существует схема с тем же именем");
    return {false, "Duplicated schema name"};
  }
  SchemaHeader sh{SaveString(args.name_), static_cast<DbSize>(args.fields_.size()),
                  master_header_.schemas.first_element};
  for (int i = 0; auto [k, v] : args.fields_) {
    sh.fields_[i] = schema_key_value{.key = SaveString(k), .value_type = v};
    i++;
  }
  SaveSchema(sh);

  return {true, ""};
}

void Database::RemoveString(DbPtr ptr) {
  debug("Удаляю строку начиная с", ptr);
  CleanStruct<kStringsPageMarker>(ptr, master_header_.strings);
}

Result Database::GetSchemaByName(const std::string &name) const {
  debug("Поиск схемы с именем", name);
  ListIterator<kSchemasPageMarker, Schema> it{self_ref_, master_header_.schemas};
  for (int i = 0; i < master_header_.schemas.count; i++) {
    auto cur = it.Read();
    if (cur.name_ == name) {
      debug("Схема найдена");
      return {true, "", result_payload(cur)};
    }
    it.Next();
  }
  debug("Схема не найдена");
  return {false, "Not found"};
}

void Database::UpdateMasterHeader() { this->file_->Write(&master_header_, sizeof(master_header_), 0); }

#endif  // LLP_INCLUDE_DATABASE_H_
