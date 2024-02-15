#include "store/api/storage.h"
#include <gtest/gtest.h>
#include <format>

TEST(StorageTest, set_key_to_null_twice) {
  constexpr bool replace_if_exists = true;
  auto store = new Storage(std::format(
      R"(D:\Projects\itmo-llp\tests\store\out\storage_test-{}.db)",
      std::time(nullptr)),
                  replace_if_exists);
  auto rt = store->root();
  EXPECT_TRUE(rt.has_value());
  EXPECT_NE(0, rt->get_ref().value());
  std::cout << "root@" << rt->get_ref().value() << std::endl;

  auto kek = store->set(*rt, "kek", 1);
  EXPECT_TRUE(kek.has_value());
  EXPECT_NE(0, kek->get_ref().value());
  std::cout << "kek<int>@" << kek->get_ref().value() << std::endl;

  auto kek_int = store->read(*kek);
  EXPECT_TRUE(kek.has_value());
  EXPECT_TRUE(std::holds_alternative<std::int32_t>(*kek_int));
  EXPECT_EQ(1, std::get<std::int32_t>(*kek_int));
  std::cout << "get<int>(kek):" << std::get<std::int32_t>(*kek_int) << std::endl;

  auto lol = store->set(*rt, "lol", "idk");
  EXPECT_TRUE(lol.has_value());
  EXPECT_NE(0, lol->get_ref().value());
  std::cout << "lol<str>@" << lol->get_ref().value() << std::endl;

  auto lol_str = store->read(*lol);
  EXPECT_TRUE(std::holds_alternative<std::string>(*lol_str));
  auto str_val =  std::get<std::string>(*lol_str);
  EXPECT_EQ("idk", str_val);
  std::cout << "get<str>(lol):" << str_val << std::endl;

//  auto n = store->get(*rt, "kek");
//  EXPECT_TRUE(n.has_value());
//  EXPECT_TRUE(n->get_ref().value() != 0);
//
//  auto v = store->read(*n);
//  EXPECT_TRUE(v.has_value());
//  EXPECT_TRUE(std::holds_alternative<std::int32_t>(*v));
//
//  if (v && std::holds_alternative<std::int32_t>(*v)) {
//    std::cout << std::get<std::int32_t>(*v) << std::endl;
//  }
}

