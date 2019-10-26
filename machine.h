#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdio>
#include <cstring>
#include <stack>
using namespace std;
#include "logging.h"
#include "register.h"
#include "instruction.h"
#include "memory.h"

template <typename T>
void PrintBits(T value, int n) {
  T t = T(1) << (n - 1);
  while (t > 0) {
    printf("%d", bool(value & t));
    t >>= 1;
  }
}

void PrintInstruction(Instruction ins) {
  cout << "OpCode: ";
  PrintBits(ins.opcode, 6); 
  cout << " D: ";
  PrintBits(ins.D, 1); 
  cout << " W: ";
  PrintBits(ins.W, 1); 
  cout << " MOD: ";
  PrintBits(ins.MOD, 2); 
  cout << " REG: ";
  PrintBits(ins.REG, 3); 
  cout << " RM: ";
  PrintBits(ins.RM, 3); 
  cout << endl;
}

class Machine {
public:
  Machine() {
    memset(&reg, 0, sizeof(reg));
    reg.FR = 512;
  }
  void load(const string& filename) {
    ifstream fin(filename);
    string line;
    getline(fin, line);
    while (!fin.eof()) {
      read_deasm(line);
      getline(fin, line);
    }
  }
  void run() {
    while (1) {
      uint16_t addr = (reg.CS << 4) + reg.IP;
      uint8_t *p = &(mem.get(addr));
      cout << hex2str(addr) << " OP:" << hex2str(*p) << endl;
      switch (*p) {
        case 0x88:
          MOVb(p+1, "Eb", "Gb");
          break;
        case 0x8C:
          MOVw(p+1, "Ew", "Sw");
          break;
        case 0x8E:
          MOVw(p+1, "Sw", "Ew");
          break;
        case 0xB0:
          MOVib(p+1, "AL", "Ib");
          break;
        case 0xB1:
          MOVib(p+1, "CL", "Ib");
          break;
        case 0xB2:
          MOVib(p+1, "DL", "Ib");
          break;
        case 0xB3:
          MOVib(p+1, "BL", "Ib");
          break;
        case 0xB4:
          MOVib(p+1, "AH", "Ib");
          break;
        case 0xB5:
          MOVib(p+1, "CH", "Ib");
          break;
        case 0xB6:
          MOVib(p+1, "DH", "Ib");
          break;
        case 0xB7:
          MOVib(p+1, "BH", "Ib");
          break;
        case 0xB8:
          MOViw(p+1, "eAX", "Iv");
          break;
        case 0xB9:
          MOViw(p+1, "eCX", "Iv");
          break;
        case 0xBA:
          MOViw(p+1, "eDX", "Iv");
          break;
        case 0xBB:
          MOViw(p+1, "eBX", "Iv");
          break;
        case 0xBC:
          MOViw(p+1, "eSP", "Iv");
          break;
        case 0xC0:
          SHRib(p+1, "Eb", "Ib");
          break;
        case 0xCD:
          INT(p+1);
          break;
        case 0xEE:
          OUT("DX", "AL");
          break;
        default:
          LOG(FATAL) << "Unknown OpCode: " << hex2str(*p);
      }
    }
  }
private:
  void read_deasm(const string& line) {
    // address | opcode | note
    uint32_t address = 0;
    int i;
    for (i = 0; i < 8; ++i) {
      const char c = line[i];
      if (c == ' ') return;
      address += hex2dec(c) * (1 << ((7 - i) * 4));
    }
    while (line[i] == ' ') ++i;
    while (line[i] != ' ') {
      uint8_t high = hex2dec(line[i++]);
      uint8_t low = hex2dec(line[i++]);
      mem.get<uint8_t>(address++) = (high << 4) + low;
    }
  }
  uint8_t hex2dec(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'Z') return c - 'A' + 10;
    if (c >= 'a' && c <= 'z') return c - 'a' + 10;
    LOG(FATAL) << "Invalid Hex: " << hex2str(c);
    return -1;
  }
  template<typename T>
  string hex2str(const T& c) {
    string res;
    int len = sizeof(T);
    const uint8_t* p = reinterpret_cast<const uint8_t*>(&c);
    for (int i = len - 1; i >= 0; --i) {
      uint8_t num = *(p + i); 
      res += hexch[(num>>4)&0xF]; 
      res += hexch[num&0xF]; 
    }
    return res;
  }
