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

  ASSERT_FALSE(db.InsertElement({0,
                                 "SCHEMKA-SCHEMKA",
                                 {
                                     {"12345-678-90123", 3.14},
                                     {"7654321-1234567", "URA, INSERT"},
                                 }})
                   .ok_);
  ASSERT_TRUE(db.InsertElement({0,
                                "SCHEMKA-SCHEMKA",
                                {
                                    {"AHAHA-AHA-AHAHA", 3.14},
                                    {"OLOLOLO-OLOLOLO", "URA, INSERT"},
                                }})
                  .ok_);

  auto _ = db.GetElements();
  ASSERT_TRUE(_.ok_);
  auto res = get<std::vector<Element>>(_.payload_);
  ASSERT_EQ(res.size(), 1);
  ASSERT_EQ(get<double>(res[0].fields_["AHAHA-AHA-AHAHA"]), 3.14);
  ASSERT_EQ(get<std::string>(res[0].fields_["OLOLOLO-OLOLOLO"]), "URA, INSERT");
  for (auto now : res) {
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

  ASSERT_FALSE(db.InsertElement({0,
                                 "SCHEMKA-SCHEMKA",
                                 {
                                     {"12345-678-90123", 3.14},
                                     {"7654321-1234567", "URA, INSERT"},
                                 }})
                   .ok_);

  auto _ = db.InsertElement({0,
                             "SCHEMKA-SCHEMKA",
                             {
                                 {"AHAHA-AHA-AHAHA", 3.14},
                                 {"OLOLOLO-OLOLOLO", "URA, INSERT"},
                             }});
  ASSERT_TRUE(_.ok_);
  auto root_id = get<int64_t>(_.payload_);
  ASSERT_FALSE(db.InsertElement({0,
                                 "SCHEMKA-SCHEMKA",
                                 {
                                     {"AHAHA-AHA-AHAHA", 3.14},
                                     {"OLOLOLO-OLOLOLO", "URA, INSERT"},
                                 }})
                   .ok_);

  ASSERT_TRUE(db.InsertElement({root_id,
                                "SCHEMKA-SCHEMKA",
                                {
                                    {"AHAHA-AHA-AHAHA", 3.14},
                                    {"OLOLOLO-OLOLOLO", "URA, INSERT2"},
                                }})
                  .ok_);

  _ = db.GetElements();
  ASSERT_TRUE(_.ok_);
  auto res = get<std::vector<Element>>(_.payload_);
  ASSERT_EQ(res.size(), 2);
  ASSERT_EQ(get<double>(res[0].fields_["AHAHA-AHA-AHAHA"]), 3.14);
  ASSERT_EQ(get<std::string>(res[0].fields_["OLOLOLO-OLOLOLO"]), "URA, INSERT2");
  ASSERT_EQ(get<double>(res[1].fields_["AHAHA-AHA-AHAHA"]), 3.14);
  ASSERT_EQ(get<std::string>(res[1].fields_["OLOLOLO-OLOLOLO"]), "URA, INSERT");
  for (auto now : res) {
    now.Print();
    std::cout << std::endl;
  }
}

