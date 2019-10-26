#pragma once
#include <cstdint>

inline uint32_t get_addr(uint16_t seg, uint16_t offset) {
  return (uint32_t(seg) << 4) + uint32_t(offset);
}

class Memory {
public:
  template <typename T=uint8_t>
  T& get(uint32_t ptr) {
    uint8_t* p = &memory[ptr];
    return *reinterpret_cast<T*>(p);
  }
  template <typename T=uint8_t>
  T& get(uint16_t seg, uint16_t offset) {
    return get<T>(get_addr(seg, offset));
  }
private:
  uint8_t memory[0x100000];
};
