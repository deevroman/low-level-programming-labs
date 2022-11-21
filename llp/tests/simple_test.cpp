#include <gtest/gtest.h>
#include "database.h"

TEST(HelloTest, BasicAssertions) {
    database db = database("", true);
    EXPECT_TRUE(db.is_in_file());
}
