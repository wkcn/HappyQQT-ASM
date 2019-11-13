#pragma once
#include <cstdint>

namespace Flag {
  const uint16_t
    CF = 1 << 0,
    PF = 1 << 2,
    AF = 1 << 4,
    ZF = 1 << 6,
    SF = 1 << 7,
    TF = 1 << 8,
    IF = 1 << 9,
    DF = 1 << 10,
    OF = 1 << 11;
};

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
  uint16_t FR;
  uint16_t SP, BP, SI, DI;
  uint16_t CS, DS, SS, ES;
  void set_flag(uint16_t g) {
    FR |= g;
  }
  void unset_flag(uint16_t g) {
    FR &= ~g;
  }
  void set_flag(uint16_t g, bool b) {
    if (b) set_flag(g);
    else unset_flag(g);
  }
  bool get_flag(uint16_t g) {
    return FR & g;
  }
};
