extern "C" {
#include "parser.h"
}

#include <query.grpc.pb.h>
#include <query.pb.h>

#include <grpc/grpc.h>
#include <grpcpp/create_channel.h>

#include "types.h"
#include <iostream>

void set_key_value(proto_query::field_key_value *field, field_key_value cur) {
  field->set_key(cur.key);
  switch (cur.value.value_type) {
  case DB_INT32: {
    field->mutable_value()->set_value_type(proto_query::DB_INT32);
    field->mutable_value()->set_int_value(cur.value.data.int_value);
    break;
  }
  case DB_DOUBLE: {
    field->mutable_value()->set_value_type(proto_query::DB_DOUBLE);
    field->mutable_value()->set_double_value(cur.value.data.double_value);
    break;
  }
  case DB_STRING: {
    field->mutable_value()->set_value_type(proto_query::DB_STRING);
    field->mutable_value()->set_str_value(cur.value.data.str_value);
    break;
  }
  case DB_BOOL: {
    field->mutable_value()->set_value_type(proto_query::DB_BOOL);
    field->mutable_value()->set_bool_value(cur.value.data.bool_value);
    break;
  }
  }
}

void node_to_proto_filter(filter *cur_node, proto_query::filter *f, proto_query::Query &q) {
  if (cur_node->left) {
    f->set_left_node(q.cond_size());
    auto cond = q.add_cond();
    node_to_proto_filter(cur_node->left, cond, q);
  }
  if (cur_node->right) {
    f->set_right_node(q.cond_size());
    auto cond = q.add_cond();
    node_to_proto_filter(cur_node->right, cond, q);
  }
  switch (cur_node->op) {
  case OP_AND:
    f->set_op(proto_query::OP_AND);
    break;
  case OP_OR:
    f->set_op(proto_query::OP_OR);
    break;
  case OP_KEY_VALUE:
    f->set_op(proto_query::OP_KEY_VALUE);
    set_key_value(f->mutable_key_value(), cur_node->key_value);
    break;
  case OP_COMP:
    f->set_op(proto_query::OP_COMP);
    set_key_value(f->mutable_key_value(), cur_node->key_value);
    break;
  }
}

int main(int argc, char *argv[]) {
  if (argc <= 1) {
    std::cout << "Usage: ./client host:port";
    return 0;
  }

  auto channel = grpc::CreateChannel(argv[1], grpc::InsecureChannelCredentials());
  std::unique_ptr<proto_query::QueryService::Stub> stub = proto_query::QueryService::NewStub(channel);

  while (true) {
    grpc::ClientContext context;
    std::cout << ">";
    if (yyparse() || q.command == CMD_NONE) {
      free_all();
      std::cout << std::endl;
      continue;
    }
    proto_query::Query _query;
    _query.set_schema(q.schema);

    switch (q.command) {
    case CMD_INSERT: {
      _query.set_command(proto_query::CommandType::CMD_INSERT);
      _query.set_parent(q.parent);
      auto cur = q.new_fields;
      while (cur) {
        auto field = _query.add_new_fields();
        set_key_value(field, cur->field);
        cur = cur->next;
      }
      break;
    }
    case CMD_FIND: {
      _query.set_command(proto_query::CommandType::CMD_FIND);
      auto cond = _query.add_cond();
      node_to_proto_filter(q.cond, cond, _query);
      break;
    }
    case CMD_UPDATE: {
      _query.set_command(proto_query::CommandType::CMD_UPDATE);

      auto cond = _query.add_cond();
      node_to_proto_filter(q.cond, cond, _query);

      auto cur = q.new_fields;
      while (cur) {
        auto field = _query.add_new_fields();
        set_key_value(field, cur->field);
        cur = cur->next;
      }
      break;
    }
    case CMD_DELETE: {
      _query.set_command(proto_query::CommandType::CMD_DELETE);
      auto cond = _query.add_cond();
      node_to_proto_filter(q.cond, cond, _query);
      break;
    }
    default:
      break;
    }

    free_all();
    proto_query::QueryResponse result;
    grpc::Status status = stub->GetQuery(&context, _query, &result);

    std::cout << "Ok: " << (result.ok() ? "True" : "False") << std::endl;
    if (!result.ok()) {
      std::cout << "Error: " << result.error_message() << std::endl;
    } else if (_query.command() == proto_query::CMD_INSERT){
      std::cout << "Inserted id: " << result.inserted_id() << std::endl;
    }
    else if (_query.command() == proto_query::CMD_FIND) {
      std::cout << "Count elements: " << result.elements_size() << std::endl;
      for (const auto &now : result.elements()) {
        std::cout << "id: " << now.id() << "\n";
        std::cout << "schema: " << now.schema() << "\n";
        for (const auto& now2 : now.key_values()) {
          std::cout << now2.key() << ": ";
          if (now2.value().value_type() == proto_query::DB_STRING) {
            std::cout << now2.value().str_value();
          } else if (now2.value().value_type() == proto_query::DB_INT32) {
            std::cout << now2.value().int_value();
          } else if (now2.value().value_type() == proto_query::DB_DOUBLE) {
            std::cout << now2.value().double_value();
          } else if (now2.value().value_type() == proto_query::DB_BOOL) {
            std::cout << (now2.value().bool_value() ? "True" : "False");
          }
          std::cout << "\n";
        }
        std::cout << "\n";
      }
    }
  }
  return 0;
}
