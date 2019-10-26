#pragma once
#include <cstdint>

enum Flag{
  CF = 0,
  PF = 2,
  AF = 4,
  ZF = 6,
  SF = 7,
  TF = 8,
  IF = 9,
  DF = 10,
  OF = 11
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
  void set_flag(Flag g) {
    uint16_t w = 1 << uint16_t(g);
    FR |= w;
  }
  void unset_flag(Flag g) {
    uint16_t w = ~(1 << uint16_t(g));
    FR &= w;
  }
};
