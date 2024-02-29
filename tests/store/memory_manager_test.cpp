#include "store/memory/memory_manager.h"
#include <gtest/gtest.h>
#include <format>

TEST(MemoryManagerTest, memory_manager_logic) {
  auto file =  fs::File(R"(D:\Projects\itmo-llp\tests\store\out\memory_manager_test.db)");
  LARGE_INTEGER liFileSize;
  if (!GetFileSizeEx(file.handle(), &liFileSize)) {
    throw std::runtime_error{"Can't get file size."};
  }
  std::cout<<liFileSize.QuadPart << std::endl;
  if (liFileSize.QuadPart == 0ll) {
    liFileSize.QuadPart = 1 << 12; // 4 KB
    if (!SetFilePointerEx(file.handle(), liFileSize, nullptr, FILE_BEGIN)) {
      throw std::runtime_error{"Can't resize file. It still has zero size."};
    }
    if (!SetEndOfFile(file.handle())) {
      throw std::runtime_error{"Can't save file size. It still has zero size."};
    }
  }
  std::cout<<liFileSize.QuadPart << std::endl;
  LARGE_INTEGER liAgainFileSize;
  if (!GetFileSizeEx(file.handle(), &liAgainFileSize)) {
    throw std::runtime_error{"Can't get file size."};
  }
  std::cout<<liAgainFileSize.QuadPart << std::endl;
}

#define STRINGIFY(x) #x
#define TO_STRING(x) STRINGIFY(x)
#define UNIQUE() TO_STRING(__COUNTER__)
TEST(MemoryManagerTest, read_write) {
  auto file = std::make_unique<fs::File>(
      std::format(R"(D:\Projects\itmo-llp\tests\store\out\mm_test_rw-{}.db)",std::time(nullptr)));
  auto mem = new mem::MemoryManager{std::move(file), true};
  auto rt_off = mem->root_ref();
  EXPECT_TRUE(rt_off.has_value());
  std::cout << "root off: " << rt_off->value() << std::endl;

  auto obj = mem->read<dom::Value>(*rt_off);
  EXPECT_TRUE(obj.has_value());
  EXPECT_TRUE(obj->is<dom::ObjectValue>());

  auto res = mem->write(*rt_off, dom::Int32Value(0x5555'8888));
  EXPECT_TRUE(res);

  auto int_val = mem->read<dom::Value>(*rt_off);
  EXPECT_TRUE(int_val.has_value());
  EXPECT_TRUE(int_val->is<dom::Int32Value>());
  EXPECT_EQ(0x5555'8888, int_val->as<dom::Int32Value>().get_int());
}

TEST(MemoryManageTest, alloc_save_reference) {
  auto file   = std::make_unique<fs::File>(std::format(
      R"(D:\Projects\itmo-llp\tests\store\out\mm_test_alloc_save_ref-{}.db)",
      std::time(nullptr)));
  auto mem    = new mem::MemoryManager{std::move(file), true};
  auto rt_off = mem->root_ref();
  EXPECT_TRUE(rt_off.has_value());
  auto rt = mem->read<dom::Value>(*rt_off);
  EXPECT_TRUE(rt.has_value());
  EXPECT_TRUE(rt->is<dom::ObjectValue>());

  std::string_view str = "keklolidk";

  auto str_ref = mem->alloc<char>(str.size());
  EXPECT_EQ(str.size(), mem->read<size_t>(str_ref));
  auto res_str_data = mem->write(str_ref.after<size_t>(), str.data(), str.size());
  EXPECT_TRUE(res_str_data);
  auto res_str_ref = mem->write(*rt_off, dom::StringValue{str_ref});
  EXPECT_TRUE(res_str_ref);

  auto str_val = mem->read<dom::Value>(*rt_off);
  EXPECT_TRUE(str_val.has_value());
  EXPECT_TRUE(str_val->is<dom::StringValue>());

  auto str_ref_read = str_val->as<dom::StringValue>().get_ref();
  EXPECT_EQ(str_ref_read.value(), str_ref.value());

  auto chars = mem->read_all<char>(str_ref);
  EXPECT_TRUE(chars.has_value());

  auto read_str = std::string_view{chars->data(), chars->size()};
  EXPECT_EQ(str, read_str);
}