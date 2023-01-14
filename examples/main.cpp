#ifdef DEBUG
// #define _GLIBCXX_DEBUG
#endif

#include "Database.h"
#include <iostream>

int main(int argc, char *argv[]) {
  if (argc <= 1) {
    std::cout << "Usage: ./example path/to/file";
    return 0;
  }
  Database db = Database(argv[1], true);
  db.CreateSchema({"BOM-BOM-BOM-BOM",
                   {
                       {"pole_poolyushko", DB_INT_32},
                       {"bool_poolyushko", DB_BOOL},
                   }});
  db.CreateSchema({"SCHEMKA-SCHEMKA",
                   {
                       {"AHAHA-AHA-AHAHA", DB_DOUBLE},
                       {"OLOLOLO-OLOLOLO", DB_STRING},
                   }});

  auto _ = db.InsertElement({0,
                             "SCHEMKA-SCHEMKA",
                             {
                                 {"AHAHA-AHA-AHAHA", 3.14},
                                 {"OLOLOLO-OLOLOLO", "URA, INSERT"},
                             }});
  auto root_id = get<int64_t>(_.payload_);
  _ = db.InsertElement({root_id,
                        "SCHEMKA-SCHEMKA",
                        {
                            {"AHAHA-AHA-AHAHA", 3.14},
                            {"OLOLOLO-OLOLOLO", "URA, INSERT2"},
                        }});
  auto child_id = get<int64_t>(_.payload_);
  db.DeleteElement(child_id);
  db.UpdateElements(
      {{"SCHEMKA-SCHEMKA", {{"OLOLOLO-OLOLOLO", OP_EQUAL, "URA, INSERT"}}},
       {{"OLOLOLO-OLOLOLO", "URA, UPDATE"}}});
  _ = db.GetElements();
  auto res = get<std::vector<Element>>(_.payload_);
  for (auto now : res) {
    now.Print();
    std::cout << std::endl;
  }
  return 0;
}
