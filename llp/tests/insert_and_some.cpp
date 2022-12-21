#include <gtest/gtest.h>

#include "Database.h"

TEST(InsertAndListTest, BasicAssertions) {
  Database db = Database("../tmp/file.hex", true);
  EXPECT_NE(db.file_, nullptr);
  db.CreateSchema({"BOM-BOM-BOM-BOM",
                   {
                       {"pole_poolyushko", DB_INT_32},
                       {"bool_poolyushko", DB_BOOL},
                   }});
  db.CreateSchema({"SUKO-SUKOO-SUKO",
                   {
                       {"AHAHA-AHA-AHAHA", DB_DOUBLE},
                       {"OLOLOLO-OLOLOLO", DB_STRING},
                   }});
  auto kek = db.GetSchemas();
  EXPECT_TRUE(kek.ok_);
  auto lol = std::get<std::vector<Schema>>(kek.payload_);
  EXPECT_EQ(lol[0].name_, "BOM-BOM-BOM-BOM");
  EXPECT_EQ(lol[0].fields_["bool_poolyushko"], DB_BOOL);
  EXPECT_EQ(lol[0].fields_["pole_poolyushko"], DB_INT_32);
  EXPECT_EQ(lol[1].name_, "SUKO-SUKOO-SUKO");
  EXPECT_EQ(lol[1].fields_["AHAHA-AHA-AHAHA"], DB_DOUBLE);
  EXPECT_EQ(lol[1].fields_["OLOLOLO-OLOLOLO"], DB_STRING);

  for (auto now : lol) {
    now.Print();
    std::cout << std::endl;
  }
}

TEST(InsertAndFindTest, BasicAssertions) {
  Database db = Database("../tmp/file.hex", true);
  EXPECT_NE(db.file_, nullptr);
  db.CreateSchema({"BOM-BOM-BOM-BOM",
                   {
                       {"pole_poolyushko", DB_INT_32},
                       {"bool_poolyushko", DB_BOOL},
                   }});
  db.CreateSchema({"SUKO-SUKOO-SUKO",
                   {
                       {"AHAHA-AHA-AHAHA", DB_DOUBLE},
                       {"OLOLOLO-OLOLOLO", DB_STRING},
                   }});
  db.CreateSchema({"WEWE-WEWEE-WEWE",
                   {
                       {"12345-678-90123", DB_DOUBLE},
                       {"7654321-1234567", DB_STRING},
                   }});
  auto kek = db.GetSchemas();
  EXPECT_TRUE(kek.ok_);
  auto lol = std::get<std::vector<Schema>>(kek.payload_);
  EXPECT_EQ(lol[0].name_, "BOM-BOM-BOM-BOM");
  EXPECT_EQ(lol[0].fields_["bool_poolyushko"], DB_BOOL);
  EXPECT_EQ(lol[0].fields_["pole_poolyushko"], DB_INT_32);
  EXPECT_EQ(lol[1].name_, "SUKO-SUKOO-SUKO");
  EXPECT_EQ(lol[1].fields_["AHAHA-AHA-AHAHA"], DB_DOUBLE);
  EXPECT_EQ(lol[1].fields_["OLOLOLO-OLOLOLO"], DB_STRING);
  EXPECT_EQ(lol[2].name_, "WEWE-WEWEE-WEWE");
  EXPECT_EQ(lol[2].fields_["12345-678-90123"], DB_DOUBLE);
  EXPECT_EQ(lol[2].fields_["7654321-1234567"], DB_STRING);

  for (auto now : lol) {
    now.Print();
    std::cout << std::endl;
  }
}

TEST(IndestAndRemove, BasicAssertions) {
  Database db = Database("../tmp/file.hex", true);
  EXPECT_NE(db.file_, nullptr);
  db.CreateSchema({"BOM-BOM-BOM-BOM",
                   {
                       {"pole_poolyushko", DB_INT_32},
                       {"bool_poolyushko", DB_BOOL},
                   }});
  db.CreateSchema({"SUKO-SUKOO-SUKO",
                   {
                       {"AHAHA-AHA-AHAHA", DB_DOUBLE},
                       {"OLOLOLO-OLOLOLO", DB_STRING},
                   }});
  db.CreateSchema({"WEWE-WEWEE-WEWE",
                   {
                       {"12345-678-90123", DB_DOUBLE},
                       {"7654321-1234567", DB_STRING},
                   }});
  auto kek = db.GetSchemas();
  EXPECT_TRUE(kek.ok_);
  auto lol = std::get<std::vector<Schema>>(kek.payload_);
  EXPECT_EQ(lol[0].name_, "BOM-BOM-BOM-BOM");
  EXPECT_EQ(lol[0].fields_["bool_poolyushko"], DB_BOOL);
  EXPECT_EQ(lol[0].fields_["pole_poolyushko"], DB_INT_32);
  EXPECT_EQ(lol[1].name_, "SUKO-SUKOO-SUKO");
  EXPECT_EQ(lol[1].fields_["AHAHA-AHA-AHAHA"], DB_DOUBLE);
  EXPECT_EQ(lol[1].fields_["OLOLOLO-OLOLOLO"], DB_STRING);
  EXPECT_EQ(lol[2].name_, "WEWE-WEWEE-WEWE");
  EXPECT_EQ(lol[2].fields_["12345-678-90123"], DB_DOUBLE);
  EXPECT_EQ(lol[2].fields_["7654321-1234567"], DB_STRING);

  auto fail = db.CreateSchema({"WEWE-WEWEE-WEWE",
                               {
                                   {"12345-678-90123", DB_DOUBLE},
                                   {"7654321-1234567", DB_STRING},
                               }});
  EXPECT_FALSE(fail.ok_);

  db.DeleteSchema("BOM-BOM-BOM-BOM");
  db.DeleteSchema("WEWE-WEWEE-WEWE");

  kek = db.GetSchemas();
  EXPECT_TRUE(kek.ok_);
  auto lal = std::get<std::vector<Schema>>(kek.payload_);
  EXPECT_EQ(lal.size(), 1);
  EXPECT_EQ(lal[0].name_, "SUKO-SUKOO-SUKO");
  EXPECT_EQ(lal[0].fields_["AHAHA-AHA-AHAHA"], DB_DOUBLE);
  EXPECT_EQ(lal[0].fields_["OLOLOLO-OLOLOLO"], DB_STRING);

  for (auto now : lal) {
    now.Print();
    std::cout << std::endl;
  }
}
