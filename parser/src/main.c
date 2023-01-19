#include <stdio.h>

#include "lexer.h"
#include "parser.h"

int main (void) {
  return yyparse();
}
