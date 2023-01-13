#include <gtest/gtest.h>

#include "Database.h"

TEST(InsertElementsAndListTest, OnlyRootElement) {
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
                       {"OLOLOLO-OLOLOLO", DB_STRING},
                   }});
  db.CreateSchema({"WEWE-WEWEE-WEWE",
                   {
                       {"12345-678-90123", DB_DOUBLE},
                       {"7654321-1234567", DB_STRING},
                   }});

  auto kek = db.InsertElement({0,
                               "SCHEMKA-SCHEMKA",
                               {
                                   {"12345-678-90123", 3.14},
                                   {"7654321-1234567", "URA, INSERT"},
                               }});
  ASSERT_FALSE(kek.ok_);
  kek = db.InsertElement({0,
                          "SCHEMKA-SCHEMKA",
                          {
                              {"AHAHA-AHA-AHAHA", 3.14},
                              {"OLOLOLO-OLOLOLO", "URA, INSERT"},
                          }});
  ASSERT_TRUE(kek.ok_);

  kek = db.GetElements();
  ASSERT_TRUE(kek.ok_);
  auto omg = get<std::vector<Element>>(kek.payload_);
  ASSERT_EQ(omg.size(), 1);
  ASSERT_EQ(get<double>(omg[0].fields_["AHAHA-AHA-AHAHA"]), 3.14);
  ASSERT_EQ(get<std::string>(omg[0].fields_["OLOLOLO-OLOLOLO"]), "URA, INSERT");
  for (auto now : omg) {
    now.Print();
    std::cout << std::endl;
  }
}

TEST(InsertElementsAndListTest, TwoElements) {
  Database db = Database("../tmp/file.hex", true);
  ASSERT_NE(db.GetFileSize(), 0);
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
  ASSERT_FALSE(kek.ok_);
  kek = db.InsertElement({0,
                          "SCHEMKA-SCHEMKA",
                          {
                              {"AHAHA-AHA-AHAHA", 3.14},
                              {"OLOLOLO-OLOLOLO", "URA, INSERT"},
                          }});
  ASSERT_TRUE(kek.ok_);
  auto root_id = get<int64_t>(kek.payload_);
  kek = db.InsertElement({0,
                          "SCHEMKA-SCHEMKA",
                          {
                              {"AHAHA-AHA-AHAHA", 3.14},
                              {"OLOLOLO-OLOLOLO", "URA, INSERT"},
                          }});
  ASSERT_FALSE(kek.ok_);

  kek = db.InsertElement({root_id,
                          "SCHEMKA-SCHEMKA",
                          {
                              {"AHAHA-AHA-AHAHA", 3.14},
                              {"OLOLOLO-OLOLOLO", "URA, INSERT2"},
                          }});
  ASSERT_TRUE(kek.ok_);

  kek = db.GetElements();
  ASSERT_TRUE(kek.ok_);
  auto omg = get<std::vector<Element>>(kek.payload_);
  ASSERT_EQ(omg.size(), 2);
  ASSERT_EQ(get<double>(omg[0].fields_["AHAHA-AHA-AHAHA"]), 3.14);
  ASSERT_EQ(get<std::string>(omg[0].fields_["OLOLOLO-OLOLOLO"]), "URA, INSERT2");
  ASSERT_EQ(get<double>(omg[1].fields_["AHAHA-AHA-AHAHA"]), 3.14);
  ASSERT_EQ(get<std::string>(omg[1].fields_["OLOLOLO-OLOLOLO"]), "URA, INSERT");
  for (auto now : omg) {
    now.Print();
    std::cout << std::endl;
  }
}

