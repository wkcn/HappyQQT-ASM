#include <thread>

#include "machine.h"
using namespace std;

void run_machine_func(Machine *machine) { machine->run(); }

int main(int argc, char *argv[]) {
  Machine machine;
  machine.load("./game/build/kernel.txt");
  machine.inject_qqt();
  thread thread_machine(run_machine_func, &machine);
  gui_main_func(&machine.gui);
  thread_machine.join();
  return 0;
}
