extern "C" {
#include "parser.h"
}

#include <message.grpc.pb.h>
#include <message.pb.h>

#include <grpc/grpc.h>
#include <grpcpp/create_channel.h>

#include "types.h"
#include <iostream>

int main(int argc, char *argv[]) {
  if (argc <= 1) {
    std::cout << "Usage: ./client host:port";
    return 0;
  }

  auto channel = grpc::CreateChannel(argv[1], grpc::InsecureChannelCredentials());
  std::unique_ptr<proto_query::QueryService::Stub> stub = proto_query::QueryService::NewStub(channel);

  while (true) {
    grpc::ClientContext context;
    if (!yyparse()) {
      // TODO free q
    }
    q;
    proto_query::Query _query;
    _query.set_schema(q.schema);

    switch (q.command) {
    case CMD_INSERT: {
      _query.set_command(proto_query::CommandType::CMD_INSERT);
      _query.set_parent(q.parent);
      auto cur = q.new_fields;
      while (cur) {
        auto field = _query.add_new_fields();
        field->set_key(cur->field.key);
        switch (cur->field.value.value_type) {
        case DB_INT32: {
          field->mutable_value()->set_value_type(proto_query::DB_INT32);
          field->mutable_value()->set_int_value(cur->field.value.data.int_value);
          break;
        }
        case DB_DOUBLE: {
          field->mutable_value()->set_value_type(proto_query::DB_DOUBLE);
          field->mutable_value()->set_double_value(cur->field.value.data.double_value);
          break;
        }
        case DB_STRING: {
          field->mutable_value()->set_value_type(proto_query::DB_STRING);
          field->mutable_value()->set_str_value(cur->field.value.data.str_value);
          break;
        }
        case DB_BOOL: {
          field->mutable_value()->set_value_type(proto_query::DB_BOOL);
          field->mutable_value()->set_bool_value(cur->field.value.data.bool_value);
          break;
        }
        }
        cur = cur->next;
      }
      break;
    }
    case CMD_FIND:
      _query.set_command(proto_query::CommandType::CMD_FIND);

      break;
    case CMD_UPDATE:
      _query.set_command(proto_query::CommandType::CMD_UPDATE);
      break;
    case CMD_DELETE:
      _query.set_command(proto_query::CommandType::CMD_DELETE);
      break;
    default:
      break;
    }

    // TODO free q
    q = query();
    proto_query::QueryResponse result;
    grpc::Status status = stub->GetQuery(&context, _query, &result);

    std::cout << "Ok: " << (result.ok() ? "True" : "False") << std::endl;
    std::cout << "Answer: " << result.answer() << std::endl;
  }
  return 0;
}
