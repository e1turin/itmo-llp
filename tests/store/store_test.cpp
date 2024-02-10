#include <gtest/gtest.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

TEST(HelloTest, StoreTest) {
  const int fd = open("kek.txt", O_RDONLY);
  if (fd == -1) {
    std::cout << "ERROR: fd == -1";
    return;
  }

  struct stat sb;

  if (fstat(fd, &sb) == -1) {
    std::cout << "ERROR: fstat == -1";
    return;
  }

  const void *addr = mmap(nullptr, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

  if (addr == MAP_FAILED) {
    std::cout << "ERROR: map failed";
  }



  EXPECT_EQ(0, 0 /* store() */);
}

