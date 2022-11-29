#include <gtest/gtest.h>
#include "database.h"

TEST(HelloTest, BasicAssertions) {
    database db = database("../tmp/file.hex", true);
    EXPECT_NE(db.file, nullptr);
    db.query({CREATE_SCHEMA, query_payload(
            create_schema_query(
                    "BOM-BOM",
                    {
                            {"pole",      DB_INT_32},
                            {"bool_pole", DB_BOOL},
                    }
            )
    )});
    db.query({CREATE_SCHEMA, query_payload(
            create_schema_query(
                    "SUKO-SUKO",
                    {
                            {"AHAHA",   DB_FLOAT},
                            {"OLOLOLO", DB_STRING},
                    }
            )
    )});
    auto kek = db.get_schemas();
    EXPECT_TRUE(kek.ok);
    auto lol = std::get<std::vector<schema>>(kek.payload);
    EXPECT_EQ(lol[0].name, "BOM-BOM");
    EXPECT_EQ(lol[0].fields["bool_pole"], DB_BOOL);
    EXPECT_EQ(lol[0].fields["pole"], DB_INT_32);

}
