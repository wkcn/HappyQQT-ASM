#include "machine.h"

int main() {
  Machine machine;
  machine.load("./game/build/kernel.txt");
  machine.run();
  return 0;
}
