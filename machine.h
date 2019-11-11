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
#include "gui.h"

template <typename T>
struct DummyIdentity {
  typedef T type;
};

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
      // cout << hex2str(addr - 0x7e00) << " OP:" << hex2str(*p) << endl;
      switch (*p) {
        case 0x00:
          ADDEbGb(p+1);
          break;
        case 0x01:
          ADDEvGv(p+1);
          break;
        case 0x02:
          ADDGbEb(p+1);
          break;
        case 0x03:
          ADDGvEv(p+1);
          break;
        case 0x06:
          PUSHw(reg.ES);
          break;
        case 0x07:
          POPw(reg.ES);
          break;
        case 0x0e:
          PUSHw(reg.CS);
          break;
        case 0x16:
          PUSHw(reg.SS);
          break;
        case 0x17:
          POPw(reg.SS);
          break;
        case 0x1e:
          PUSHw(reg.DS);
          break;
        case 0x1f:
          POPw(reg.DS);
          break;
        case 0x24:
          ANDALIb(p+1);
          break;
        case 0x28:
          SUBEbGb(p+1);
          break;
        case 0x29:
          SUBEvGv(p+1);
          break;
        case 0x2a:
          SUBGbEb(p+1);
          break;
        case 0x2b:
          SUBGvEv(p+1);
          break;
        case 0x26:
          pre_seg = &reg.ES;
          break;
        case 0x2E:
          pre_seg = &reg.CS;
          break;
        case 0x3E:
          pre_seg = &reg.DS;
          break;
        case 0x40:
          INC(reg.AX);
          break;
        case 0x41:
          INC(reg.CX);
          break;
        case 0x42:
          INC(reg.DX);
          break;
        case 0x43:
          INC(reg.BX);
          break;
        case 0x44:
          INC(reg.SP);
          break;
        case 0x45:
          INC(reg.BP);
          break;
        case 0x46:
          INC(reg.SI);
          break;
        case 0x47:
          INC(reg.DI);
          break;
        case 0x48:
          DEC(reg.AX);
          break;
        case 0x49:
          DEC(reg.CX);
          break;
        case 0x4a:
          DEC(reg.DX);
          break;
        case 0x4b:
          DEC(reg.BX);
          break;
        case 0x4c:
          DEC(reg.SP);
          break;
        case 0x4d:
          DEC(reg.BP);
          break;
        case 0x4e:
          DEC(reg.SI);
          break;
        case 0x4f:
          DEC(reg.DI);
          break;
        case 0x50:
          PUSHw(reg.AX);
          break;
        case 0x51:
          PUSHw(reg.CX);
          break;
        case 0x52:
          PUSHw(reg.DX);
          break;
        case 0x53:
          PUSHw(reg.BX);
          break;
        case 0x54:
          PUSHw(reg.SP);
          break;
        case 0x55:
          PUSHw(reg.BP);
          break;
        case 0x56:
          PUSHw(reg.SI);
          break;
        case 0x57:
          PUSHw(reg.DI);
          break;
        case 0x58:
          POPw(reg.AX);
          break;
        case 0x59:
          POPw(reg.CX);
          break;
        case 0x5A:
          POPw(reg.DX);
          break;
        case 0x5B:
          POPw(reg.BX);
          break;
        case 0x5C:
          POPw(reg.SP);
          break;
        case 0x5D:
          POPw(reg.BP);
          break;
        case 0x5E:
          POPw(reg.SI);
          break;
        case 0x5F:
          POPw(reg.DI);
          break;
        case 0x60:
          PUSHA();
          break;
        case 0x61:
          POPA();
          break;
        case 0x74:
          JZ(p+1);
          break;
        case 0x75:
          JNZ(p+1);
          break;
        case 0x80:
          CMP(p+1);
          break;
        case 0x81:
          ADDEvIv(p+1);
          break;
        case 0x88:
          MOVEbGb(p+1);
          break;
        case 0x89:
          MOVEvGv(p+1);
          break;
        case 0x8a:
          MOVGbEb(p+1);
          break;
        case 0x8b:
          MOVGvEv(p+1);
          break;
        case 0x8c:
          MOVEwSw(p+1);
          break;
        case 0x8e:
          MOVSwEw(p+1);
          break;
        case 0xa1:
          MOVeAXOv(p+1);
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
          SHEbIb(p+1);
          break;
        case 0xc3:
          RET();
          break;
        case 0xc7:
          MOVEvIv(p+1);
          break;
        case 0xcd:
          INT(p+1);
          break;
        case 0xd8:
          CallInjectFunc();
          break;
        case 0xd0:
          SHEb1(p+1);
          break;
        case 0xe2:
          LOOP(p+1);
          break;
        case 0xe8:
          CALL(p+1);
          break;
        case 0xe6:
          OUTIbAL(p+1);
          break;
        case 0xee:
          OUTDXAL();
          break;
        case 0xf7:
          MULEv(p+1);
          break;
        case 0xfb:
          STI();
          break;
        default:
          LOG(FATAL) << "Unknown OpCode: [" << hex2str(addr - 0x7e00) << "]" << hex2str(*p);
      }
      reg.IP += 1;
    }
  }
