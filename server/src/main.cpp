#include <message.grpc.pb.h>
#include <message.pb.h>

#include <grpc/grpc.h>
#include <grpcpp/server_builder.h>

#include "Database.h"
#include <iostream>

class QueryService final : public proto_query::QueryService::Service {
public:
  Database *db;
  explicit QueryService(Database *database) : db(database) {}
  virtual ::grpc::Status GetQuery(::grpc::ServerContext *context, const ::proto_query::Query *req,
                                  ::proto_query::QueryResponse *res) {
    std::cout << "Server: GetQuery for \"" << CommandType_Name(req->command()) << "\"." << std::endl;

    std::map<std::string, DataItem> fields;
    switch (req->command()) {
    case proto_query::CMD_INSERT: {
      for (const auto &now : req->new_fields()) {
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
      auto q_res = db->InsertElement({req->parent(), req->schema(), fields});
      res->set_ok(q_res.ok_);
      break;
    }
    case proto_query::CMD_FIND: {
      break;
    }
    case proto_query::CMD_UPDATE: {
      break;
    }
    case proto_query::CMD_DELETE: {
      break;
    }
    default:
      res->set_ok(false);
      res->set_answer("kek");
      break;
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
  builder.RegisterService(&my_service);

  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  server->Wait();

  return 0;
}
