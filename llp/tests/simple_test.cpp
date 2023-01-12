#include <gtest/gtest.h>

#include "Database.h"

TEST(HelloTest, BasicAssertions) {
  Database db = Database("../tmp/file.hex", true);
  EXPECT_NE(db.file_, nullptr);
  {
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
  }
  
  auto kek = db.InsertElement({0,
                               "SCHEMKA-SCHEMKA",
                               {
                                   {"12345-678-90123", 3.14},
                                   {"7654321-1234567", "URA, INSERT"},
                               }});
  EXPECT_FALSE(kek.ok_);
  kek = db.InsertElement({0,
                          "SCHEMKA-SCHEMKA",
                          {
                              {"AHAHA-AHA-AHAHA", 3.14},
                              {"OLOLOLO-OLOLOLO", "URA, INSERT"},
                          }});
  EXPECT_TRUE(kek.ok_);
  auto root_id = get<int64_t>(kek.payload_);
  kek = db.InsertElement({0,
                          "SCHEMKA-SCHEMKA",
                          {
                              {"AHAHA-AHA-AHAHA", 3.14},
                              {"OLOLOLO-OLOLOLO", "URA, INSERT"},
                          }});
  EXPECT_FALSE(kek.ok_);
  
  kek = db.InsertElement({root_id,
                          "SCHEMKA-SCHEMKA",
                          {
                              {"AHAHA-AHA-AHAHA", 3.14},
                              {"OLOLOLO-OLOLOLO", "URA, INSERT2"},
                          }});
  EXPECT_TRUE(kek.ok_);
  auto child_id = get<int64_t>(kek.payload_);
  
  kek = db.DeleteElement(root_id);
  EXPECT_FALSE(kek.ok_);
  kek = db.DeleteElement(child_id);
  EXPECT_TRUE(kek.ok_);
  
  kek = db.GetElements();
  EXPECT_TRUE(kek.ok_);
  auto omg = get<std::vector<Element>>(kek.payload_);
  EXPECT_EQ(omg.size(), 1);
  EXPECT_EQ(get<double>(omg[0].fields_["AHAHA-AHA-AHAHA"]), 3.14);
  EXPECT_EQ(get<std::string>(omg[0].fields_["OLOLOLO-OLOLOLO"]), "URA, INSERT");
  
  for (auto now : omg) {
    now.Print();
    std::cout << std::endl;
  }
}
