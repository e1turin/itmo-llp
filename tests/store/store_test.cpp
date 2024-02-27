#include "store/api/storage.h"
#include <gtest/gtest.h>

TEST(StorageTest, store_logic) {
  constexpr bool replace_if_exists = true;
  auto store = new Storage(R"(D:\Projects\itmo-llp\tests\store\res\store_test-2.db)",
                  replace_if_exists);
  auto rt = store->root();
  EXPECT_EQ(true, rt.has_value());
  EXPECT_EQ(true, rt->get_ref().value() != 0);

  auto kek = store->set(*rt, "kek", 1);
  EXPECT_EQ(true, kek.has_value());
  EXPECT_EQ(true, kek->get_ref().value() != 0);

  auto kek_str = store->read(*kek);
  EXPECT_EQ(true, kek.has_value());
  EXPECT_EQ(true, std::holds_alternative<std::string_view >(*kek_str));

  auto lol = store->set(*rt, "lol", "idk");
  EXPECT_EQ(true, lol.has_value());
  EXPECT_EQ(true, lol->get_ref().value() != 0);

  auto n = store->get(*rt, "kek");
  EXPECT_EQ(true, n.has_value());
  EXPECT_EQ(true, n->get_ref().value() != 0);

  auto v = store->read(*n);
  EXPECT_EQ(true, v.has_value());
  EXPECT_EQ(true, std::holds_alternative<std::int32_t>(*v));

  if (v && std::holds_alternative<std::int32_t>(*v)) {
    std::cout << std::get<std::int32_t>(*v) << std::endl;
  }
}

