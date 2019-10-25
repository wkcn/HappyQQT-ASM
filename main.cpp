#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdio>
using namespace std;

struct Registers {
  union {
    uint16_t AX;
    struct { uint8_t AL, AH; };
  };

  union {
    uint16_t BX;
    struct { uint8_t BL, BH; };
  };

  union {
    uint16_t CX;
    struct { uint8_t CL, CH; };
  };

  union {
    uint16_t DX;
    struct { uint8_t DL, DH; };
  };

  uint16_t IP;
  uint32_t FR;
  uint16_t SP, BP, SI, DI;
  uint16_t CS, DS, SS, ES;
};

int main() {
  Registers reg;
  reg.AX = 0x1234;
  cout << reg.AX << endl;
  cout << 0x12 << endl;
  cout << 0x34 << endl;
  cout << int(reg.AH) << endl;
  cout << int(reg.AL) << endl;
  return 0;
}
