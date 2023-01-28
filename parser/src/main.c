#include "parser.h"

int main() {
  int status = yyparse();
  print_query(q);
  print_allocations_size();
  return status; 
}