TEST(InsertElementsAndListTest, BrokenPapentIds) {
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
                         {"some", DB_DOUBLE},
                         {"some2", DB_DOUBLE},
                         {"OLOLOLO-OLOLOLO", DB_STRING},
                     }});
    db.CreateSchema({"WEWE-WEWEE-WEWE",
                     {
                         {"12345-678-90123", DB_DOUBLE},
                         {"7654321-1234567", DB_STRING},
                     }});
  }

  auto _ = db.InsertElement({0,
                             "SCHEMKA-SCHEMKA",
                             {
                                 {"AHAHA-AHA-AHAHA", 3.14},
                                 {"some", 3.14},
                                 {"some2", 3.14},
                                 {"OLOLOLO-OLOLOLO", "URA, INSERT"},
                             }});
  ASSERT_TRUE(_.ok_);
  auto root_id = get<int64_t>(_.payload_);

  ASSERT_TRUE(db.InsertElement({root_id,
                                "SCHEMKA-SCHEMKA",
                                {
                                    {"AHAHA-AHA-AHAHA", 3.14},
                                    {"some", 3.14},
                                    {"some2", 3.14},
                                    {"OLOLOLO-OLOLOLO", "URA, INSERT2"},
                                }})
                  .ok_);

  ASSERT_FALSE(db.InsertElement({1,
                                 "SCHEMKA-SCHEMKA",
                                 {
                                     {"AHAHA-AHA-AHAHA", 3.14},
                                     {"some", 3.14},
                                     {"some2", 3.14},
                                     {"OLOLOLO-OLOLOLO", "URA, INSERT2"},
                                 }})
                   .ok_);

  ASSERT_FALSE(db.InsertElement({static_cast<DbSize>(root_id + sizeof(PageChunk)),
                                 "SCHEMKA-SCHEMKA",
                                 {
                                     {"AHAHA-AHA-AHAHA", 3.14},
                                     {"some", 3.14},
                                     {"some2", 3.14},
                                     {"OLOLOLO-OLOLOLO", "URA, INSERT2"},
                                 }})
                   .ok_);

  ASSERT_FALSE(db.InsertElement({sizeof(file_header),
                                 "SCHEMKA-SCHEMKA",
                                 {
                                     {"AHAHA-AHA-AHAHA", 3.14},
                                     {"some", 3.14},
                                     {"some2", 3.14},
                                     {"OLOLOLO-OLOLOLO", "URA, INSERT2"},
                                 }})
                   .ok_);

  ASSERT_FALSE(db.InsertElement({sizeof(file_header) + sizeof(page_header),
                                 "SCHEMKA-SCHEMKA",
                                 {
                                     {"AHAHA-AHA-AHAHA", 3.14},
                                     {"some", 3.14},
                                     {"some2", 3.14},
                                     {"OLOLOLO-OLOLOLO", "URA, INSERT2"},
                                 }})
                   .ok_);

  ASSERT_FALSE(db.InsertElement({-1,
                                 "SCHEMKA-SCHEMKA",
                                 {
                                     {"AHAHA-AHA-AHAHA", 3.14},
                                     {"some", 3.14},
                                     {"some2", 3.14},
                                     {"OLOLOLO-OLOLOLO", "URA, INSERT2"},
                                 }})
                   .ok_);

  _ = db.GetElements();
  ASSERT_TRUE(_.ok_);
  auto res = get<std::vector<Element>>(_.payload_);
  ASSERT_EQ(res.size(), 2);
  for (auto now : res) {
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

  auto _ = db.InsertElement({0,
                             "SCHEMKA-SCHEMKA",
                             {
                                 {"12345-678-90123", 3.14},
                                 {"7654321-1234567", "URA, INSERT"},
                             }});
  ASSERT_FALSE(_.ok_);
  _ = db.InsertElement({0,
                        "SCHEMKA-SCHEMKA",
                        {
                            {"AHAHA-AHA-AHAHA", 3.14},
                            {"OLOLOLO-OLOLOLO", "URA, INSERT"},
                        }});
  ASSERT_TRUE(_.ok_);
  auto root_id = get<int64_t>(_.payload_);
  _ = db.InsertElement({0,
                        "SCHEMKA-SCHEMKA",
                        {
                            {"AHAHA-AHA-AHAHA", 3.14},
                            {"OLOLOLO-OLOLOLO", "URA, INSERT"},
                        }});
  ASSERT_FALSE(_.ok_);

  _ = db.InsertElement({root_id,
                        "SCHEMKA-SCHEMKA",
                        {
                            {"AHAHA-AHA-AHAHA", 3.14},
                            {"OLOLOLO-OLOLOLO", "URA, INSERT2"},
                        }});
  ASSERT_TRUE(_.ok_);
  auto child_id = get<int64_t>(_.payload_);

  _ = db.DeleteElement(root_id);
  ASSERT_FALSE(_.ok_);
  _ = db.DeleteElement(child_id);
  ASSERT_TRUE(_.ok_);

  _ = db.GetElements();
  ASSERT_TRUE(_.ok_);
  auto res = get<std::vector<Element>>(_.payload_);
  ASSERT_EQ(res.size(), 1);
  ASSERT_EQ(get<double>(res[0].fields_["AHAHA-AHA-AHAHA"]), 3.14);
  ASSERT_EQ(get<std::string>(res[0].fields_["OLOLOLO-OLOLOLO"]), "URA, INSERT");

  for (auto now : res) {
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

  auto _ = db.InsertElement({0,
                             "SCHEMKA-SCHEMKA",
                             {
                                 {"AHAHA-AHA-AHAHA", 3.14},
                                 {"OLOLOLO-OLOLOLO", "URA, INSERT"},
                             }});
  ASSERT_TRUE(_.ok_);
  auto root_id = get<int64_t>(_.payload_);

  _ = db.InsertElement({root_id,
                        "SCHEMKA-SCHEMKA",
                        {
                            {"AHAHA-AHA-AHAHA", 3.14},
                            {"OLOLOLO-OLOLOLO", "URA, INSERT2"},
                        }});
  ASSERT_TRUE(_.ok_);
  auto child_id = get<int64_t>(_.payload_);

  _ = db.UpdateElements(
      {{"SCHEMKA-SCHEMKA", {{"OLOLOLO-OLOLOLO", OP_EQUAL, "URA, INSERT2"}}}, {{"OLOLOLO-OLOLOLO", "URA, UPDATE"}}});
  ASSERT_TRUE(_.ok_);

  _ = db.GetElementByPath({root_id, child_id});
  ASSERT_TRUE(_.ok_);
  auto elem = get<Element>(_.payload_);
  ASSERT_EQ(get<double>(elem.fields_["AHAHA-AHA-AHAHA"]), 3.14);
  ASSERT_EQ(get<std::string>(elem.fields_["OLOLOLO-OLOLOLO"]), "URA, UPDATE");

  _ = db.GetElements();
  ASSERT_TRUE(_.ok_);
  auto res = get<std::vector<Element>>(_.payload_);
  ASSERT_EQ(res.size(), 2);
  ASSERT_EQ(get<double>(res[0].fields_["AHAHA-AHA-AHAHA"]), 3.14);
  ASSERT_EQ(get<std::string>(res[0].fields_["OLOLOLO-OLOLOLO"]), "URA, UPDATE");
  ASSERT_EQ(get<double>(res[1].fields_["AHAHA-AHA-AHAHA"]), 3.14);
  ASSERT_EQ(get<std::string>(res[1].fields_["OLOLOLO-OLOLOLO"]), "URA, INSERT");

  for (auto now : res) {
    now.Print();
    std::cout << std::endl;
  }
}
