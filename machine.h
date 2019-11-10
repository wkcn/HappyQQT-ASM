#pragma once
#include <iostream>
#include <functional>
#include <fstream>
#include <map>
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
    reg.IP = 0x7e00;
    ifstream fin(filename);
    string line;
    getline(fin, line);
    while (!fin.eof()) {
      read_deasm(line);
      getline(fin, line);
    }
  }
  void inject_qqt() {
    uint16_t base_addr = 0x7e00;
    mem.get(base_addr + 0x17d) = 0xd8;
    inject_functions[base_addr + 0x17d] = std::bind(&Machine::ReadFloppy, this);
  }
  void run() {
    while (1) {
      uint32_t addr = get_addr(reg.CS, reg.IP);
      uint8_t *p = &(mem.get(addr));
      cout << hex2str(addr - 0x7e00) << " OP:" << hex2str(*p) << endl;
      switch (*p) {
        case 0x24:
          ANDALIb(p+1);
          break;
        case 0x88:
          MOVEbGb(p+1);
          break;
        case 0x8c:
          MOVEwSw(p+1);
          break;
        case 0x8e:
          MOVSwEw(p+1);
          break;
        case 0xb0:
          MOVReg16Ib(p+1, reg.AL);
          break;
        case 0xb1:
          MOVReg16Ib(p+1, reg.CL);
          break;
        case 0xb2:
          MOVReg16Ib(p+1, reg.DL);
          break;
        case 0xb3:
          MOVReg16Ib(p+1, reg.BL);
          break;
        case 0xb4:
          MOVReg16Ib(p+1, reg.AH);
          break;
        case 0xb5:
          MOVReg16Ib(p+1, reg.CH);
          break;
        case 0xb6:
          MOVReg16Ib(p+1, reg.DH);
          break;
        case 0xb7:
          MOVReg16Ib(p+1, reg.BH);
          break;
        case 0xb8:
          MOVeIv(p+1, reg.AX);
          break;
        case 0xb9:
          MOVeIv(p+1, reg.CX);
          break;
        case 0xba:
          MOVeIv(p+1, reg.DX);
          break;
        case 0xbb:
          MOVeIv(p+1, reg.BX);
          break;
        case 0xbc:
          MOVeIv(p+1, reg.SP);
          break;
        case 0xbd:
          MOVeIv(p+1, reg.BP);
          break;
        case 0xbe:
          MOVeIv(p+1, reg.SI);
          break;
        case 0xbf:
          MOVeIv(p+1, reg.DI);
          break;
        case 0xc0:
          SHREbIb(p+1);
          break;
        case 0xcd:
          INT(p+1);
          break;
        case 0xee:
          OUT();
          break;
        default:
          LOG(FATAL) << "Unknown OpCode: " << hex2str(*p);
      }
      reg.IP += 1;
    }
  }
private:
  void ANDALIb(uint8_t *p) {
    // Acc, Imm
    uint8_t& lv = reg.AL;
    uint8_t& rv = *reinterpret_cast<uint8_t*>(p);
    cout << int(rv) << endl;
    lv &= rv;
    UpdateFlag(lv);
    reg.IP += 1;
  }
  void INT(uint8_t *p) {
    cout << "NotImplemented: INT 0x" << hex2str(*p) << endl;
    reg.IP += 1;
  }
  void MOVEbGb(uint8_t *p) {
    // Reg8, Reg8
    ModRM& modrm = read_oosssmmm(p);
    CHECK(modrm.MOD == 0b11);
    uint8_t &rv = GetReg8(modrm.REG);
    uint8_t &lv = GetReg8(modrm.RM);
    lv = rv;
    reg.IP += 1;
  }
  void MOVEwSw(uint8_t *p) {
    // 10001100oosssmmm
    // Reg16, Seg 
    ModRM& modrm = read_oosssmmm(p);
    CHECK(modrm.MOD == 0b11);
    uint16_t &rv = GetSeg16(modrm.REG);
    uint16_t &lv = GetReg16(modrm.RM);
    lv = rv;
    reg.IP += 1;
  }
  void MOVSwEw(uint8_t *p) {
    // Seg, Reg16
    ModRM& modrm = read_oosssmmm(p);
    CHECK(modrm.MOD == 0b11);
    uint16_t &lv = GetSeg16(modrm.REG);
    uint16_t &rv = GetReg16(modrm.RM);
    lv = rv;
    reg.IP += 1;
  }
  void MOVeIv(uint8_t *p, uint16_t &lv) {
    uint16_t& rv = *reinterpret_cast<uint16_t*>(p);
    lv = rv;
    reg.IP += 2;
  }
  void MOVReg16Ib(uint8_t *p, uint8_t &lv) {
    uint8_t& rv = *reinterpret_cast<uint8_t*>(p);
    lv = rv;
    reg.IP += 1;
  }
  void OUT() {
    cout << "NotImplemented: out dx, al" << endl; 
  }
  void RET() {
    reg.IP = mem.get<uint16_t>(reg.SS, reg.SP);
    reg.SP += 2;
  }
  void SHREbIb(uint8_t *p) {
    ModRM& modrm = read_oosssmmm(p);
    CHECK(modrm.MOD == 0b11);
    CHECK(modrm.REG == 0b101);
    uint8_t &lv = GetReg8(modrm.RM);
    uint8_t& rv = *reinterpret_cast<uint8_t*>(p + 1);
    lv >>= rv;
    UpdateFlag(lv);
    reg.IP += 2;
  }