private:
  void INT(uint8_t *p) {
    cout << "NotImplemented: INT 0x" << hex2str(*p) << endl;
    reg.IP += 2;
  }
  void OUT(const char *a, const char *b) {
    cout << "NotImplemented: OUT: " << a << ", " << b << endl;
    reg.IP += 1;
  }
  void MOVib(uint8_t *p, const char* mc1, const char* mc2) {
    uint8_t& rv = *reinterpret_cast<uint8_t*>(p + 1);
    uint8_t& lv = get_register_b_by_name(mc1);
    lv = rv;
    reg.IP += 2;
  }
  void MOViw(uint8_t *p, const char* mc1, const char* mc2) {
    uint16_t& rv = *reinterpret_cast<uint16_t*>(p + 1);
    uint16_t& lv = get_register_w_by_name(mc1);
    lv = rv;
    reg.IP += 3;
  }
  void MOVb(uint8_t *p, const char* mc1, const char* mc2) {
    const ModRM& modrm = *reinterpret_cast<ModRM*>(p);
    CHECK(mc1[0] == 'E');
    CHECK(mc2[0] == 'G');
    uint8_t& rv = get_register_b(modrm.REG);
    uint8_t& lv = get_register_b(modrm.RM);
    lv = rv;
    reg.IP += 2;
  }
  void MOVw(uint8_t *p, const char* mc1, const char* mc2) {
    const ModRM& modrm = *reinterpret_cast<ModRM*>(p);
    uint16_t& rv = get_register_w(modrm.REG, mc2[0]);
    uint16_t& lv = get_modrm_w(modrm.MOD, modrm.RM);
    lv = rv;
    reg.IP += 2;
  }
  void SHRib(uint8_t *p, const char* mc1, const char* mc2) {
    CHECK_EQ((*p) & 0xF8, 0xE8);
    CHECK(mc1[0] == 'E');
    CHECK(mc2[0] == 'I');
    uint8_t& lv = get_register_b((*p)&0x7);
    uint8_t& rv = *reinterpret_cast<uint8_t*>(p + 1);
    lv >>= rv;
    reg.IP += 3;
  }
private:
  uint8_t& get_register_b(const uint8_t REG) {
    switch (REG) {
      case 0:
        return reg.AL;
      case 1:
        return reg.CL;
      case 2:
        return reg.DL;
      case 3:
        return reg.BL;
      case 4:
        return reg.AH;
      case 5:
        return reg.CH;
      case 6:
        return reg.DH;
      case 7:
        return reg.BH;
    }
    LOG(INFO) << "Unknown Register";
    return reg.AL;
  }
  uint16_t& get_register_w(const uint8_t REG, const char c) {
    switch (c) {
      case 'S':
        switch (REG) {
          case 0:
            return reg.DS;
          case 1:
            return reg.ES;
          case 4:
            return reg.SS;
          case 5:
            return reg.CS;
          case 6:
            return reg.IP;
        }
        break;
      case 'E': 
        switch (REG) {
          case 0:
            return reg.AX;
          case 1:
            return reg.CX;
          case 2:
            return reg.DX;
          case 3:
            return reg.BX;
          case 4:
            return reg.SP;
          case 5:
            return reg.BP;
          case 6:
            return reg.SI;
          case 7:
            return reg.DI;
        }
        break;
    }
    LOG(INFO) << "Unknown Register";
    return reg.AX;
  }
  uint16_t& get_modrm_w(const uint8_t MOD, const uint8_t RM) {
    switch (MOD) {
      case 3:
        switch (RM) {
          case 0:
            return reg.AX;
          case 1:
            return reg.CX;
          case 2:
            return reg.DX;
          case 3:
            return reg.BX;
          case 4:
            return reg.SP;
          case 5:
            return reg.BP;
          case 6:
            return reg.SI;
          case 7:
            return reg.DI;
        }
        break;
    }
    LOG(INFO) << "Unknown MoDRM";
    return reg.AX;
  }
  uint8_t& get_register_b_by_name(const char* name) {
    switch (name[0]) {
      case 'A':
        return name[1] == 'L' ? reg.AL : reg.AH;
      case 'C':
        return name[1] == 'L' ? reg.CL : reg.CH;
      case 'D':
        return name[1] == 'L' ? reg.DL : reg.DH;
      case 'B':
        return name[1] == 'L' ? reg.BL : reg.BH;
    }
    LOG(FATAL) << "FAIL";
    return reg.AL;
  }
  uint16_t& get_register_w_by_name(const char* name) {
    switch (name[1]) {
      case 'A':
        return reg.AX;
      case 'C':
        return reg.CX;
      case 'D':
        return name[2] == 'X' ? reg.DX : reg.DI;
      case 'B':
        return name[2] == 'X' ? reg.BX : reg.BP;
      case 'S':
        return name[2] == 'I' ? reg.SI : reg.SP;
    }
    LOG(FATAL) << "FAIL";
    return reg.AX;
  }
private:
  Registers reg;
  Memory mem;
  const char hexch[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
};