private:
  void ADDEbGb(uint8_t *p) {
    uint8_t *eb, *gb;
    _GetEvGv(p, eb, gb);
    *eb += *gb;
    // TODO Update Flag
  }
  void ADDEvGv(uint8_t *p) {
    uint16_t *ev, *gv;
    _GetEvGv(p, ev, gv);
    *ev += *gv;
    // TODO Update Flag
  }
  void ADDGbEb(uint8_t *p) {
    uint8_t *eb, *gb;
    _GetEvGv(p, eb, gb);
    *gb += *eb;
    // TODO Update Flag
  }
  void ADDGvEv(uint8_t *p) {
    uint16_t *ev, *gv;
    _GetEvGv(p, ev, gv);
    *gv += *ev;
    // TODO Update Flag
  }
  void ANDALIb(uint8_t *p) {
    // Acc, Imm
    uint8_t& lv = reg.AL;
    uint8_t& rv = *reinterpret_cast<uint8_t*>(p);
    lv &= rv;
    UpdateFlag(lv);
    reg.IP += 1;
  }
  void ADDEvIv(uint8_t *p) {
    // Mem, Imm
    ModRM& modrm = read_oosssmmm(p);
    CHECK(modrm.MOD == 0b11);
    CHECK(modrm.REG == 0b000);
    uint16_t &lv = GetReg16(modrm.RM);
    uint16_t &rv = *reinterpret_cast<uint16_t*>(p+1);
    lv += rv;
    reg.IP += 3;
    // TODO Update Flag
  }
  void CALL(uint8_t *p) {
    reg.SP -= 2;
    reg.IP += 2;
    mem.get<uint16_t>(reg.SS, reg.SP) = reg.IP;
    reg.IP += *reinterpret_cast<uint16_t*>(p);
  }
  template<typename T, typename uT>
  uT _SUB(uT dest, uT source) {
    uT res = dest - source;
    reg.set_flag(Flag::ZF, res == 0);
    reg.set_flag(Flag::CF, dest < source);
    bool pos_res = static_cast<T>(res) >= 0;
    bool pos_dest = static_cast<T>(dest) >= 0;
    bool pos_source = static_cast<T>(source) >= 0;
    bool overflow = res != 0 && (pos_dest == pos_source) && pos_res != pos_dest;
    reg.set_flag(Flag::OF, overflow);
    reg.set_flag(Flag::SF, !pos_res);
    return res;
  }
  inline uint8_t _SUB8(uint8_t dest, uint8_t source) {
    return _SUB<int8_t, uint8_t>(dest, source);
  }
  void CMP(uint8_t *p) {
    ModRM& modrm = read_oosssmmm(p);
    CHECK(modrm.MOD == 0b11);
    CHECK(modrm.REG == 0b111);
    // Reg, Imm
    uint8_t &lv = GetReg8(modrm.RM);
    uint8_t &rv = *reinterpret_cast<uint8_t*>(p+1);
    _SUB8(lv, rv);
    reg.IP += 2;
  }
  void DEC(uint16_t &lv) {
    --lv;
    // TODO Update Flag
  }
  void INC(uint16_t &lv) {
    ++lv;
    // TODO Update Flag
  }
  void INT(uint8_t *p) {
    cout << "NotImplemented: INT 0x" << hex2str(*p) << endl;
    reg.IP += 1;
  }
  void _IPJump(uint8_t step) {
    reg.IP += step;
    if (step & (1 << 7)) {
      reg.IP -= 0x100;
    }
  }
  void JZ(uint8_t *p) {
    if (reg.get_flag(Flag::ZF))
      _IPJump(*reinterpret_cast<uint8_t*>(p));
    reg.IP += 1;
  }
  void JNZ(uint8_t *p) {
    if (!reg.get_flag(Flag::ZF))
      _IPJump(*reinterpret_cast<uint8_t*>(p));
    reg.IP += 1;
  }
  void LOOP(uint8_t *p) {
    if (--reg.CX != 0) {
      _IPJump(*p);
    }
    reg.IP += 1;
  }
  template <typename uT>
  void _GetEvGv(uint8_t *p, uT *&ev, uT *&gv) {
    ModRM& modrm = read_oosssmmm(p);
    gv = &GetReg<uT>(modrm.REG);
    if (modrm.MOD == 0b11) {
      ev = &GetReg<uT>(modrm.RM);
      // Reg, Reg
      reg.IP += 1;
    } else { 
      // Mem, Reg
      if (modrm.MOD == 0b00) {
        CHECK(modrm.MOD == 0b00);
        if (modrm.RM == 0b110) {
          uT& offset = *reinterpret_cast<uT*>(p+1);
          CHECK(pre_seg);
          uint32_t addr = get_addr(*pre_seg, offset);
          pre_seg = nullptr;
          ev = &mem.get<uT>(addr);
          reg.IP += 3;
        } else {
          uint32_t addr;
          if (modrm.RM == 0b100) {
            // DS: [SI]
            // 0134
            if (!pre_seg) pre_seg = &reg.DS;
            addr = get_addr(*pre_seg, reg.SI);
          } else {
            CHECK(modrm.RM == 0b101);
            if (!pre_seg) pre_seg = &reg.DS;
            addr = get_addr(*pre_seg, reg.DI);
            // DS: [DI]
          }
          pre_seg = nullptr;
          ev = &mem.get<uT>(addr);
          reg.IP += 1;
        }
      } else {
        CHECK(modrm.MOD == 0b01);
        CHECK(modrm.RM == 0b100); // DS:[SI]
        uint8_t& offset = *reinterpret_cast<uint8_t*>(p+1);
        if (!pre_seg) pre_seg = &reg.DS;
        uint32_t addr = get_addr(*pre_seg, reg.SI + offset);
        pre_seg = nullptr;
        ev = &mem.get<uT>(addr);
        reg.IP += 2;
      }
    }
  }
  void MOVEbGb(uint8_t *p) {
    uint8_t *eb, *gb;
    _GetEvGv(p, eb, gb);
    *eb = *gb;
  }
  void MOVEvGv(uint8_t *p) {
    uint16_t *ev, *gv;
    _GetEvGv(p, ev, gv);
    *ev = *gv;
  }
  void MOVGbEb(uint8_t *p) {
    uint8_t *eb, *gb;
    _GetEvGv(p, eb, gb);
    *gb = *eb;
  }
  void MOVGvEv(uint8_t *p) {
    uint16_t *ev, *gv;
    _GetEvGv(p, ev, gv);
    *gv = *ev;
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
  void MOVeAXOv(uint8_t *p) {
    // Acc, MemOfs
    uint16_t& offset = *reinterpret_cast<uint16_t*>(p);
    CHECK(pre_seg);
    uint32_t addr = get_addr(*pre_seg, offset);
    uint16_t &rv = mem.get<uint16_t>(addr);
    uint16_t &lv = reg.AX;
    pre_seg = nullptr;
    lv = rv;
    reg.IP += 2;
  }
  void MOVeIv(uint8_t *p, uint16_t &lv) {
    uint16_t& rv = *reinterpret_cast<uint16_t*>(p);
    lv = rv;
    reg.IP += 2;
  }
  void MOVEvIv(uint8_t *p) {
    // Mem, Imm
    ModRM& modrm = read_oosssmmm(p);
    CHECK(modrm.MOD == 0b00);
    CHECK(modrm.REG == 0b000);
    CHECK(modrm.RM == 0b110);
    CHECK(pre_seg);
    uint16_t& offset = *reinterpret_cast<uint16_t*>(p+1);
    uint16_t& value = *reinterpret_cast<uint16_t*>(p+3);
    uint32_t addr = get_addr(*pre_seg, offset);
    pre_seg = nullptr;
    mem.get<uint16_t>(addr) = value;
    reg.IP += 5;
  }
  void MOVReg16Ib(uint8_t *p, uint8_t &lv) {
    uint8_t& rv = *reinterpret_cast<uint8_t*>(p);
    lv = rv;
    reg.IP += 1;
  }
  void MULEv(uint8_t *p) {
    ModRM& modrm = read_oosssmmm(p);
    CHECK(modrm.MOD == 0b11);
    CHECK(modrm.REG == 0b100);
    uint16_t &rv = GetReg16(modrm.RM);
    uint32_t rv32 = (uint32_t)rv;
    uint32_t lv32 = (uint32_t)reg.AX;
    lv32 *= rv32;
    reg.AX = lv32 & 0xFFFF;
    reg.DX = (lv32 >> 16) & 0xFFFF;
    // TODO UpdateFlag
    reg.IP += 1;
  }
  void OUTDXAL() {
    switch (reg.DX) {
      case 0x3c6:
        // set palette mask
        break;
      case 0x3c8:
        // set palette index
        gui.set_palette_index(reg.AL);
        break;
      case 0x3c9:
        // set palette value 
        gui.set_palette_value(reg.AL);
        break;
      default:
        cout << "NotImplemented: out dx, al - DX: " << hex2str(reg.DX) << " AL: " << hex2str(reg.AL) << endl; 
    };
  }
  void OUTIbAL(uint8_t *p) {
    uint8_t &lv = *reinterpret_cast<uint8_t*>(p);
    cout << "NotImplemented: out Ib, al - Ib: " << hex2str(lv) << " AL: " << hex2str(reg.AL) << endl; 
    reg.IP += 1;
  }
  void POPw(uint16_t &lv) {
    lv = mem.get<uint16_t>(reg.SS, reg.SP);
    reg.SP += 2;
  }
  void PUSHw(uint16_t &rv) {
    reg.SP -= 2;
    mem.get<uint16_t>(reg.SS, reg.SP) = rv;
  }
  void PUSHA() {
    uint16_t old_SP = reg.SP;
    PUSHw(reg.AX);
    PUSHw(reg.CX);
    PUSHw(reg.DX);
    PUSHw(reg.BX);
    PUSHw(old_SP);
    PUSHw(reg.BP);
    PUSHw(reg.SI);
    PUSHw(reg.DI);
  }
  void POPA() {
    uint16_t old_SP;
    POPw(reg.DI);
    POPw(reg.SI);
    POPw(reg.BP);
    POPw(old_SP);
    POPw(reg.BX);
    POPw(reg.DX);
    POPw(reg.CX);
    POPw(reg.AX);
    reg.SP = old_SP;
  }
  void RET() {
    reg.IP = mem.get<uint16_t>(reg.SS, reg.SP);
    reg.SP += 2;
  }
  void SHEb1(uint8_t *p) {
    ModRM& modrm = read_oosssmmm(p);
    CHECK(modrm.MOD == 0b11);
    CHECK(modrm.REG == 0b100);
    uint8_t &lv = GetReg8(modrm.RM);
    if (modrm.REG == 0b101) {
      lv >>= 1;
    } else {
      CHECK(modrm.REG == 0b100);
      lv <<= 1;
    }
    UpdateFlag(lv);
    reg.IP += 1;
  }
  void SHEbIb(uint8_t *p) {
    ModRM& modrm = read_oosssmmm(p);
    CHECK(modrm.MOD == 0b11);
    uint8_t &lv = GetReg8(modrm.RM);
    uint8_t &rv = *reinterpret_cast<uint8_t*>(p + 1);
    if (modrm.REG == 0b101) {
      lv >>= rv;
    } else {
      CHECK(modrm.REG == 0b100);
      lv <<= rv;
    }
    UpdateFlag(lv);
    reg.IP += 2;
  }
  void STI() {
    cout << "NotImplemented: STI" << endl; 
  }
  void SUBEbGb(uint8_t *p) {
    uint8_t *eb, *gb;
    _GetEvGv(p, eb, gb);
    *eb -= *gb;
    // TODO Update Flag
  }
  void SUBEvGv(uint8_t *p) {
    uint16_t *ev, *gv;
    _GetEvGv(p, ev, gv);
    *ev -= *gv;
    // TODO Update Flag
  }
  void SUBGbEb(uint8_t *p) {
    uint8_t *eb, *gb;
    _GetEvGv(p, eb, gb);
    *gb -= *eb;
    // TODO Update Flag
  }
  void SUBGvEv(uint8_t *p) {
    uint16_t *ev, *gv;
    _GetEvGv(p, ev, gv);
    *gv -= *ev;
    // TODO Update Flag
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
  template <typename T> inline T& _GetReg(const uint8_t, DummyIdentity<T>);
  inline uint8_t& _GetReg(const uint8_t REG, DummyIdentity<uint8_t>) {
    return GetReg8(REG);
  }
  inline uint16_t& _GetReg(const uint8_t REG, DummyIdentity<uint16_t>) {
    return GetReg16(REG);
  }
  template <typename T> inline T& GetReg(const uint8_t REG) {
    return _GetReg(REG, DummyIdentity<T>());
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
  GUI gui;
  const char hexch[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
  map<uint32_t, std::function<void()> > inject_functions;
};

