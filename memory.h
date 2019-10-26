#pragma once
#include <cstdint>


class Memory {
public:
  template <typename T=uint8_t>
  T& get(uint16_t ptr) {
    uint8_t* p = &memory[ptr];
    return *reinterpret_cast<T*>(p);
  }
private:
  uint8_t memory[0x10000];
};
