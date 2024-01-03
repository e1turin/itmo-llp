#include <gtest/gtest.h>
#include "store/filesystem/store.h"


TEST(HelloTest, StoreTest) {
  EXPECT_EQ(0, store());
}

