#include <gtest/gtest.h>
#include "database.h"

TEST(HelloTest, BasicAssertions) {
    database db = database("", true);
    EXPECT_FALSE(db.is_in_file());
//    db.query({CREATE_SCHEMA, query_payload(
//            create_schema_query(
//                    "BOM-BOM",
//                    {
//                            {"pole",      DB_INT_32},
//                            {"bool_pole", DB_BOOL},
//                    }
//            )
//    )});
//    for (auto now: db.get_schemas()) {
//
//    }
}