TEST(InsertElementsAndListTest, DeleteElements) {
  Database db = Database("../tmp/file.hex", true);
  ASSERT_NE(db.GetFileSize(), 0);
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
  ASSERT_FALSE(kek.ok_);
  kek = db.InsertElement({0,
                          "SCHEMKA-SCHEMKA",
                          {
                              {"AHAHA-AHA-AHAHA", 3.14},
                              {"OLOLOLO-OLOLOLO", "URA, INSERT"},
                          }});
  ASSERT_TRUE(kek.ok_);
  auto root_id = get<int64_t>(kek.payload_);
  kek = db.InsertElement({0,
                          "SCHEMKA-SCHEMKA",
                          {
                              {"AHAHA-AHA-AHAHA", 3.14},
                              {"OLOLOLO-OLOLOLO", "URA, INSERT"},
                          }});
  ASSERT_FALSE(kek.ok_);

  kek = db.InsertElement({root_id,
                          "SCHEMKA-SCHEMKA",
                          {
                              {"AHAHA-AHA-AHAHA", 3.14},
                              {"OLOLOLO-OLOLOLO", "URA, INSERT2"},
                          }});
  ASSERT_TRUE(kek.ok_);
  auto child_id = get<int64_t>(kek.payload_);

  kek = db.DeleteElement(root_id);
  ASSERT_FALSE(kek.ok_);
  kek = db.DeleteElement(child_id);
  ASSERT_TRUE(kek.ok_);

  kek = db.GetElements();
  ASSERT_TRUE(kek.ok_);
  auto omg = get<std::vector<Element>>(kek.payload_);
  ASSERT_EQ(omg.size(), 1);
  ASSERT_EQ(get<double>(omg[0].fields_["AHAHA-AHA-AHAHA"]), 3.14);
  ASSERT_EQ(get<std::string>(omg[0].fields_["OLOLOLO-OLOLOLO"]), "URA, INSERT");

  for (auto now : omg) {
    now.Print();
    std::cout << std::endl;
  }
}

TEST(InsertElementsAndListTest, UpdateElement) {
  Database db = Database("../tmp/file.hex", true);
  ASSERT_NE(db.GetFileSize(), 0);
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
                              {"AHAHA-AHA-AHAHA", 3.14},
                              {"OLOLOLO-OLOLOLO", "URA, INSERT"},
                          }});
  ASSERT_TRUE(kek.ok_);
  auto root_id = get<int64_t>(kek.payload_);

  kek = db.InsertElement({root_id,
                          "SCHEMKA-SCHEMKA",
                          {
                              {"AHAHA-AHA-AHAHA", 3.14},
                              {"OLOLOLO-OLOLOLO", "URA, INSERT2"},
                          }});
  ASSERT_TRUE(kek.ok_);
  auto child_id = get<int64_t>(kek.payload_);

  kek = db.UpdateElements(
      {{"SCHEMKA-SCHEMKA", {{"OLOLOLO-OLOLOLO", OP_EQUAL, "URA, INSERT2"}}}, {{"OLOLOLO-OLOLOLO", "URA, UPDATE"}}});
  ASSERT_TRUE(kek.ok_);

  kek = db.GetElementByPath({root_id, child_id});
  ASSERT_TRUE(kek.ok_);
  auto elem = get<Element>(kek.payload_);
  ASSERT_EQ(get<double>(elem.fields_["AHAHA-AHA-AHAHA"]), 3.14);
  ASSERT_EQ(get<std::string>(elem.fields_["OLOLOLO-OLOLOLO"]), "URA, UPDATE");

  kek = db.GetElements();
  ASSERT_TRUE(kek.ok_);
  auto omg = get<std::vector<Element>>(kek.payload_);
  ASSERT_EQ(omg.size(), 2);
  ASSERT_EQ(get<double>(omg[0].fields_["AHAHA-AHA-AHAHA"]), 3.14);
  ASSERT_EQ(get<std::string>(omg[0].fields_["OLOLOLO-OLOLOLO"]), "URA, UPDATE");
  ASSERT_EQ(get<double>(omg[1].fields_["AHAHA-AHA-AHAHA"]), 3.14);
  ASSERT_EQ(get<std::string>(omg[1].fields_["OLOLOLO-OLOLOLO"]), "URA, INSERT");

  for (auto now : omg) {
    now.Print();
    std::cout << std::endl;
  }
}

