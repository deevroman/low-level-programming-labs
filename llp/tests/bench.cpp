#include <gtest/gtest.h>

#include "Database.h"

double GetTime() { return 1. * clock() / CLOCKS_PER_SEC; }

TEST(BenchTest, OnlyInserts) {
  std::ios_base::sync_with_stdio(false);
  Database db = Database("../tmp/file.hex", true);

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
  db.CreateSchema({"WEWE-WEWEE-WEWE",
                   {
                       {"12345-678-90123", DB_DOUBLE},
                       {"7654321-1234567", DB_STRING},
                   }});
  auto start_time = GetTime();
  auto last_time = start_time;
  auto root_id = get<int64_t>(db.InsertElement({0,
                                                "SCHEMKA-SCHEMKA",
                                                {
                                                    {"AHAHA-AHA-AHAHA", 3.14},
                                                    {"OLOLOLO-OLOLOLO", "URA, INSERT"},
                                                }})
                                  .payload_);
  for (int i = 0; i < 30; i++) {
    for (int j = 0; j < 10000; j++) {
      db.InsertElement({root_id,
                        "SCHEMKA-SCHEMKA",
                        {
                            {"AHAHA-AHA-AHAHA", last_time},
                            {"OLOLOLO-OLOLOLO", "URA, INSERT" + std::to_string(j)},
                        }});
    }
    auto now = GetTime();
    std::cout << (i + 1) * 10000 + 1 << '\t' << now - last_time << '\t' << db.GetFileSize() << std::endl;
    last_time = now;
  }
}

TEST(BenchTest, InsertsAndSelect) {
  std::ios_base::sync_with_stdio(false);
  Database db = Database("../tmp/file.hex", true);

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
  db.CreateSchema({"WEWE-WEWEE-WEWE",
                   {
                       {"12345-678-90123", DB_DOUBLE},
                       {"7654321-1234567", DB_STRING},
                   }});
  auto start_time = GetTime();
  auto last_time = start_time;
  auto root_id = get<int64_t>(db.InsertElement({0,
                                                "SCHEMKA-SCHEMKA",
                                                {
                                                    {"AHAHA-AHA-AHAHA", 3.14},
                                                    {"OLOLOLO-OLOLOLO", "URA, INSERT"},
                                                }})
                                  .payload_);
  for (int i = 0; i < 20; i++) {
    for (int j = 0; j < 10000; j++) {
      db.InsertElement({root_id,
                        "SCHEMKA-SCHEMKA",
                        {
                            {"AHAHA-AHA-AHAHA", last_time},
                            {"OLOLOLO-OLOLOLO", "URA, INSERT" + std::to_string(j)},
                        }});
    }
    last_time = GetTime();
    db.GetElements({"SCHEMKA-SCHEMKA", {{"AHAHA-AHA-AHAHA", OP_LESS, 3.14}}});
    auto now = GetTime();
    std::cout << (i + 1) * 10000 + 1 << '\t' << now - last_time << '\t' << db.GetFileSize() << std::endl;
    last_time = now;
  }
}

TEST(BenchTest, InsertsAndRemove) {
  std::ios_base::sync_with_stdio(false);
  Database db = Database("../tmp/file.hex", true);

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
  db.CreateSchema({"WEWE-WEWEE-WEWE",
                   {
                       {"12345-678-90123", DB_DOUBLE},
                       {"7654321-1234567", DB_STRING},
                   }});
  auto start_time = GetTime();
  auto last_time = start_time;
  auto root_id = get<int64_t>(db.InsertElement({0,
                                                "SCHEMKA-SCHEMKA",
                                                {
                                                    {"AHAHA-AHA-AHAHA", 3.14},
                                                    {"OLOLOLO-OLOLOLO", "URA, INSERT"},
                                                }})
                                  .payload_);
  for (int i = 0; i < 20; i++) {
    for (int j = 0; j < 10000; j++) {
      db.InsertElement({root_id,
                        "SCHEMKA-SCHEMKA",
                        {
                            {"AHAHA-AHA-AHAHA", last_time},
                            {"OLOLOLO-OLOLOLO", "URA, INSERT" + std::to_string(j)},
                        }});
      db.InsertElement({root_id,
                        "SCHEMKA-SCHEMKA",
                        {
                            {"AHAHA-AHA-AHAHA", 3.14},
                            {"OLOLOLO-OLOLOLO", "URA, INSERT" + std::to_string(j)},
                        }});
    }
    last_time = GetTime();
    db.DeleteElements({"SCHEMKA-SCHEMKA", {{"AHAHA-AHA-AHAHA", OP_EQUAL, 3.14}}});
    auto now = GetTime();
    std::cout << (i + 1) * 10000 + 1 << '\t' << now - last_time << '\t' << db.GetFileSize() << std::endl;
    last_time = now;
  }
}
