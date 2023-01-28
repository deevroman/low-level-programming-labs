#include <query.grpc.pb.h>
#include <query.pb.h>

#include <grpc/grpc.h>
#include <grpcpp/server_builder.h>

#include "Database.h"
#include <iostream>
#include <vector>

std::map<std::string, DataItem>
proto_new_fields_to_db_new_fields(const google::protobuf::RepeatedPtrField<proto_query::field_key_value> &new_fields) {
  std::map<std::string, DataItem> fields;
  for (const auto &now : new_fields) {
    switch (now.value().value_type()) {
    case proto_query::DB_INT32:
      fields[now.key()] = {now.value().int_value()};
      break;
    case proto_query::DB_DOUBLE:
      fields[now.key()] = {now.value().double_value()};
      break;
    case proto_query::DB_STRING:
      fields[now.key()] = {now.value().str_value()};
      break;
    case proto_query::DB_BOOL:
      fields[now.key()] = {now.value().bool_value()};
      break;
    default:
      break;
    }
  }
  return fields;
}

std::vector<fields_conditional>
proto_new_fields_to_db_conds(const google::protobuf::RepeatedPtrField<proto_query::filter> &conds) {
  std::vector<fields_conditional> conditionals;
  for (const auto &now : conds) {
    conditionals.push_back({});
    conditionals.back().left_op = now.left_node();
    conditionals.back().right_op = now.right_node();
    conditionals.back().field_name = now.key_value().key();
    switch (now.op()) {
    case proto_query::OP_AND:
      conditionals.back().op = OP_AND;
      break;
    case proto_query::OP_OR:
      conditionals.back().op = OP_OR;
      break;
    default:
      break;
    }
    switch (now.key_value().value().value_type()) {
    case proto_query::DB_INT32:
      conditionals.back().value = now.key_value().value().int_value();
      break;
    case proto_query::DB_DOUBLE:
      conditionals.back().value = now.key_value().value().double_value();
      break;
    case proto_query::DB_STRING:
      conditionals.back().value = now.key_value().value().str_value();
      break;
    case proto_query::DB_BOOL:
      conditionals.back().value = now.key_value().value().bool_value();
      break;
    default:
      break;
    }
  }
  return conditionals;
}

class QueryService final : public proto_query::QueryService::Service {
public:
  Database *db;
  explicit QueryService(Database *database) : db(database) {}
  virtual ::grpc::Status GetQuery(::grpc::ServerContext *context, const ::proto_query::Query *req,
                                  ::proto_query::QueryResponse *res) {
    std::cout << "Command: " << CommandType_Name(req->command()) << std::endl;

    switch (req->command()) {
    case proto_query::CMD_INSERT: {
      auto q_res =
          db->InsertElement({req->parent(), req->schema(), proto_new_fields_to_db_new_fields(req->new_fields())});
      res->set_ok(q_res.ok_);
      res->set_error_message(q_res.error_);
      if (q_res.ok_) {
        res->set_inserted_id(get<int64_t>(q_res.payload_));
      }
      break;
    }
    case proto_query::CMD_FIND: {
      auto q_res = db->GetElements({req->schema(), proto_new_fields_to_db_conds(req->cond())});
      res->set_ok(q_res.ok_);
      res->set_error_message(q_res.error_);
      if (q_res.ok_) {
        for (const auto &now : std::get<std::vector<Element>>(q_res.payload_)) {
          auto elem = res->add_elements();
          elem->set_id(now.id_);
          elem->set_schema(req->schema());
          for (const auto &now2 : now.fields_) {
            auto kv = elem->add_key_values();
            kv->set_key(now2.first);
            switch (now2.second.index()) {
            case DB_INT_32:
              kv->mutable_value()->set_value_type(proto_query::DB_INT32);
              kv->mutable_value()->set_int_value(get<int32_t>(now2.second));
              break;
            case proto_query::DB_DOUBLE:
              kv->mutable_value()->set_value_type(proto_query::DB_DOUBLE);
              kv->mutable_value()->set_double_value(get<double>(now2.second));
              break;
            case proto_query::DB_STRING:
              kv->mutable_value()->set_value_type(proto_query::DB_STRING);
              kv->mutable_value()->set_str_value(get<std::string>(now2.second));
              break;
            case proto_query::DB_BOOL:
              kv->mutable_value()->set_value_type(proto_query::DB_BOOL);
              kv->mutable_value()->set_bool_value(get<bool>(now2.second));
              break;
            default:
              break;
            }
          }
        }
      }
      break;
    }
    case proto_query::CMD_UPDATE: {
      auto q_res = db->UpdateElements({req->schema(), proto_new_fields_to_db_conds(req->cond()),
                                       proto_new_fields_to_db_new_fields(req->new_fields())});
      res->set_ok(q_res.ok_);
      res->set_error_message(q_res.error_);
      break;
    }
    case proto_query::CMD_DELETE: {
      auto q_res = db->DeleteElements({req->schema(), proto_new_fields_to_db_conds(req->cond())});
      res->set_ok(q_res.ok_);
      res->set_error_message(q_res.error_);
      break;
    }
    default:
      res->set_ok(false);
      res->set_error_message("Unknown command");
      break;
    }
    std::cout << "Status: " << (res->ok() ? "OK" : "Failed") << "." << std::endl;
    if (!res->ok()) {
      std::cout << "Error: " << res->error_message() << "." << std::endl;
    }
    return grpc::Status::OK;
  }
};

int main(int argc, char *argv[]) {
  if (argc <= 1) {
    std::cout << "Usage: ./server host:port file";
    return 0;
  }
  grpc::ServerBuilder builder;
  builder.AddListeningPort(argv[1], grpc::InsecureServerCredentials());
  auto db = Database(argv[2], true);
  QueryService my_service(&db);
  my_service.db->CreateSchema({"MainSchema",
                               {
                                   {"int_pole", DB_INT_32},
                                   {"bool_pole", DB_BOOL},
                                   {"str_pole", DB_STRING},
                               }});
  my_service.db->CreateSchema({"SCHEMA",
                               {
                                   {"pole", DB_STRING},
                                   {"vtoroe_pole", DB_BOOL},
                               }});
  builder.RegisterService(&my_service);

  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  std::cout << "Server started" << std::endl;
  server->Wait();

  return 0;
}
