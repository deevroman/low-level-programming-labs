#ifdef DEBUG
//#define _GLIBCXX_DEBUG
#endif

#include <iostream>
#include "Database.h"

int main(int argc, char *argv[]) {
  std::string s = "suka";
  if (argc <= 1) {
    std::cout << "Usage: ./example path/to/file";
    return 0;
  }
  Database db = Database(argv[1], true);
  db.Query({CREATE_SCHEMA, query_payload(
      create_schema_query(
          "BOM-BOM-BOM-BOM",
          {
              {"pole_poolyushko", DB_INT_32},
              {"bool_poolyushko", DB_BOOL},
          }
      )
  )});
  db.Query({CREATE_SCHEMA, query_payload(
      create_schema_query(
          "SUKO-SUKOO-SUKO",
          {
              {"AHAHA-AHA-AHAHA", DB_FLOAT},
              {"OLOLOLO-OLOLOLO", DB_STRING},
          }
      )
  )});
  auto kek = db.GetSchemas();
  auto lol = std::get<std::vector<Schema>>(kek.payload_);

  for (auto now : lol) {
    now.Print();
  }
  return 0;
}
