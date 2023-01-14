#include <gtest/gtest.h>

#include "Database.h"

TEST(InsertSchemasAndListTest, TwoInserts) {
  Database db = Database("../tmp/file.hex", true);
  ASSERT_NE(db.GetFileSize(), 0);
  db.CreateSchema({"BOM-BOM-BOM-BOM",
                   {
                       {"pole_poolyushko", DB_INT_32},
                       {"bool_poolyushko", DB_BOOL},
                   }});
  db.CreateSchema({"SCHEMKA-SCHEMKA",
                   {
                       {"AHAHA-AHA-AHAHA", DB_DOUBLE},
                       {"OresOLO-OresOLO", DB_STRING},
                   }});
  auto _ = db.GetSchemas();
  ASSERT_TRUE(_.ok_);
  auto res = get<std::vector<Schema>>(_.payload_);
  ASSERT_EQ(res[1].name_, "BOM-BOM-BOM-BOM");
  ASSERT_EQ(res[1].fields_["bool_poolyushko"], DB_BOOL);
  ASSERT_EQ(res[1].fields_["pole_poolyushko"], DB_INT_32);
  ASSERT_EQ(res[0].name_, "SCHEMKA-SCHEMKA");
  ASSERT_EQ(res[0].fields_["AHAHA-AHA-AHAHA"], DB_DOUBLE);
  ASSERT_EQ(res[0].fields_["OresOLO-OresOLO"], DB_STRING);
  
  for (auto now : res) {
    now.Print();
    std::cout << std::endl;
  }
}

TEST(InsertSchemasAndListTest, DuplicateInsert) {
  Database db = Database("../tmp/file.hex", true);
  ASSERT_NE(db.GetFileSize(), 0);
  db.CreateSchema({"BOM-BOM-BOM-BOM",
                   {
                       {"pole_poolyushko", DB_INT_32},
                       {"bool_poolyushko", DB_BOOL},
                   }});
  db.CreateSchema({"SCHEMKA-SCHEMKA",
                   {
                       {"AHAHA-AHA-AHAHA", DB_DOUBLE},
                       {"OresOLO-OresOLO", DB_STRING},
                   }});
  db.CreateSchema({"WEWE-WEWEE-WEWE",
                   {
                       {"12345-678-90123", DB_DOUBLE},
                       {"7654321-1234567", DB_STRING},
                   }});
  auto _ = db.GetSchemas();
  ASSERT_TRUE(_.ok_);
  auto res = get<std::vector<Schema>>(_.payload_);
  ASSERT_EQ(res[2].name_, "BOM-BOM-BOM-BOM");
  ASSERT_EQ(res[2].fields_["bool_poolyushko"], DB_BOOL);
  ASSERT_EQ(res[2].fields_["pole_poolyushko"], DB_INT_32);
  ASSERT_EQ(res[1].name_, "SCHEMKA-SCHEMKA");
  ASSERT_EQ(res[1].fields_["AHAHA-AHA-AHAHA"], DB_DOUBLE);
  ASSERT_EQ(res[1].fields_["OresOLO-OresOLO"], DB_STRING);
  ASSERT_EQ(res[0].name_, "WEWE-WEWEE-WEWE");
  ASSERT_EQ(res[0].fields_["12345-678-90123"], DB_DOUBLE);
  ASSERT_EQ(res[0].fields_["7654321-1234567"], DB_STRING);

  auto fail = db.CreateSchema({"WEWE-WEWEE-WEWE",
                               {
                                   {"12345-678-90123", DB_DOUBLE},
                                   {"7654321-1234567", DB_STRING},
                               }});
  ASSERT_FALSE(fail.ok_);
  
  for (auto now : res) {
    now.Print();
    std::cout << std::endl;
  }
}

TEST(InsertSchemasAndRemove, DeleteSchemas) {
  Database db = Database("../tmp/file.hex", true);
  ASSERT_NE(db.GetFileSize(), 0);
  db.CreateSchema({"BOM-BOM-BOM-BOM",
                   {
                       {"pole_poolyushko", DB_INT_32},
                       {"bool_poolyushko", DB_BOOL},
                   }});
  db.CreateSchema({"SCHEMKA-SCHEMKA",
                   {
                       {"AHAHA-AHA-AHAHA", DB_DOUBLE},
                       {"OresOLO-OresOLO", DB_STRING},
                   }});
  db.CreateSchema({"WEWE-WEWEE-WEWE",
                   {
                       {"12345-678-90123", DB_DOUBLE},
                       {"7654321-1234567", DB_STRING},
                   }});
  auto _ = db.GetSchemas();
  ASSERT_TRUE(_.ok_);
  auto res = get<std::vector<Schema>>(_.payload_);
  ASSERT_EQ(res[2].name_, "BOM-BOM-BOM-BOM");
  ASSERT_EQ(res[2].fields_["bool_poolyushko"], DB_BOOL);
  ASSERT_EQ(res[2].fields_["pole_poolyushko"], DB_INT_32);
  ASSERT_EQ(res[1].name_, "SCHEMKA-SCHEMKA");
  ASSERT_EQ(res[1].fields_["AHAHA-AHA-AHAHA"], DB_DOUBLE);
  ASSERT_EQ(res[1].fields_["OresOLO-OresOLO"], DB_STRING);
  ASSERT_EQ(res[0].name_, "WEWE-WEWEE-WEWE");
  ASSERT_EQ(res[0].fields_["12345-678-90123"], DB_DOUBLE);
  ASSERT_EQ(res[0].fields_["7654321-1234567"], DB_STRING);

  auto fail = db.CreateSchema({"WEWE-WEWEE-WEWE",
                               {
                                   {"12345-678-90123", DB_DOUBLE},
                                   {"7654321-1234567", DB_STRING},
                               }});
  ASSERT_FALSE(fail.ok_);

  db.DeleteSchema("BOM-BOM-BOM-BOM");
  db.DeleteSchema("WEWE-WEWEE-WEWE");

  _ = db.GetSchemas();
  ASSERT_TRUE(_.ok_);
  res = get<std::vector<Schema>>(_.payload_);
  ASSERT_EQ(res.size(), 1);
  ASSERT_EQ(res[0].name_, "SCHEMKA-SCHEMKA");
  ASSERT_EQ(res[0].fields_["AHAHA-AHA-AHAHA"], DB_DOUBLE);
  ASSERT_EQ(res[0].fields_["OresOLO-OresOLO"], DB_STRING);

  for (auto now : res) {
    now.Print();
    std::cout << std::endl;
  }
}
