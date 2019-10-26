#pragma once
#include <cstdint>

union Instruction{
  struct {
    uint8_t RM : 3;
    uint8_t REG : 3;
    uint8_t MOD : 2;
    uint8_t W : 1;
    uint8_t D : 1;
    uint8_t opcode : 6;
  };
  uint16_t value;
};

union ModRM {
  struct {
    uint8_t RM : 3;
    uint8_t REG : 3;
    uint8_t MOD : 2;
  };
  uint8_t value;
};
