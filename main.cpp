#include <iostream>
#include "store/filesystem/store.h"
#include "store/dom/store2.h"

int main(int argc, char *argv[]) {
  std::cout << store();
  std::cout << store2();

  return 2;
}