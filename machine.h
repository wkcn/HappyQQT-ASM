#pragma once
#include <chrono>
#include <iostream>
#include <functional>
#include <fstream>
#include <map>
#include <queue>
#include <string>
#include <vector>
#include <stack>
#include <cstdio>
#include <cstring>
#include <ctime>
using namespace std;
#include "logging.h"
#include "register.h"
#include "instruction.h"
#include "memory.h"
#include "gui.h"

const int UpdateTimes = 60;
using time_point = std::chrono::steady_clock::time_point;
using ms_type = std::chrono::duration<int, ratio<1, 1000> >;
using milliseconds = std::chrono::milliseconds;

inline time_point get_time_now() {
  return std::chrono::steady_clock::now();
}

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

#define INJECT_FUNC_CODE 0xd8

class Machine {
public:
  Machine() {
    memset(&reg, 0, sizeof(reg));
    reg.FR = 512;
    gui.set_video_addr(&(mem.get(0xa000, 0)));
    InitIVT();
    last_int08h_time = get_time_now();
    use_prefix = false;
  }
  void InitIVT() {
    for (int i = 0; i < 0x100; ++i) {
      mem.get<uint16_t>(i * 4) = i;
      mem.get<uint16_t>(i * 4 + 2) = 0xFFFF;
    }
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
    mem.get(base_addr + 0x17d) = INJECT_FUNC_CODE;
    inject_functions[base_addr + 0x17d] = std::bind(&Machine::ReadFloppy, this);
  }
  void run() {
    while (1) {
      if (rep_mode) {
        if (--reg.CX == 0) {
          rep_mode = false;
        } else if (reg.IP == rep_next_ip && reg.CS == rep_next_cs) {
          reg.CS = rep_CS;
          reg.IP = rep_IP;
        }
      }

      if (!use_prefix) {
        time_point cur_time = get_time_now();
        int diff_ms = std::chrono::duration_cast<milliseconds>(cur_time - last_int08h_time).count();
        if (diff_ms >= 1000 / UpdateTimes) {
          CALL_INT(0x08);
          last_int08h_time = cur_time;
        }
      }

      use_prefix = false;

      uint32_t addr = get_addr(reg.CS, reg.IP);
      uint8_t *p = &(mem.get(addr));
#if 0
      cout << hex2str(addr - 0x7e00) << " OP:" << hex2str(*p) << endl <<
        " AX:" << hex2str(reg.AX) <<
        " BX:" << hex2str(reg.BX) <<
        " CX:" << hex2str(reg.CX) <<
        " DX:" << hex2str(reg.DX) <<
        " SP:" << hex2str(reg.SP) <<
        " BP:" << hex2str(reg.BP) <<
        " SI:" << hex2str(reg.SI) <<
        " DI:" << hex2str(reg.DI) <<
        endl <<
        " CS:" << hex2str(reg.CS) <<
        " DS:" << hex2str(reg.DS) <<
        " SS:" << hex2str(reg.SS) <<
        " ES:" << hex2str(reg.ES) <<
        endl;
#endif
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
        case 0x0f:
          JZTWOBYTE(p+1);
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
          SetSegPrefix(reg.ES);
          break;
        case 0x2E:
          SetSegPrefix(reg.CS);
          break;
        case 0x3E:
          SetSegPrefix(reg.DS);
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
        case 0x66:
          Set32bPrefix();
          break;
        case 0x6a:
          PUSHIb(p+1);
          break;
        case 0x74:
          JZ(p+1);
          break;
        case 0x75:
          JNZ(p+1);
          break;
        case 0x80:
          CMPEbIb(p+1);
          break;
        case 0x81:
          ADDEvIv(p+1);
          break;
        case 0x83:
          ADDEvIb(p+1);
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
        case 0xa5:
          MOVSWXvYv(p+1);
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
        case 0xc6:
          MOVEbIb(p+1);
          break;
        case 0xc7:
          MOVEvIv(p+1);
          break;
        case 0xcd:
          INT(p+1);
          break;
        case INJECT_FUNC_CODE:
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
        case 0xf3:
          REP();
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

      if (!use_prefix) {
        if (rep_mode) {
          if (rep_next_cs == 0xFFFF) {
            rep_next_ip = reg.IP;
            rep_next_cs = reg.CS;
          }
        }
      }

    }
  }
private:
  void ADDEbGb(uint8_t *p) {
    uint8_t *eb, *gb;
    _GetEvGv(p, eb, gb);
    *eb = _ADD<int8_t, uint8_t>(*eb, *gb);
  }
  void ADDEvGv(uint8_t *p) {
    uint16_t *ev, *gv;
    _GetEvGv(p, ev, gv);
    *ev = _ADD<int16_t, uint16_t>(*ev, *gv);
  }
  void ADDGbEb(uint8_t *p) {
    uint8_t *eb, *gb;
    _GetEvGv(p, eb, gb);
    *gb = _ADD<int8_t, uint8_t>(*gb, *eb);
  }
  void ADDGvEv(uint8_t *p) {
    uint16_t *ev, *gv;
    _GetEvGv(p, ev, gv);
    *gv = _ADD<int16_t, uint16_t>(*gv, *ev);
  }
  void ANDALIb(uint8_t *p) {
    // Acc, Imm
    uint8_t& lv = reg.AL;
    uint8_t& rv = *reinterpret_cast<uint8_t*>(p);
    lv &= rv;
    reg.IP += 1;
    reg.set_flag(Flag::ZF, lv == 0);
  }
  void ADDEvIv(uint8_t *p) {
    // Mem, Imm
    uint16_t *ev;
    _GetEv(p, ev);
    uint16_t &rv = *reinterpret_cast<uint16_t*>(p+1);
    *ev = _ADD<int16_t, uint16_t>(*ev, rv);
    reg.IP += 2;
  }
  void ADDEvIb(uint8_t *p) {
    uint16_t *ev;
    _GetEv(p, ev);
    uint16_t rv = *reinterpret_cast<uint8_t*>(p+1);
    *ev = _ADD<int16_t, uint16_t>(*ev, rv);
    reg.IP += 1;
  }
  void CALL(uint8_t *p) {
    reg.IP += 2; // since offset occupy 2 bytes
    PUSHw(reg.IP);
    reg.IP += *reinterpret_cast<uint16_t*>(p);
  }
  void CALLF(uint16_t seg, uint16_t offset) {
    PUSHw(reg.CS);
    PUSHw(reg.IP);
    reg.CS = seg;
    reg.IP = offset;
  }
  void CALL_INT(uint16_t id) {
    if (!reg.get_flag(Flag::IF)) return;
    uint16_t &offset = mem.get<uint16_t>(id * 4);
    uint16_t &seg = mem.get<uint16_t>(id * 4 + 2);
    PUSHF();
    if (seg == 0xFFFF) {
      CallDefaultINT(offset);
    } else {
      CALLF(seg, offset);
    }
  }
  void CallDefaultINT(uint16_t id) {
    PUSHw(reg.CS);
    PUSHw(reg.IP);
    switch (id) {
      case 0x10:
        ScreenINT();
        break;
      case 0x16:
        KeyBoardINT();
        break;
      case 0x1a:
        ClockINT();
        break;
      default:
        LOG(FATAL) << "NotImplemented INT: " << hex2str(id);
    };
    IRET();
  }
  inline uint8_t _SUB8(uint8_t dest, uint8_t source) {
    return _SUB<int8_t, uint8_t>(dest, source);
  }
  void CMPEbIb(uint8_t *p) {
    uint8_t *ev;
    _GetEv(p, ev);
    uint8_t &rv = *reinterpret_cast<uint8_t*>(p+1);
    _SUB8(*ev, rv);
    reg.IP += 1;
  }
  void DEC(uint16_t &lv) {
    lv = _SUB<int16_t, uint16_t>(lv, 1);
  }
  void INC(uint16_t &lv) {
    lv = _ADD<int16_t, uint16_t>(lv, 1);
  }
  void INT(uint8_t *p) {
    uint8_t id = *p;
    reg.IP += 1;
    CALL_INT(id);
  }
  void _IPStep(uint8_t step) {
    reg.IP += step;
    if (step & (1 << 7)) {
      reg.IP -= 0x100;
    }
  }
  void _IPStep(uint16_t step) {
    reg.IP += step;
  }
  void JZ(uint8_t *p) {
    if (reg.get_flag(Flag::ZF))
      _IPStep(*reinterpret_cast<uint8_t*>(p));
    reg.IP += 1;
  }
  void JZTWOBYTE(uint8_t *p) {
    CHECK(*p == 0x84);
    if (reg.get_flag(Flag::ZF))
      _IPStep(*reinterpret_cast<uint16_t*>(p+1));
    reg.IP += 3;
  }
  void JNZ(uint8_t *p) {
    if (!reg.get_flag(Flag::ZF))
      _IPStep(*reinterpret_cast<uint8_t*>(p));
    reg.IP += 1;
  }
  void LOOP(uint8_t *p) {
    if (--reg.CX != 0) {
      _IPStep(*p);
    }
    reg.IP += 1;
  }
  template <typename uT>
  void _GetGv_nostep(uint8_t *p, uT *&gv) {
    ModRM& modrm = read_oosssmmm(p);
    gv = &GetReg<uT>(modrm.REG);
  }
  template <typename uT>
  void _GetEv(uint8_t *p, uT *&ev) {
    ModRM &modrm = read_oosssmmm(p);
    if (modrm.MOD == 0b11) {
      ev = &GetReg<uT>(modrm.RM);
      // Reg, Reg
      reg.IP += 1;
    } else { 
      // Mem, Reg
      if (modrm.MOD == 0b00) {
        if (modrm.RM == 0b110) {
          uT& offset = *reinterpret_cast<uT*>(p+1); CHECK(pre_seg);
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
  template <typename uT>
  void _GetEvGv(uint8_t *p, uT *&ev, uT *&gv) {
    _GetGv_nostep(p, gv);
    _GetEv(p, ev);
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
    if (!pre_seg) pre_seg = &reg.DS;
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
  void MOVEbIb(uint8_t *p) {
    uint8_t *ev;
    _GetEv(p, ev);
    uint8_t &iv = *reinterpret_cast<uint8_t*>(p+2);
    *ev = iv;
    reg.IP += 1;
  }
  void MOVEvIv(uint8_t *p) {
    // Mem, Imm
    uint16_t *ev;
    _GetEv(p, ev);
    uint16_t &iv = *reinterpret_cast<uint16_t*>(p+3);
    *ev = iv;
    reg.IP += 2;
  }
  void MOVReg16Ib(uint8_t *p, uint8_t &lv) {
    uint8_t& rv = *reinterpret_cast<uint8_t*>(p);
    lv = rv;
    reg.IP += 1;
  }
  void MOVSWXvYv(uint8_t *p) {
   // [ds:si] -> [es:di]
   mem.get<uint16_t>(reg.ES, reg.DI) = mem.get<uint16_t>(reg.DS, reg.SI);
   if (reg.get_flag(Flag::DF)) {
     reg.DI -= 2;
     reg.SI -= 2;
   } else {
     reg.DI += 2;
     reg.SI += 2;
   }
  }
  void MULEv(uint8_t *p) {
    uint16_t *ev;
    _GetEv(p, ev);
    uint32_t rv32 = (uint32_t)(*ev);
    uint32_t lv32 = (uint32_t)reg.AX;
    lv32 *= rv32;
    reg.AX = lv32 & 0xFFFF;
    reg.DX = (lv32 >> 16) & 0xFFFF;
    // TODO UpdateFlag
    reg.set_flag(Flag::ZF, reg.AX == 0 && reg.DX == 0);
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
    switch (lv) {
      case 0x40:
      case 0x43:
        break;
      default:
        LOG(FATAL) << "NotImplemented: out Ib, al - Ib: " << hex2str(lv) << " AL: " << hex2str(reg.AL);
    };
    reg.IP += 1;
  }
  void POPw(uint16_t &lv) {
    lv = mem.get<uint16_t>(reg.SS, reg.SP);
    reg.SP += 2;
  }
  void POPb(uint8_t &lv) {
    lv = mem.get<uint8_t>(reg.SS, reg.SP);
    reg.SP += 1;
  }
  void PUSHw(uint16_t &rv) {
    reg.SP -= 2;
    mem.get<uint16_t>(reg.SS, reg.SP) = rv;
  }
  void PUSHb(uint8_t &rv) {
    reg.SP -= 1;
    mem.get<uint8_t>(reg.SS, reg.SP) = rv;
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
  void PUSHF() {
    PUSHw(reg.FR);
  }
  void PUSHIb(uint8_t *p) {
    PUSHb(*p);
    reg.IP += 1;
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
  void POPF() {
    POPw(reg.FR);
  }
  void RET() {
    POPw(reg.IP);
  }
  void IRET() {
    POPw(reg.IP);
    POPw(reg.CS);
    POPw(reg.FR);
  }
  void REP() {
    rep_mode = true;
    rep_CS = reg.CS;
    rep_IP = reg.IP + 1;
    use_prefix = true;
    rep_next_cs = 0xFFFF;
    rep_next_ip = 0xFFFF;
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
    reg.set_flag(Flag::ZF, lv == 0);
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
    reg.set_flag(Flag::ZF, lv == 0);
    reg.IP += 2;
  }
  void STI() {
    reg.set_flag(Flag::IF);
  }
  void SUBEbGb(uint8_t *p) {
    uint8_t *eb, *gb;
    _GetEvGv(p, eb, gb);
    *eb = _SUB<int8_t, uint8_t>(*eb, *gb);
  }
  void SUBEvGv(uint8_t *p) {
    uint16_t *ev, *gv;
    _GetEvGv(p, ev, gv);
    *ev = _SUB<int16_t, uint16_t>(*ev, *gv);
  }
  void SUBGbEb(uint8_t *p) {
    uint8_t *eb, *gb;
    _GetEvGv(p, eb, gb);
    *gb = _SUB<int8_t, uint8_t>(*gb, *eb);
  }
  void SUBGvEv(uint8_t *p) {
    uint16_t *ev, *gv;
    _GetEvGv(p, ev, gv);
    *gv = _SUB<int16_t, uint16_t>(*gv, *ev);
  }
private:
  template<typename T, typename uT>
  uT _ADD(uT dest, uT source) {
    uT res = dest + source;
    reg.set_flag(Flag::ZF, res == 0);
    bool pos_res = static_cast<T>(res) >= 0;
    bool pos_dest = static_cast<T>(dest) >= 0;
    bool pos_source = static_cast<T>(source) >= 0;
    bool overflow = res != 0 && (pos_dest == pos_source) && pos_res != pos_dest;
    reg.set_flag(Flag::OF, overflow);
    reg.set_flag(Flag::SF, !pos_res);
    return res;
  }
  template<typename T, typename uT>
  uT _SUB(uT dest, uT source) {
    uT res = dest - source;
    reg.set_flag(Flag::ZF, res == 0);
    reg.set_flag(Flag::CF, dest < source);
    bool pos_res = static_cast<T>(res) >= 0;
    bool pos_dest = static_cast<T>(dest) >= 0;
    bool pos_source = static_cast<T>(source) >= 0;
    bool overflow = res != 0 && (pos_dest != pos_source) && pos_res != pos_dest;
    reg.set_flag(Flag::OF, overflow);
    reg.set_flag(Flag::SF, !pos_res);
    return res;
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
  void SetSegPrefix(uint16_t &reg) {
    pre_seg = &reg;
    use_prefix = true;
  }
  void Set32bPrefix() {
    use_prefix = true;
  }
private:
  void ScreenINT() {
    switch (reg.AX) {
      case 0x03:
      case 0x13:
        break;
      default:
        LOG(FATAL) << "NotImplemented ScreenINT: " << hex2str(reg.AH);
    };
  }
  void KeyBoardINT() {
    switch (reg.AH) {
      case 0x00:
        reg.AL = gui.get_key();
        break;
      default:
        LOG(FATAL) << "NotImplemented KeyBoardINT: " << hex2str(reg.AH);
    };
  }
  void ClockINT() {
    time_t cur_time;
    time(&cur_time);
    tm *time_p = localtime(&cur_time);
    switch (reg.AX) {
      case 0x0200:
        reg.CH = time_p->tm_hour;
        reg.CL = time_p->tm_min;
        reg.DH = time_p->tm_sec;
        reg.DL = 0;
        reg.unset_flag(Flag::CF);
        break;
      default:
        LOG(FATAL) << "NotImplemented ClockINT: " << hex2str(reg.AH);
    };
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
  bool rep_mode = false;
  uint16_t rep_CS, rep_IP; 
  uint16_t rep_next_cs, rep_next_ip;
private:
  Registers reg;
  Memory mem;
  GUI gui;
  const char hexch[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
  map<uint32_t, std::function<void()> > inject_functions;
  queue<uint8_t> key_buffer;
  time_point last_int08h_time;
  bool use_prefix;
};

