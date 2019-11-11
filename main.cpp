#include <GL/glut.h>
#include <GL/glu.h>
#include "machine.h"

int main(int argc, char *argv[]) {
  Machine machine;
  machine.load("./game/build/kernel.txt");
  machine.inject_qqt();
  machine.run();
  return 0;
}
