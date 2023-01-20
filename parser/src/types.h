#ifndef PARSER_SRC_TYPES_H_
#define PARSER_SRC_TYPES_H_

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum Command { CMD_INSERT = 0, CMD_FIND, CMD_UPDATE, CMD_DELETE };

static const char *const commands_name[] = {
    [CMD_INSERT] = "Insert",
    [CMD_FIND] = "Find",
    [CMD_UPDATE] = "Update",
    [CMD_DELETE] = "Delete",
};

enum ValueType { DB_INT32 = 0, DB_DOUBLE, DB_STRING, DB_BOOL };
enum Comparator { OP_LT = 0, OP_LET, OP_GT, OP_GET, OP_NE, OP_REGEX };

struct query {
  enum Command command;
  int64_t parent;
  char *schema;
  struct filter *cond;
  struct new_field *new_fields;
};

enum ASTType { OP_AND, OP_OR, OP_KEY_VALUE, OP_COMP };

struct value {
  enum ValueType value_type;
  union {
    uint64_t int_value;
    double double_value;
    bool bool_value;
    char *str_value;
  } data;
};

struct field_key_value {
  char *key;
  struct value value;
};

struct filter {
  struct filter *left;
  struct filter *right;
  struct field_key_value key_value;
  int level;
  enum Comparator comp;
  enum ASTType op;
};

struct new_field {
  struct new_field *next;
  struct field_key_value field;
};

static void print_spaces(int cnt){
  for(int i = 0; i < cnt; i++){
    printf("  ");
  }
}

static void print_field(struct field_key_value key_value) {
  printf("'%s':", key_value.key);
  switch (key_value.value.value_type) {
  case DB_STRING:
    printf("'%s'", key_value.value.data.str_value);
    break;
  case DB_INT32:
    printf("'%lld'", key_value.value.data.int_value);
    break;
  case DB_DOUBLE:
    printf("'%f'", key_value.value.data.double_value);
    break;
  case DB_BOOL:
    printf("'%s'", (key_value.value.data.double_value ? "true" : "false"));
    break;
  }
  printf("\n");
}

static void print_filter(struct filter *f, int level) {
  print_spaces(level);
  switch (f->op) {
  case OP_AND:
  case OP_OR:
    if (f->op == OP_AND) {
      printf("&&\n");
    } else if (f->op == OP_OR) {
      printf("||\n");
    }
    if (f->left) {
      print_filter(f->left, level + (f->level != f->left->level));
    }
    if (f->right) {
      print_filter(f->right, level + (f->level != f->right->level));
    }
    break;
  case OP_KEY_VALUE:
  case OP_COMP:
    print_field(f->key_value);
    break;
  }
}

static void print_fields(struct new_field *f) {
  while (f) {
    print_field(f->field);
    f = f->next;
  }
}

static void print_query(struct query q) {
  printf("Command: %s\n", commands_name[q.command]);
  if (q.command == CMD_INSERT) {
    printf("Parent id: %lld\n", q.parent);
  }
  printf("Schema: %s\n", q.schema);
  size_t filter_count = 0;
  size_t comp_count = 0;
  if (q.cond) {
    printf("Filters:\n");
    print_filter(q.cond, 1);
  }
  if (q.new_fields) {
    printf("New fields: \n");
    print_fields(q.new_fields);
  }
}

#endif // PARSER_SRC_TYPES_H_