private:
  inline ModRM& read_oosssmmm(uint8_t *p) {
    // MOD, REG, RM
    // oo,  sss, mmm
    return *reinterpret_cast<ModRM*>(p);
  }
private:
  uint16_t& GetSeg16(const uint8_t REG) {
    switch (REG) {
      case 0:
        return reg.ES;
      case 1:
        return reg.CS;
      case 2:
        return reg.SS;
      case 3:
        return reg.DS;
    };
    LOG(FATAL) << "Unknown Segment Register";
    return reg.CS;
  }
  uint8_t& GetReg8(const uint8_t REG) {
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
  uint16_t& GetReg16(const uint8_t RM) {
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
    LOG(FATAL) << "Unknown Register";
    return reg.AX;
  }
private:
  void read_deasm(const string& line) {
    // address | opcode | note
    uint32_t addr = get_addr(reg.CS, reg.IP);
    int i;
    for (i = 0; i < 8; ++i) {
      const char c = line[i];
      if (c == ' ') return;
      addr += hex2dec(c) * (1 << ((7 - i) * 4));
    }
    while (line[i] == ' ') ++i;
    while (line[i] != ' ') {
      uint8_t high = hex2dec(line[i++]);
      uint8_t low = hex2dec(line[i++]);
      mem.get<uint8_t>(addr++) = (high << 4) + low;
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
  template <typename T>
  void UpdateFlag(const T& v) {
    if (v == 0) reg.set_flag(ZF);
  }
private:
  void CallInjectFunc() {
    if (inject_functions.count(reg.IP)) {
      inject_functions[reg.IP]();
    }
  }
private:
  void ReadFloppy() {
    cout << "Read Floppy..." << endl;
    const uint16_t FileNameP = 0x7f77;   
    const uint16_t FileSegment = 0x7f7b;   
    const uint16_t FileOffset = 0x7f79; 
    uint16_t str_p = mem.get<uint16_t>(reg.CS, FileNameP);
    char* s = reinterpret_cast<char*>(&mem.get<uint16_t>(reg.CS, str_p));
    uint16_t seg = mem.get<uint16_t>(reg.CS, FileSegment);
    uint16_t offset = mem.get<uint16_t>(reg.CS, FileOffset);
    string filename;
    for (int i = 0; i < 8; ++i) {
      char c = s[i];
      if (c != ' ') filename += c - 'A' + 'a';
    }
    filename += ".res";
    cout << "Read " << filename << endl;
    filename = "./game/build/" + filename;
    ifstream fin(filename, ifstream::binary);
    CHECK(fin);
    fin.seekg(0, fin.end);
    int length = fin.tellg();
    fin.seekg(0, fin.beg);
    char* ptr = reinterpret_cast<char*>(&mem.get(seg, offset));
    fin.read(ptr, length);
    RET();
  }
private:
  uint16_t *pre_seg = nullptr;
private:
  Registers reg;
  Memory mem;
  const char hexch[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
  map<uint32_t, std::function<void()> > inject_functions;
};

