extern "C" {
#include "parser.h"
}
#include "message.pb.h"
#include <iostream>
int main() {
  message::Message m;
  std::cout << "Hello Proto!" << std::endl;
  return yyparse();
}
