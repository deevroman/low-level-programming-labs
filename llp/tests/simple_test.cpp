#include <gtest/gtest.h>

#include "Database.h"

TEST(HelloTest, BasicAssertions) {
  Database db = Database("../tmp/file.hex", true);
  EXPECT_NE(db.file_, nullptr);
  db.Query({CREATE_SCHEMA, query_payload(create_schema_query("BOM-BOM-BOM-BOM", {
                                                                                    {"pole_poolyushko", DB_INT_32},
                                                                                    {"bool_poolyushko", DB_BOOL},
                                                                                }))});
  db.Query({CREATE_SCHEMA, query_payload(create_schema_query("SUKO-SUKOO-SUKO", {
                                                                                    {"AHAHA-AHA-AHAHA", DB_FLOAT},
                                                                                    {"OLOLOLO-OLOLOLO", DB_STRING},
                                                                                }))});
  db.Query({CREATE_SCHEMA, query_payload(create_schema_query("WEWE-WEWEE-WEWE", {
                                                                                    {"12345-678-90123", DB_FLOAT},
                                                                                    {"7654321-1234567", DB_STRING},
                                                                                }))});
  auto kek = db.GetSchemas();
  EXPECT_TRUE(kek.ok_);
  auto lol = std::get<std::vector<Schema>>(kek.payload_);
  EXPECT_EQ(lol[0].name_, "BOM-BOM-BOM-BOM");
  EXPECT_EQ(lol[0].fields_["bool_poolyushko"], DB_BOOL);
  EXPECT_EQ(lol[0].fields_["pole_poolyushko"], DB_INT_32);
  EXPECT_EQ(lol[1].name_, "SUKO-SUKOO-SUKO");
  EXPECT_EQ(lol[1].fields_["AHAHA-AHA-AHAHA"], DB_FLOAT);
  EXPECT_EQ(lol[1].fields_["OLOLOLO-OLOLOLO"], DB_STRING);
  EXPECT_EQ(lol[2].name_, "WEWE-WEWEE-WEWE");
  EXPECT_EQ(lol[2].fields_["12345-678-90123"], DB_FLOAT);
  EXPECT_EQ(lol[2].fields_["7654321-1234567"], DB_STRING);

  auto fail =
      db.Query({CREATE_SCHEMA, query_payload(create_schema_query("WEWE-WEWEE-WEWE", {
                                                                                        {"12345-678-90123", DB_FLOAT},
                                                                                        {"7654321-1234567", DB_STRING},
                                                                                    }))});
  EXPECT_FALSE(fail.ok_);

  db.Query({DELETE_SCHEMA, query_payload(delete_schema_query("BOM-BOM-BOM-BOM"))});
  db.Query({DELETE_SCHEMA, query_payload(delete_schema_query("WEWE-WEWEE-WEWE"))});

  kek = db.GetSchemas();
  EXPECT_TRUE(kek.ok_);
  auto lal = std::get<std::vector<Schema>>(kek.payload_);
  EXPECT_EQ(lal.size(), 1);
  EXPECT_EQ(lal[0].name_, "SUKO-SUKOO-SUKO");
  EXPECT_EQ(lal[0].fields_["AHAHA-AHA-AHAHA"], DB_FLOAT);
  EXPECT_EQ(lal[0].fields_["OLOLOLO-OLOLOLO"], DB_STRING);

  db.Query({INSERT, query_payload(insert_query("WEWE-WEWEE-WEWE", "SUKO-SUKOO-SUKO",
                                               {
                                                   {"12345-678-90123", 3.14},
                                                   {"7654321-1234567", "URA, INSERT"},
                                               }))});

  for (auto now : lal) {
    now.Print();
    std::cout << std::endl;
  }
}
