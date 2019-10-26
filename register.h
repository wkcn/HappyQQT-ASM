#pragma once
#include <cstdint>

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
