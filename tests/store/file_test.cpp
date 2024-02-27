#include "store/fs/file.h"
#include <gtest/gtest.h>

TEST(FileTest, open_file) {
  auto file = fs::File {R"(D:\Projects\itmo-llp\tests\store\res\file-test.txt)"};
  auto str = std::string_view {"kek"};

  WriteFile(file.handle(),str.data(),str.size(), nullptr, nullptr);
}
