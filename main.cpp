#include <iostream>
#include "store.h"
#include "store2.h"

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;
  std::cout << store();
  std::cout << store2();

  return 2;
}