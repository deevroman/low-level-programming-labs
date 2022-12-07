#include <gtest/gtest.h>
#include "Database.h"

TEST(HelloTest, BasicAssertions) {
  Database db = Database("../tmp/file.hex", true);
  EXPECT_NE(db.file_, nullptr);
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
  EXPECT_TRUE(kek.ok_);
  auto lol = std::get<std::vector<Schema>>(kek.payload_);
  EXPECT_EQ(lol[0].name_, "BOM-BOM-BOM-BOM");
  EXPECT_EQ(lol[0].fields_["bool_poolyushko"], DB_BOOL);
  EXPECT_EQ(lol[0].fields_["pole_poolyushko"], DB_INT_32);
  EXPECT_EQ(lol[1].name_, "SUKO-SUKOO-SUKO");
  EXPECT_EQ(lol[1].fields_["AHAHA-AHA-AHAHA"], DB_FLOAT);
  EXPECT_EQ(lol[1].fields_["OLOLOLO-OLOLOLO"], DB_STRING);

  for (auto now : lol) {
    now.Print();
  }
}
