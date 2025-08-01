#pragma once
#include <array>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <queue>
#include <sstream>
#include <stack>
#include <string>
#include <vector>
using namespace std;
#include "ai.h"
#include "gui.h"
#include "instruction.h"
#include "logging.h"
#include "memory.h"
#include "register.h"
#include "shared_memory.h"
#include "symbol.h"

const int UpdateTimes = 40;
using time_point = std::chrono::steady_clock::time_point;
using ms_type = std::chrono::duration<int, ratio<1, 1000>>;
using milliseconds = std::chrono::milliseconds;

inline time_point get_time_now() { return std::chrono::steady_clock::now(); }

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
  const uint32_t base_addr = 0x7e00;
  GUI gui;
  Machine() : shared_mem("/happyqqt_shm", 0x100000) {
    // mem.init(new uint8_t[0x100000]);
    mem.init(shared_mem.data());
    memset(&reg, 0, sizeof(reg));
    reg.FR = 512;
    gui.set_video_addr(&(mem.get(0xa000, 0)));
    InitIVT();
    last_int08h_time = get_time_now();
    InitNum1();
  }
  void InitIVT() {
    for (int i = 0; i < 0x100; ++i) {
      mem.get<uint16_t>(i * 4) = i;
      mem.get<uint16_t>(i * 4 + 2) = 0xFFFF;
    }
  }
  void InitNum1() {
    for (int i = 0; i < 256; ++i) {
      int v = i;
      int c = 0;
      while (v > 0) {
        ++c;
        v &= v - 1;
      }
      num_1bits[i] = c;
    }
  }
  void load(const string &filename) {
    reg.IP = base_addr;
    ifstream fin(filename);
    string line;
    getline(fin, line);
    while (!fin.eof()) {
      read_deasm(line);
      getline(fin, line);
    }

    // protect CODE
    mem.protect(base_addr + 0x17d, base_addr + 0xf75 + 1);
    // do not protect RAND_SEED
    mem.protect(base_addr + 0x30e, base_addr + 0x30e + 1, false);
    // TQAX, TQAY, FBInd, FBpos
    mem.protect(base_addr + 0x9de, base_addr + 0x9e6, false);
    // OldPower
    mem.protect(base_addr + 0x636, base_addr + 0x63c, false);
  }
  void inject_qqt() {
    mem.get(base_addr + 0x17d) = INJECT_FUNC_CODE;
    inject_functions[base_addr + 0x17d] = std::bind(&Machine::ReadFloppy, this);
    // 0x3152 is the function Update
    mem.get(base_addr + 0x3152) = INJECT_FUNC_CODE;
    inject_functions[base_addr + 0x3152] = std::bind(&Machine::QQTUpdate, this);
    // 0x3276 is the function AI
    mem.get(base_addr + 0x3276) = INJECT_FUNC_CODE;
    inject_functions[base_addr + 0x3276] = std::bind(&Machine::QQTAI, this);

    // inject ai
    auto get_mem_ptr = [&](string name) -> void * {
      uint8_t *p = &mem.get<uint8_t>(base_addr + SYMBOLS.at(name));
      return (void *)p;
    };
#define bind_var(name) (name = static_cast<decltype(name)>(get_mem_ptr(#name)))
    bind_var(Players);
    bind_var(PASSED_DATA);
    bind_var(STATE_DATA);
    bind_var(Bombs);
    bind_var(PlayerHP);
    bind_var(BossHP);
    bind_var(PlayerNOHARM);
    bind_var(BossNOHARM);
    CHECK_EQ(*BossHP, 5) << int(*BossHP);
    CHECK_EQ(*PlayerHP, 1);
  }
  void run() {
    while (1) {
      if (recording) {
        // string info = GetCurrentState();
        string info = hex2str(reg.IP - base_addr) + ":" +
                      hex2str(mem.get<uint8_t>(reg.IP)) + ":" + hex2str(reg.CX);
        history.push(info);
        if (history.size() > 100) history.pop();
      }

      uint32_t addr = get_addr(reg.CS, reg.IP);
      uint8_t *p = &(mem.get(addr));

      // process prefix
      bool use_prefix = true;
      switch (*p) {
        case 0x26:
          SetSegPrefix(&reg.ES);
          break;
        case 0x2e:
          SetSegPrefix(&reg.CS);
          break;
        case 0x36:
          SetSegPrefix(&reg.SS);
          break;
        case 0x3e:
          SetSegPrefix(&reg.DS);
          break;
        default:
          use_prefix = false;
      }
      if (use_prefix) {
        reg.IP += 1;
        uint32_t addr = get_addr(reg.CS, reg.IP);
        p = &(mem.get(addr));
      }

      switch (*p) {
        case 0x00:
          ADDEbGb(p + 1);
          break;
        case 0x01:
          ADDEvGv(p + 1);
          break;
        case 0x02:
          ADDGbEb(p + 1);
          break;
        case 0x03:
          ADDGvEv(p + 1);
          break;
        case 0x04:
          ADDALIb(p + 1);
          break;
        case 0x05:
          ADDeAXIv(p + 1);
          break;
        case 0x06:
          PUSHw(reg.ES);
          break;
        case 0x07:
          POPw(reg.ES);
          break;
        case 0x08:
          OREbGb(p + 1);
          break;
        case 0x09:
          OREvGv(p + 1);
          break;
        case 0x0a:
          ORGbEb(p + 1);
          break;
        case 0x0b:
          ORGvEv(p + 1);
          break;
        case 0x0c:
          ORALIb(p + 1);
          break;
        case 0x0d:
          OReAXIv(p + 1);
          break;
        case 0x0e:
          PUSHw(reg.CS);
          break;
        case 0x0f:
          TWOBYTE(p + 1);
          break;
        case 0x10:
          ADCEbGb(p + 1);
          break;
        case 0x11:
          ADCEvGv(p + 1);
          break;
        case 0x12:
          ADCGbEb(p + 1);
          break;
        case 0x13:
          ADCGvEv(p + 1);
          break;
        case 0x14:
          ADCALIb(p + 1);
          break;
        case 0x16:
          PUSHw(reg.SS);
          break;
        case 0x17:
          POPw(reg.SS);
          break;
        case 0x18:
          SBBEbGb(p + 1);
          break;
        case 0x19:
          SBBEvGv(p + 1);
          break;
        case 0x1a:
          SBBGbEb(p + 1);
          break;
        case 0x1b:
          SBBGvEv(p + 1);
          break;
        case 0x1c:
          SBBALIb(p + 1);
          break;
        case 0x1e:
          PUSHw(reg.DS);
          break;
        case 0x1f:
          POPw(reg.DS);
          break;
        case 0x20:
          ANDEbGb(p + 1);
          break;
        case 0x21:
          ANDEvGv(p + 1);
          break;
        case 0x22:
          ANDGbEb(p + 1);
          break;
        case 0x23:
          ANDGvEv(p + 1);
          break;
        case 0x24:
          ANDALIb(p + 1);
          break;
        case 0x25:
          ANDeAXIv(p + 1);
          break;
        case 0x27:
          DAA();
          break;
        case 0x28:
          SUBEbGb(p + 1);
          break;
        case 0x29:
          SUBEvGv(p + 1);
          break;
        case 0x2a:
          SUBGbEb(p + 1);
          break;
        case 0x2b:
          SUBGvEv(p + 1);
          break;
        case 0x2c:
          SUBALIb(p + 1);
          break;
        case 0x2d:
          SUBeAXIv(p + 1);
          break;
        case 0x2f:
          DAS();
          break;
        case 0x30:
          XOREbGb(p + 1);
          break;
        case 0x31:
          XOREvGv(p + 1);
          break;
        case 0x32:
          XORGbEb(p + 1);
          break;
        case 0x33:
          XORGvEv(p + 1);
          break;
        case 0x34:
          XORALIb(p + 1);
          break;
        case 0x35:
          XOReAXIv(p + 1);
          break;
        case 0x37:
          AAA();
          break;
        case 0x38:
          CMPEbGb(p + 1);
          break;
        case 0x39:
          CMPEvGv(p + 1);
          break;
        case 0x3a:
          CMPGbEb(p + 1);
          break;
        case 0x3b:
          CMPGvEv(p + 1);
          break;
        case 0x3c:
          CMPALIb(p + 1);
          break;
        case 0x3d:
          CMPeAXIv(p + 1);
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
        case 0x62:
          BOUNDGvMa(p + 1);
          break;
        case 0x66:
          PrintState();
          LOG(FATAL) << "Does not support x86 Instruction";
          Set32bPrefix();
          break;
        case 0x67:
          // dummy
          break;
        case 0x6a:
          PUSHIb(p + 1);
          break;
        case 0x72:
          JC(p + 1);
          break;
        case 0x73:
          JNC(p + 1);
          break;
        case 0x74:
          JZ(p + 1);
          break;
        case 0x75:
          JNZ(p + 1);
          break;
        case 0x76:
          JNA(p + 1);
          break;
        case 0x77:
          JA(p + 1);
          break;
        case 0x78:
          JS(p + 1);
          break;
        case 0x79:
          JNS(p + 1);
          break;
        case 0x7a:
          JP(p + 1);
          break;
        case 0x7b:
          JNP(p + 1);
          break;
        case 0x7c:
          JL(p + 1);
          break;
        case 0x7d:
          JNL(p + 1);
          break;
        case 0x7e:
          JNG(p + 1);
          break;
        case 0x7f:
          JG(p + 1);
          break;
        case 0x80:
          CMPEbIb(p + 1);
          break;
        case 0x81:
          MultiEvIv(p + 1);
          break;
        case 0x83:
          ADDSUBEvIb(p + 1);
          break;
        case 0x84:
          TESTEbGb(p + 1);
          break;
        case 0x85:
          TESTEvGv(p + 1);
          break;
        case 0x88:
          MOVEbGb(p + 1);
          break;
        case 0x89:
          MOVEvGv(p + 1);
          break;
        case 0x8a:
          MOVGbEb(p + 1);
          break;
        case 0x8b:
          MOVGvEv(p + 1);
          break;
        case 0x8c:
          MOVEwSw(p + 1);
          break;
        case 0x8d:
          LEAGvM(p + 1);
          break;
        case 0x8e:
          MOVSwEw(p + 1);
          break;
        case 0x90:
          // nop
          break;
        case 0xa0:
          MOVALOb(p + 1);
          break;
        case 0xa1:
          MOVeAXOv(p + 1);
          break;
        case 0xa2:
          MOVObAL(p + 1);
          break;
        case 0xa3:
          MOVOveAX(p + 1);
          break;
        case 0xa5:
          MOVSWXvYv(p + 1);
          break;
        case 0xb0:
          MOVReg16Ib(p + 1, reg.AL);
          break;
        case 0xb1:
          MOVReg16Ib(p + 1, reg.CL);
          break;
        case 0xb2:
          MOVReg16Ib(p + 1, reg.DL);
          break;
        case 0xb3:
          MOVReg16Ib(p + 1, reg.BL);
          break;
        case 0xb4:
          MOVReg16Ib(p + 1, reg.AH);
          break;
        case 0xb5:
          MOVReg16Ib(p + 1, reg.CH);
          break;
        case 0xb6:
          MOVReg16Ib(p + 1, reg.DH);
          break;
        case 0xb7:
          MOVReg16Ib(p + 1, reg.BH);
          break;
        case 0xb8:
          MOVeIv(p + 1, reg.AX);
          break;
        case 0xb9:
          MOVeIv(p + 1, reg.CX);
          break;
        case 0xba:
          MOVeIv(p + 1, reg.DX);
          break;
        case 0xbb:
          MOVeIv(p + 1, reg.BX);
          break;
        case 0xbc:
          MOVeIv(p + 1, reg.SP);
          break;
        case 0xbd:
          MOVeIv(p + 1, reg.BP);
          break;
        case 0xbe:
          MOVeIv(p + 1, reg.SI);
          break;
        case 0xbf:
          MOVeIv(p + 1, reg.DI);
          break;
        case 0xc0:
          SHEbIb(p + 1);
          break;
        case 0xc1:
          SHEvIb(p + 1);
          break;
        case 0xc3:
          RET();
          break;
        case 0xc6:
          MOVEbIb(p + 1);
          break;
        case 0xc7:
          MOVEvIv(p + 1);
          break;
        case 0xc9:
          LEAVE();
          break;
        case 0xcd:
          INT(p + 1);
          break;
        case 0xcf:
          IRET();
          break;
        case INJECT_FUNC_CODE:
          CallInjectFunc();
          break;
        case 0xd0:
          SHEb1(p + 1);
          break;
        case 0xe2:
          LOOP(p + 1);
          break;
        case 0xe3:
          JCXZ(p + 1);
          break;
        case 0xe8:
          CALL(p + 1);
          break;
        case 0xe6:
          OUTIbAL(p + 1);
          break;
        case 0xe9:
          JMPJz(p + 1);
          break;
        case 0xeb:
          JMPJb(p + 1);
          break;
        case 0xee:
          OUTDXAL();
          break;
        case 0xf3:
          REP(p + 1);
          break;
        case 0xf6:
          MULEb(p + 1);
          break;
        case 0xf7:
          MULEv(p + 1);
          break;
        case 0xfa:
          CLI();
          break;
        case 0xfb:
          STI();
          break;
        case 0xfe:
          INCDECb(p + 1);
          break;
        case 0xff:
          INCDECw(p + 1);
          break;
        default:
          PrintHistory();
          LOG(FATAL) << "Unknown OpCode: [" << hex2str(reg.CS) << ":"
                     << hex2str(reg.IP) << "=" << hex2str(addr - base_addr)
                     << "]" << hex2str(*p);
      }

      time_point cur_time = get_time_now();
      int diff_ms =
          std::chrono::duration_cast<milliseconds>(cur_time - last_int08h_time)
              .count();
      if (diff_ms >= 1000 / UpdateTimes) {
        if (CALL_INT(0x08)) {
          reg.IP -= 1;
        }
        last_int08h_time = cur_time;
      }

      reg.IP += 1;
    }
  }

 private:
  void AAA() {
    if ((reg.AL & 0xf) > 9 || reg.get_flag(Flag::AF)) {
      reg.AL += 6;
      reg.AH += 1;
      reg.set_flag(Flag::AF | Flag::CF);
    } else {
      reg.unset_flag(Flag::AF | Flag::CF);
    }
    reg.AL &= 0xf;
    // TOFIX
    UpdateFlag(reg.AX);
  }
  void ADDEbGb(uint8_t *p) {
    uint8_t *eb, *gb;
    _GetEvGv(p, eb, gb);
    *eb = _ADD<int8_t, uint8_t>(*eb, *gb);
    MemoryProtect(eb);
  }
  void ADDEvGv(uint8_t *p) {
    uint16_t *ev, *gv;
    _GetEvGv(p, ev, gv);
    *ev = _ADD<int16_t, uint16_t>(*ev, *gv);
    MemoryProtect(ev);
  }
  void ADDGbEb(uint8_t *p) {
    uint8_t *eb, *gb;
    _GetEvGv(p, eb, gb);
    *gb = _ADD<int8_t, uint8_t>(*gb, *eb);
    MemoryProtect(gb);
  }
  void ADDGvEv(uint8_t *p) {
    uint16_t *ev, *gv;
    _GetEvGv(p, ev, gv);
    *gv = _ADD<int16_t, uint16_t>(*gv, *ev);
    MemoryProtect(gv);
  }
  void ADDeAXIv(uint8_t *p) {
    uint16_t &rv = *reinterpret_cast<uint16_t *>(p);
    reg.AX = _ADD<int16_t, uint16_t>(reg.AX, rv);
    reg.IP += use_32bits_mode ? 4 : 2;
  }
  void ADCEbGb(uint8_t *p) {
    uint8_t *eb, *gb;
    _GetEvGv(p, eb, gb);
    *eb = _ADD<int8_t, uint8_t>(*eb, *gb, reg.get_flag(Flag::CF));
    MemoryProtect(eb);
  }
  void ADCEvGv(uint8_t *p) {
    uint16_t *ev, *gv;
    _GetEvGv(p, ev, gv);
    *ev = _ADD<int16_t, uint16_t>(*ev, *gv, reg.get_flag(Flag::CF));
    MemoryProtect(ev);
  }
  void ADCGbEb(uint8_t *p) {
    uint8_t *eb, *gb;
    _GetEvGv(p, eb, gb);
    *gb = _ADD<int8_t, uint8_t>(*gb, *eb, reg.get_flag(Flag::CF));
    MemoryProtect(gb);
  }
  void ADCGvEv(uint8_t *p) {
    uint16_t *ev, *gv;
    _GetEvGv(p, ev, gv);
    *gv = _ADD<int16_t, uint16_t>(*gv, *ev, reg.get_flag(Flag::CF));
    MemoryProtect(gv);
  }
  void ADCALIb(uint8_t *p) {
    // Acc, Imm
    uint8_t &lv = reg.AL;
    uint8_t &rv = *reinterpret_cast<uint8_t *>(p);
    lv = _ADD<int8_t, uint8_t>(lv, rv, reg.get_flag(Flag::CF));
    reg.IP += 1;
    UpdateFlag(lv);
  }
  void ANDALIb(uint8_t *p) {
    // Acc, Imm
    uint8_t &lv = reg.AL;
    uint8_t &rv = *reinterpret_cast<uint8_t *>(p);
    lv &= rv;
    reg.IP += 1;
    UpdateFlag(lv);
  }
  void ANDeAXIv(uint8_t *p) {
    uint16_t &rv = *reinterpret_cast<uint16_t *>(p);
    reg.AX &= rv;
    reg.IP += use_32bits_mode ? 4 : 2;
    UpdateFlag(reg.AX);
  }
  void ANDEbGb(uint8_t *p) {
    uint8_t *eb, *gb;
    _GetEvGv(p, eb, gb);
    *eb &= *gb;
    UpdateFlag(*eb);
    MemoryProtect(eb);
  }
  void ANDEvGv(uint8_t *p) {
    uint16_t *ev, *gv;
    _GetEvGv(p, ev, gv);
    *ev &= *gv;
    UpdateFlag(*ev);
    MemoryProtect(ev);
  }
  void ANDGbEb(uint8_t *p) {
    uint8_t *eb, *gb;
    _GetEvGv(p, eb, gb);
    *gb &= *eb;
    UpdateFlag(*gb);
    MemoryProtect(gb);
  }
  void ANDGvEv(uint8_t *p) {
    uint16_t *ev, *gv;
    _GetEvGv(p, ev, gv);
    *gv &= *ev;
    UpdateFlag(*gv);
    MemoryProtect(gv);
  }
  void MultiEvIv(uint8_t *p) {
    // Mem, Imm
    uint16_t *ev, *iv;
    _GetEvIv(p, ev, iv);
    ModRM &modrm = read_oosssmmm(p);
    switch (modrm.REG) {
      case 0b000:
        // add
        *ev = _ADD<int16_t, uint16_t>(*ev, *iv);
        break;
      case 0b100:
        // and
        *ev &= *iv;
        UpdateFlag(*ev);
        break;
      case 0b101:
        // sub
        *ev = _SUB<int16_t, uint16_t>(*ev, *iv);
        break;
      case 0b111:
        // cmp
        _SUB<int16_t, uint16_t>(*ev, *iv);
        break;
      default:
        LOG(FATAL) << "Not supported " << hex2str(modrm);
    }
    MemoryProtect(ev);
  }
  void ADDALIb(uint8_t *p) {
    // Acc, Imm
    uint8_t &lv = reg.AL;
    uint8_t &rv = *reinterpret_cast<uint8_t *>(p);
    lv = _ADD<int8_t, uint8_t>(lv, rv);
    reg.IP += 1;
  }
  void ADDSUBEvIb(uint8_t *p) {
    // Although ev is 16 bit,s but the computation is 8 bits.
    ModRM &modrm = read_oosssmmm(p);
    uint16_t *ev;
    uint8_t *uiv;
    _GetEvIv(p, ev, uiv);
    // iv should be signed
    int8_t iv = static_cast<int8_t>(*uiv);
    switch (modrm.REG) {
      case 0b000:
        *ev = _ADD<int16_t, uint16_t>(*ev, iv);
        break;
      case 0b100:
        *ev = _AND<int16_t, uint16_t>(*ev, iv);
        break;
      case 0b101:
        *ev = _SUB<int16_t, uint16_t>(*ev, iv);
        break;
      case 0b010:
        *ev = _ADD<int16_t, uint16_t>(*ev, iv, reg.get_flag(Flag::CF));
        break;
      case 0b111:
        _SUB<int16_t, uint16_t>(*ev, iv);
        break;
      default:
        PrintState();
        PrintHistory();
        LOG(FATAL) << "Wrong modrm: " << hex2str(modrm);
    };
    MemoryProtect(ev);
  }
  void BOUNDGvMa(uint8_t *p) {
    // TODO
    reg.IP += 2;
  }
  void CALL(uint8_t *p) {
    reg.IP += use_32bits_mode ? 4 : 2;
    uint16_t IPPlus1 = reg.IP + 1;
    PUSHw(IPPlus1);
    reg.IP += *reinterpret_cast<uint16_t *>(p);
  }
  void CALLF(uint16_t seg, uint16_t offset) {
    PUSHw(reg.CS);
    uint16_t IPPlus1 = reg.IP + 1;
    PUSHw(IPPlus1);
    reg.CS = seg;
    reg.IP = offset;
  }
  bool CALL_INT(uint16_t id) {
    // if (id == 8) return false;
    if (!reg.get_flag(Flag::IF)) {
      return false;
    }
    // cout << "CALL INT: " << id << endl;
    uint16_t &offset = mem.get<uint16_t>(id * 4);
    uint16_t &seg = mem.get<uint16_t>(id * 4 + 2);
    // cout << "=====III" << id << "|" << hex2str(reg.FR) << ", " <<
    // hex2str(reg.CS) << ", " << hex2str(reg.IP) << "!" << hex2str(seg) << ", "
    // << hex2str(offset) << endl;
    PUSHF();
    reg.unset_flag(Flag::IF);
    if (seg == 0xFFFF) {
      CallDefaultINT(offset);
      return false;
    } else {
      CALLF(seg, offset);
    }
    return true;
  }
  void CallDefaultINT(uint16_t id) {
    PUSHw(reg.CS);
    uint16_t IPPlus1 = reg.IP + 1;
    PUSHw(IPPlus1);
    switch (id) {
      case 0x08:
        break;
      case 0x10:
        ScreenINT();
        break;
      case 0x1a:
        ClockINT();
        break;
      default:
        if (id == 0x16) {
          bool zf = KeyBoardINT();
          IRET();
          reg.set_flag(Flag::ZF, zf);
          return;
        }
        LOG(FATAL) << "NotImplemented INT: " << hex2str(id) << " In "
                   << hex2str(reg.CS) << ":" << hex2str(reg.IP);
    };
    IRET();
  }
  void CLI() { reg.unset_flag(Flag::IF); }
  inline uint8_t _SUB8(uint8_t dest, uint8_t source) {
    return _SUB<int8_t, uint8_t>(dest, source);
  }
  void CMPALIb(uint8_t *p) {
    _SUB8(reg.AL, *p);
    reg.IP += 1;
  }
  void CMPeAXIv(uint8_t *p) {
    uint16_t &rv = *reinterpret_cast<uint16_t *>(p);
    _SUB<int16_t, uint16_t>(reg.AX, rv);
    reg.IP += use_32bits_mode ? 4 : 2;
  }
  void CMPEbIb(uint8_t *p) {
    uint8_t *ev, *iv;
    _GetEvIv(p, ev, iv);
    _SUB8(*ev, *iv);
  }
  void CMPEbGb(uint8_t *p) {
    uint8_t *eb, *gb;
    _GetEvGv(p, eb, gb);
    _SUB<int8_t, uint8_t>(*eb, *gb);
  }
  void CMPEvGv(uint8_t *p) {
    uint16_t *ev, *gv;
    _GetEvGv(p, ev, gv);
    _SUB<int16_t, uint16_t>(*ev, *gv);
  }
  void CMPGbEb(uint8_t *p) {
    uint8_t *eb, *gb;
    _GetEvGv(p, eb, gb);
    _SUB<int8_t, uint8_t>(*gb, *eb);
  }
  void CMPGvEv(uint8_t *p) {
    uint16_t *ev, *gv;
    _GetEvGv(p, ev, gv);
    _SUB<int16_t, uint16_t>(*gv, *ev);
  }
  void DAA() {
    // TOFIX
    bool success = false;
    if ((reg.AL & 0xf) > 9 || reg.get_flag(Flag::AF)) {
      reg.AL += 6;
      reg.set_flag(Flag::AF);
      success = true;
    }
    if ((((reg.AL >> 4) & 0xf) > 9) || reg.get_flag(Flag::CF)) {
      reg.AL += 0x60;
      reg.set_flag(Flag::CF);
      success = true;
    }
    if (!success) {
      reg.unset_flag(Flag::AF | Flag::CF);
    }
    UpdateFlag(reg.AL);
  }
  void DAS() {
    // TOFIX
    bool success = false;
    if ((reg.AL & 0xf) > 9 || reg.get_flag(Flag::AF)) {
      reg.AL -= 6;
      reg.set_flag(Flag::AF);
      success = true;
    }
    if (reg.AL > 0x9f || reg.get_flag(Flag::CF)) {
      reg.AL -= 0x60;
      reg.set_flag(Flag::CF);
      success = true;
    }
    if (!success) {
      reg.unset_flag(Flag::AF | Flag::CF);
    }
    UpdateFlag(reg.AL);
  }
  void DEC(uint16_t &lv) { lv = _SUB<int16_t, uint16_t>(lv, 1); }
  void INC(uint16_t &lv) { lv = _ADD<int16_t, uint16_t>(lv, 1); }
  void INC(uint8_t &lv) { lv = _ADD<int8_t, uint8_t>(lv, 1); }
  void INCDECb(uint8_t *p) {
    ModRM &modrm = read_oosssmmm(p);
    uint8_t *v;
    _GetEv(p, v);
    if (modrm.REG == 0b000) {
      *v = _ADD<int8_t, uint8_t>(*v, 1);
    } else {
      if (modrm.REG != 0b001) PrintHistory();
      CHECK(modrm.REG == 0b001)
          << hex2str(*p) << "!" << hex2str(reg.CS) << ":" << hex2str(reg.IP);
      *v = _SUB<int8_t, uint8_t>(*v, 1);
    }
    MemoryProtect(v);
  }
  void INCDECw(uint8_t *p) {
    ModRM &modrm = read_oosssmmm(p);
    uint16_t *v;
    _GetEv(p, v);
    if (modrm.REG == 0b000) {
      *v = _ADD<int16_t, uint16_t>(*v, 1);
    } else {
      if (modrm.REG != 0b001) PrintHistory();
      CHECK(modrm.REG == 0b001)
          << hex2str(*p) << "!" << hex2str(reg.CS) << ":" << hex2str(reg.IP);
      *v = _SUB<int16_t, uint16_t>(*v, 1);
    }
    MemoryProtect(v);
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
  void _IPStep(uint16_t step) { reg.IP += step; }
  void JMPJb(uint8_t *p) {
    _IPStep(*reinterpret_cast<uint8_t *>(p));
    reg.IP += 1;
  }
  void JMPJz(uint8_t *p) {
    _IPStep(*reinterpret_cast<uint16_t *>(p));
    reg.IP += 2;
  }
  void JC(uint8_t *p) {
    if (reg.get_flag(Flag::CF)) _IPStep(*reinterpret_cast<uint8_t *>(p));
    reg.IP += 1;
  }
  void JNC(uint8_t *p) {
    if (!reg.get_flag(Flag::CF)) _IPStep(*reinterpret_cast<uint8_t *>(p));
    reg.IP += 1;
  }
  void JZ(uint8_t *p) {
    if (reg.get_flag(Flag::ZF)) _IPStep(*reinterpret_cast<uint8_t *>(p));
    reg.IP += 1;
  }
  void JCXZ(uint8_t *p) {
    if (reg.CX == 0) _IPStep(*reinterpret_cast<uint8_t *>(p));
    reg.IP += 1;
  }
  void JNZ(uint8_t *p) {
    if (!reg.get_flag(Flag::ZF)) _IPStep(*reinterpret_cast<uint8_t *>(p));
    reg.IP += 1;
  }
  void JNA(uint8_t *p) {
    if (reg.get_flag(Flag::ZF | Flag::CF))
      _IPStep(*reinterpret_cast<uint8_t *>(p));
    reg.IP += 1;
  }
  void JA(uint8_t *p) {
    if (!reg.get_flag(Flag::ZF | Flag::CF))
      _IPStep(*reinterpret_cast<uint8_t *>(p));
    reg.IP += 1;
  }
  void JS(uint8_t *p) {
    if (reg.get_flag(Flag::SF)) _IPStep(*reinterpret_cast<uint8_t *>(p));
    reg.IP += 1;
  }
  void JNS(uint8_t *p) {
    if (!reg.get_flag(Flag::SF)) _IPStep(*reinterpret_cast<uint8_t *>(p));
    reg.IP += 1;
  }
  void JP(uint8_t *p) {
    if (reg.get_flag(Flag::PF)) _IPStep(*reinterpret_cast<uint8_t *>(p));
    reg.IP += 1;
  }
  void JNP(uint8_t *p) {
    if (!reg.get_flag(Flag::PF)) _IPStep(*reinterpret_cast<uint8_t *>(p));
    reg.IP += 1;
  }
  void JNG(uint8_t *p) {
    if (reg.get_flag(Flag::ZF) ||
        (reg.get_flag(Flag::SF) != reg.get_flag(Flag::OF)))
      _IPStep(*reinterpret_cast<uint8_t *>(p));
    reg.IP += 1;
  }
  void JG(uint8_t *p) {
    if (!reg.get_flag(Flag::ZF) &&
        (reg.get_flag(Flag::SF) == reg.get_flag(Flag::OF)))
      _IPStep(*reinterpret_cast<uint8_t *>(p));
    reg.IP += 1;
  }
  void JNL(uint8_t *p) {
    // not less than
    if (reg.get_flag(Flag::ZF) ||
        (reg.get_flag(Flag::SF) == reg.get_flag(Flag::OF)))
      _IPStep(*reinterpret_cast<uint8_t *>(p));
    reg.IP += 1;
  }
  void JL(uint8_t *p) {
    // less than
    if (!reg.get_flag(Flag::ZF) &&
        (reg.get_flag(Flag::SF) != reg.get_flag(Flag::OF)))
      _IPStep(*reinterpret_cast<uint8_t *>(p));
    reg.IP += 1;
  }
  void LOOP(uint8_t *p) {
    if (--reg.CX != 0) {
      _IPStep(*p);
    }
    reg.IP += 1;
  }
  void LEAVE() {
    reg.SP = reg.BP;
    POPw(reg.BP);
    // cout << "LEAVEEE SP: " << hex2str(reg.SP);
  }
  void LEAGvM(uint8_t *p) {
    ModRM &modrm = read_oosssmmm(p);
    CHECK(modrm.MOD == 0b01);
    uint16_t *gv, *ev;
    gv = &GetReg16(modrm.REG);
    ev = &GetReg16(modrm.RM);
    *gv = *ev + static_cast<uint16_t>(*(p + 1));
    reg.IP += 2;
  }
  template <typename uT>
  void _GetGv_nostep(uint8_t *p, uT *&gv) {
    ModRM &modrm = read_oosssmmm(p);
    gv = &GetReg<uT>(modrm.REG);
  }
  int _GetSegAndOffset(uint8_t *p, uint16_t &offset) {
    int ip_update = 1;
    ModRM &modrm = read_oosssmmm(p);
    if (!pre_seg) {
      switch (modrm.RM) {
        case 0b000:
        case 0b001:
        case 0b100:
        case 0b101:
        case 0b111:
          pre_seg = &reg.DS;
          break;
        case 0b010:
        case 0b011:
        case 0b110:
          pre_seg = &reg.SS;
          break;
        default:
          LOG(FATAL) << "Unknown modrm.RM: " << hex2str(modrm.RM);
      };
    }

    if (modrm.MOD == 0b00 && modrm.RM == 0b110) {
      offset = *reinterpret_cast<uint16_t *>(p + 1);
      ip_update += 2;
    } else {
      switch (modrm.RM) {
        case 0b000:
          offset = reg.BX + reg.SI;
          break;
        case 0b001:
          offset = reg.BX + reg.DI;
          break;
        case 0b010:
          offset = reg.BP + reg.SI;
          break;
        case 0b011:
          offset = reg.BP + reg.DI;
          break;
        case 0b100:
          offset = reg.SI;
          break;
        case 0b101:
          offset = reg.DI;
          break;
        case 0b110:
          offset = reg.BP;
          break;
        case 0b111:
          offset = reg.BX;
          break;
        default:
          LOG(FATAL) << "Unknown modrm.RM: " << hex2str(modrm.RM);
      };

      if (modrm.MOD == 0b01) {
        offset += *(p + 1);
        ip_update += 1;
      } else if (modrm.MOD == 0b10) {
        offset += *reinterpret_cast<uint16_t *>(p + 1);
        ip_update += 2;
      } else
        CHECK(modrm.MOD == 0b00)
            << hex2str(reg.CS) << ":" << hex2str(reg.IP) << ", "
            << hex2str(reg.IP - base_addr) << ", " << hex2str(*p);
    }
    reg.IP += ip_update;
    return ip_update;
  }
  template <typename uT>
  int _GetEv(uint8_t *p, uT *&ev) {
    // MOD, RM
    ModRM &modrm = read_oosssmmm(p);
    if (modrm.MOD == 0b11) {
      ev = &GetReg<uT>(modrm.RM);
      reg.IP += 1;
      return 1;
    }
    // pre_seg != nullptr
    // Mem, Reg
    uint16_t offset;
    int ip_update = _GetSegAndOffset(p, offset);
    CHECK(pre_seg);
    uint32_t addr = get_addr(*pre_seg, offset);
    ev = &mem.get<uT>(addr);
    pre_seg = nullptr;
    return ip_update;
  }
  template <typename eT, typename gT>
  void _GetEvGv(uint8_t *p, eT *&ev, gT *&gv) {
    _GetGv_nostep(p, gv);
    _GetEv(p, ev);
  }
  template <typename uT, typename iT>
  void _GetEvIv(uint8_t *p, uT *&ev, iT *&iv) {
    iv = reinterpret_cast<iT *>(p + _GetEv(p, ev));
    reg.IP += sizeof(iT);
  }
  void MOVEbGb(uint8_t *p) {
    uint8_t *eb, *gb;
    _GetEvGv(p, eb, gb);
    *eb = *gb;
    MemoryProtect(eb);
  }
  void MOVEvGv(uint8_t *p) {
    uint16_t *ev, *gv;
    _GetEvGv(p, ev, gv);
    *ev = *gv;
    MemoryProtect(ev);
  }
  void MOVGbEb(uint8_t *p) {
    uint8_t *eb, *gb;
    _GetEvGv(p, eb, gb);
    *gb = *eb;
    MemoryProtect(gb);
  }
  void MOVGvEv(uint8_t *p) {
    uint16_t *ev, *gv;
    _GetEvGv(p, ev, gv);
    *gv = *ev;
    MemoryProtect(gv);
  }
  void MOVEwSw(uint8_t *p) {
    // 10001100oosssmmm
    // Reg16, Seg
    ModRM &modrm = read_oosssmmm(p);
    CHECK(modrm.MOD == 0b11);
    uint16_t &rv = GetSeg16(modrm.REG);
    uint16_t &lv = GetReg16(modrm.RM);
    lv = rv;
    reg.IP += 1;
  }
  void MOVSwEw(uint8_t *p) {
    // Seg, Reg16
    ModRM &modrm = read_oosssmmm(p);
    CHECK(modrm.MOD == 0b11);
    uint16_t &lv = GetSeg16(modrm.REG);
    uint16_t &rv = GetReg16(modrm.RM);
    lv = rv;
    reg.IP += 1;
  }
  void MOVALOb(uint8_t *p) {
    // Acc, MemOfs
    uint16_t &offset = *reinterpret_cast<uint16_t *>(p);
    if (!pre_seg) pre_seg = &reg.DS;
    uint32_t addr = get_addr(*pre_seg, offset);
    uint8_t &rv = mem.get<uint8_t>(addr);
    uint8_t &lv = reg.AL;
    pre_seg = nullptr;
    lv = rv;
    reg.IP += 2;
  }
  void MOVObAL(uint8_t *p) {
    // Acc, MemOfs
    uint16_t &offset = *reinterpret_cast<uint16_t *>(p);
    if (!pre_seg) pre_seg = &reg.DS;
    uint32_t addr = get_addr(*pre_seg, offset);
    uint8_t &lv = mem.get<uint8_t>(addr);
    uint8_t &rv = reg.AL;
    pre_seg = nullptr;
    lv = rv;
    reg.IP += 2;
    MemoryProtect(&lv);
  }
  void MOVeAXOv(uint8_t *p) {
    // Acc, MemOfs
    uint16_t &offset = *reinterpret_cast<uint16_t *>(p);
    if (!pre_seg) pre_seg = &reg.DS;
    uint32_t addr = get_addr(*pre_seg, offset);
    uint16_t &rv = mem.get<uint16_t>(addr);
    uint16_t &lv = reg.AX;
    pre_seg = nullptr;
    lv = rv;
    reg.IP += 2;
  }
  void MOVOveAX(uint8_t *p) {
    uint16_t &offset = *reinterpret_cast<uint16_t *>(p);
    if (!pre_seg) pre_seg = &reg.DS;
    uint32_t addr = get_addr(*pre_seg, offset);
    uint16_t &lv = mem.get<uint16_t>(addr);
    uint16_t &rv = reg.AX;
    pre_seg = nullptr;
    lv = rv;
    reg.IP += 2;
    MemoryProtect(&lv);
  }
  void MOVeIv(uint8_t *p, uint16_t &lv) {
    uint16_t &rv = *reinterpret_cast<uint16_t *>(p);
    lv = rv;
    reg.IP += 2;
  }
  void MOVEbIb(uint8_t *p) {
    uint8_t *ev, *iv;
    _GetEvIv(p, ev, iv);
    *ev = *iv;
    MemoryProtect(ev);
  }
  void MOVEvIv(uint8_t *p) {
    uint16_t *ev, *iv;
    _GetEvIv(p, ev, iv);
    *ev = *iv;
    MemoryProtect(ev);
  }
  void MOVReg16Ib(uint8_t *p, uint8_t &lv) {
    uint8_t &rv = *reinterpret_cast<uint8_t *>(p);
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
  void MULEb(uint8_t *p) {
    ModRM &modrm = read_oosssmmm(p);
    uint8_t *ev;
    _GetEv(p, ev);
    uint16_t rv16 = static_cast<uint16_t>(*ev);
    if (modrm.REG == 0b100) {
      uint16_t lv16 = static_cast<uint16_t>(reg.AL);
      reg.AX = lv16 * rv16;
      UpdateFlag(reg.AX);
    } else {
      CHECK(modrm.REG == 0b110);
      uint16_t lv16 = static_cast<uint16_t>(reg.AX);
      reg.AL = lv16 / rv16;
      reg.AH = lv16 % rv16;
      UpdateFlag(reg.AL);
    }
  }
  void MULEv(uint8_t *p) {
    ModRM &modrm = read_oosssmmm(p);
    uint16_t *ev;
    _GetEv(p, ev);
    uint32_t rv32 = static_cast<uint32_t>(*ev);
    if (modrm.REG == 0b100) {
      uint32_t lv32 = static_cast<uint32_t>(reg.AX);
      lv32 *= rv32;
      reg.AX = lv32 & 0xFFFF;
      reg.DX = (lv32 >> 16) & 0xFFFF;
      UpdateFlag(reg.AX);
    } else {
      CHECK(modrm.REG == 0b110);
      // DXAX
      uint32_t dxax =
          (((uint32_t)reg.DX) << 16) | (((uint32_t)reg.AX) & 0xFFFF);
      reg.AX = dxax / rv32;
      reg.DX = dxax % rv32;
      UpdateFlag(reg.AX);
    }
    // TODO UpdateFlag
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
        cout << "NotImplemented: out dx, al - DX: " << hex2str(reg.DX)
             << " AL: " << hex2str(reg.AL) << endl;
    };
  }
  void OUTIbAL(uint8_t *p) {
    uint8_t &lv = *reinterpret_cast<uint8_t *>(p);
    switch (lv) {
      case 0x20:
      case 0x40:
      case 0x43:
      case 0xa0:
        break;
      default:
        LOG(FATAL) << "NotImplemented: out Ib, al - Ib: " << hex2str(lv)
                   << " AL: " << hex2str(reg.AL);
    };
    reg.IP += 1;
  }
  void POPw(uint16_t &lv) {
    lv = mem.get<uint16_t>(reg.SS, reg.SP);
    reg.SP += use_32bits_mode ? 4 : 2;
  }
  void POPb(uint8_t &lv) {
    // sign-extend the 8-bit immediate operand to 16-bit
    lv = mem.get<uint8_t>(reg.SS, reg.SP);
    reg.SP += use_32bits_mode ? 4 : 2;
  }
  void PUSHw(uint16_t &rv) {
    if (use_32bits_mode) {
      reg.SP -= 4;
      mem.get<uint16_t>(reg.SS, reg.SP) = rv;
      mem.get<uint16_t>(reg.SS, reg.SP + 2) = 0;
    } else {
      reg.SP -= 2;
      mem.get<uint16_t>(reg.SS, reg.SP) = rv;
    }
  }
  void PUSHb(uint8_t &rv) {
    // sign-extend the 8-bit immediate operand to 16-bit
    uint16_t rv16 = rv;
    PUSHw(rv16);
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
  void PUSHF() { PUSHw(reg.FR); }
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
  void POPF() { POPw(reg.FR); }
  void RET() {
    POPw(reg.IP);
    reg.IP -= 1;  // cancel +1 in the end
  }
  void IRET() {
    POPw(reg.IP);
    POPw(reg.CS);
    POPw(reg.FR);
    reg.IP -= 1;  // cancel +1 in the end
    // PrintState();
  }
  void OREbGb(uint8_t *p) {
    uint8_t *eb, *gb;
    _GetEvGv(p, eb, gb);
    *eb |= *gb;
    UpdateFlag(*eb);
    MemoryProtect(eb);
  }
  void OREvGv(uint8_t *p) {
    uint16_t *eb, *gb;
    _GetEvGv(p, eb, gb);
    *eb |= *gb;
    UpdateFlag(*eb);
    MemoryProtect(eb);
  }
  void ORGbEb(uint8_t *p) {
    uint8_t *eb, *gb;
    _GetEvGv(p, eb, gb);
    *gb |= *eb;
    UpdateFlag(*gb);
    MemoryProtect(gb);
  }
  void ORGvEv(uint8_t *p) {
    uint16_t *eb, *gb;
    _GetEvGv(p, eb, gb);
    *gb |= *eb;
    UpdateFlag(*gb);
    MemoryProtect(gb);
  }
  void ORALIb(uint8_t *p) {
    reg.AL |= *p;
    UpdateFlag(reg.AL);
    reg.IP += 1;
  }
  void OReAXIv(uint8_t *p) {
    uint16_t &rv = *reinterpret_cast<uint16_t *>(p);
    reg.AX |= rv;
    reg.IP += use_32bits_mode ? 4 : 2;
  }
  void REP(uint8_t *p) {
    CHECK(*p == 0xa5);  // movsw, [es:di] = [ds:si]
    void *dst = &mem.get<uint16_t>(get_addr(reg.ES, reg.DI));
    void *src = &mem.get<uint16_t>(get_addr(reg.DS, reg.SI));
    const int n = reg.CX * 2;
    memcpy(dst, src, n);
    reg.DI += n;
    reg.SI += n;
    reg.CX = 0;
    reg.IP += 1;
  }
  void SBBEvGv(uint8_t *p) {
    uint16_t *ev, *gv;
    _GetEvGv(p, ev, gv);
    *ev = _SUB<int16_t, uint16_t>(*ev, (*gv), reg.get_flag(Flag::CF));
    MemoryProtect(ev);
  }
  void SBBALIb(uint8_t *p) {
    // Acc, Imm
    uint8_t &lv = reg.AL;
    uint8_t &rv = *reinterpret_cast<uint8_t *>(p);
    lv = _SUB<int8_t, uint8_t>(lv, rv, reg.get_flag(Flag::CF));
    reg.IP += 1;
  }
  void SBBEbGb(uint8_t *p) {
    uint8_t *eb, *gb;
    _GetEvGv(p, eb, gb);
    *eb = _SUB<int8_t, uint8_t>(*eb, *gb, reg.get_flag(Flag::CF));
    MemoryProtect(eb);
  }
  void SBBGbEb(uint8_t *p) {
    uint8_t *eb, *gb;
    _GetEvGv(p, eb, gb);
    *gb = _SUB<int8_t, uint8_t>(*gb, *eb, reg.get_flag(Flag::CF));
    MemoryProtect(gb);
  }
  void SBBGvEv(uint8_t *p) {
    uint16_t *ev, *gv;
    _GetEvGv(p, ev, gv);
    *gv = _SUB<int16_t, uint16_t>(*gv, *ev, reg.get_flag(Flag::CF));
    MemoryProtect(gv);
  }
  void SHEb1(uint8_t *p) {
    ModRM &modrm = read_oosssmmm(p);
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
    ModRM &modrm = read_oosssmmm(p);
    uint8_t *ev, *iv;
    _GetEvIv(p, ev, iv);
    if (modrm.REG == 0b101) {
      (*ev) >>= (*iv);
    } else {
      CHECK(modrm.REG == 0b100);
      (*ev) <<= (*iv);
    }
    UpdateFlag(*ev);
    MemoryProtect(ev);
  }
  void SHEvIb(uint8_t *p) {
    ModRM &modrm = read_oosssmmm(p);
    uint16_t *ev;
    uint8_t *iv;
    _GetEvIv(p, ev, iv);
    uint16_t old_CX = reg.DX;
    switch (modrm.REG) {
      case 0b100:
        (*ev) <<= (*iv);
        break;
      case 0b101:
        (*ev) >>= (*iv);
        /*
        if (*iv == 0x4 && ev == &reg.DX) {
          cout << "NEW EV: " << hex2str(*ev) << " @@: " << hex2str(old_CX) <<
        endl; PrintState();
        }
        */
        break;
      case 0b111:
        (*ev) >>= 1;
        break;
      default:
        LOG(FATAL) << "Wrong modrm.REG: " << hex2str(modrm.REG);
    };
    UpdateFlag(*ev);
    MemoryProtect(ev);
  }
  void STI() { reg.set_flag(Flag::IF); }
  void SUBEbGb(uint8_t *p) {
    uint8_t *eb, *gb;
    _GetEvGv(p, eb, gb);
    *eb = _SUB<int8_t, uint8_t>(*eb, *gb);
    MemoryProtect(eb);
  }
  void SUBEvGv(uint8_t *p) {
    uint16_t *ev, *gv;
    _GetEvGv(p, ev, gv);
    *ev = _SUB<int16_t, uint16_t>(*ev, *gv);
    MemoryProtect(ev);
  }
  void SUBGbEb(uint8_t *p) {
    uint8_t *eb, *gb;
    _GetEvGv(p, eb, gb);
    *gb = _SUB<int8_t, uint8_t>(*gb, *eb);
    MemoryProtect(gb);
  }
  void SUBGvEv(uint8_t *p) {
    uint16_t *ev, *gv;
    _GetEvGv(p, ev, gv);
    *gv = _SUB<int16_t, uint16_t>(*gv, *ev);
    MemoryProtect(gv);
  }
  void SUBeAXIv(uint8_t *p) {
    uint16_t &rv = *reinterpret_cast<uint16_t *>(p);
    reg.AX = _SUB<int16_t, uint16_t>(reg.AX, rv);
    reg.IP += use_32bits_mode ? 4 : 2;
  }
  void SUBALIb(uint8_t *p) {
    // Acc, Imm
    uint8_t &lv = reg.AL;
    uint8_t &rv = *reinterpret_cast<uint8_t *>(p);
    lv = _SUB<int8_t, uint8_t>(lv, rv);
    reg.IP += 1;
  }
  void TESTEbGb(uint8_t *p) {
    uint8_t *eb, *gb;
    _GetEvGv(p, eb, gb);
    UpdateFlag((*eb) & (*gb));
  }
  void TESTEvGv(uint8_t *p) {
    uint16_t *ev, *gv;
    _GetEvGv(p, ev, gv);
    UpdateFlag((*ev) & (*gv));
  }
  void _MOVZXw(uint8_t *p) {
    uint16_t *ev, *gv;
    _GetEvGv(p, ev, gv);
    *ev = *gv;
    MemoryProtect(ev);
  }
  void _MOVZXb(uint8_t *p) {
    uint16_t *lv;
    uint8_t *rv;
    _GetEvGv<uint8_t, uint16_t>(p, rv, lv);
    *lv = *rv;
    MemoryProtect(lv);
  }
  void _JCNEAR(uint8_t *p) {
    if (reg.get_flag(Flag::CF)) _IPStep(*reinterpret_cast<uint16_t *>(p));
    reg.IP += 2;
  }
  void _JLNEAR(uint8_t *p) {
    // TOFIX
    if (!reg.get_flag(Flag::ZF) &&
        (reg.get_flag(Flag::SF) != reg.get_flag(Flag::OF)))
      _IPStep(*reinterpret_cast<uint16_t *>(p));
    reg.IP += 2;
  }
  void TWOBYTE(uint8_t *p) {
    // if (*p == 0xb7) cout << "AAA:" << hex2str(reg.IP) << endl;
    switch (*p) {
      case 0x82:
        _JCNEAR(p + 1);
        reg.IP += 1;
        break;
      case 0x84:
        if (reg.get_flag(Flag::ZF)) {
          _IPStep(*reinterpret_cast<uint16_t *>(p + 1));
        }
        reg.IP += 3;
        break;
      case 0x85:
        if (!reg.get_flag(Flag::ZF))
          _IPStep(*reinterpret_cast<uint16_t *>(p + 1));
        reg.IP += 3;
        break;
      case 0x8c:
        _JLNEAR(p + 1);
        reg.IP += 1;
        break;
      case 0x8e:
        if (reg.get_flag(Flag::ZF) || reg.get_flag(Flag::CF))
          _IPStep(*reinterpret_cast<uint16_t *>(p + 1));
        reg.IP += 3;
        break;
      case 0xb6:
        _MOVZXb(p + 1);
        reg.IP += 1;
        break;
      case 0xb7:
        _MOVZXw(p + 1);
        reg.IP += 1;
        break;
      default:
        LOG(FATAL) << "Wrong JZTWOBYTE: " << hex2str(*p);
    };
    // if (*p == 0xb7) cout << "BBB:" << hex2str(reg.IP) << endl;
  }
  void XOREbGb(uint8_t *p) {
    uint8_t *eb, *gb;
    _GetEvGv(p, eb, gb);
    *eb = (*eb) ^ (*gb);
    UpdateFlag(*eb);
    MemoryProtect(eb);
  }
  void XOREvGv(uint8_t *p) {
    uint16_t *ev, *gv;
    _GetEvGv(p, ev, gv);
    *ev = (*ev) ^ (*gv);
    UpdateFlag(*ev);
    MemoryProtect(ev);
  }
  void XORGbEb(uint8_t *p) {
    uint8_t *eb, *gb;
    _GetEvGv(p, eb, gb);
    *gb = (*gb) ^ (*eb);
    UpdateFlag(*gb);
    MemoryProtect(gb);
  }
  void XORGvEv(uint8_t *p) {
    uint16_t *ev, *gv;
    _GetEvGv(p, ev, gv);
    *gv = (*gv) ^ (*ev);
    UpdateFlag(*gv);
    MemoryProtect(gv);
  }
  void XORALIb(uint8_t *p) {
    uint8_t &lv = reg.AL;
    uint8_t &rv = *reinterpret_cast<uint8_t *>(p);
    lv ^= rv;
    UpdateFlag(lv);
    reg.IP += 1;
  }
  void XOReAXIv(uint8_t *p) {
    uint16_t &rv = *reinterpret_cast<uint16_t *>(p);
    reg.AX ^= rv;
    UpdateFlag(reg.AX);
    reg.IP += use_32bits_mode ? 4 : 2;
  }

 private:
  template <typename T, typename uT>
  inline void CheckOFandCF(int32_t val) {
    constexpr int32_t min_val = std::numeric_limits<T>::min();
    constexpr int32_t max_val = std::numeric_limits<T>::max();
    constexpr int32_t umin_val = std::numeric_limits<uT>::min();
    constexpr int32_t umax_val = std::numeric_limits<uT>::max();
    reg.set_flag(Flag::OF, (min_val > val || val > max_val));    // signed
    reg.set_flag(Flag::CF, (umin_val > val || val > umax_val));  // unsigned
  }
  template <typename T, typename uT>
  uT _ADD(uT dest, uT source, bool carry = false) {
    int32_t res = (int32_t)dest + (int32_t)source;
    if (carry) ++res;
    CheckOFandCF<T, uT>(res);
    UpdateFlag((uT)res);
    return res;
  }
  template <typename T, typename uT>
  uT _SUB(uT dest, uT source, bool borrow = false) {
    int32_t res = (int32_t)dest - (int32_t)source;
    if (borrow) --res;
    CheckOFandCF<T, uT>(res);
    UpdateFlag((uT)res);
    return res;
  }
  template <typename T, typename uT>
  uT _AND(uT dest, uT source) {
    uT res = dest & source;
    UpdateFlag(res);
    return res;
  }

 private:
  inline ModRM &read_oosssmmm(uint8_t *p) {
    // MOD, REG, RM
    // oo,  sss, mmm
    return *reinterpret_cast<ModRM *>(p);
  }

 private:
  template <typename T>
  void UpdateFlag(T v) {
    reg.set_flag(Flag::ZF, v == 0);
    reg.set_flag(Flag::SF, (v >> (sizeof(T) * 8 - 1)) & 1);  // the highest bit
    reg.set_flag(Flag::PF, num_1bits[v & 0xff] % 2);
  }
  uint16_t &GetSeg16(const uint8_t REG) {
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
  uint8_t &GetReg8(const uint8_t REG) {
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
  uint16_t &GetReg16(const uint8_t RM) {
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
  template <typename T>
  inline T &_GetReg(const uint8_t, DummyIdentity<T>);
  inline uint8_t &_GetReg(const uint8_t REG, DummyIdentity<uint8_t>) {
    return GetReg8(REG);
  }
  inline uint16_t &_GetReg(const uint8_t REG, DummyIdentity<uint16_t>) {
    return GetReg16(REG);
  }
  template <typename T>
  inline T &GetReg(const uint8_t REG) {
    return _GetReg(REG, DummyIdentity<T>());
  }
  void SetSegPrefix(uint16_t *reg) { pre_seg = reg; }
  void Set32bPrefix() { use_32bits_mode = true; }

 private:
  // memory
  void MemoryProtect(void *p, const string &msg = "") {
    return;
    const uint8_t *reg_p = static_cast<uint8_t *>((void *)&reg);
    if (p >= reg_p && p < reg_p + sizeof(Registers)) return;
    const uint8_t *mem_p = &mem.get<uint8_t>(0);
    if (!(p >= mem_p && p < mem_p + 0x100000)) {
      PrintHistory();
      PrintState();
      LOG(FATAL) << "Access Invalid Memory " << hex2str((uint8_t *)p - mem_p)
                 << msg;
    }
    // memory
    uint32_t addr = ((uint8_t *)p - mem_p);
    if (mem.is_protected(addr)) {
      PrintHistory();
      PrintState();
      LOG(FATAL) << "The protected memory " << hex2str(addr - base_addr)
                 << " is changed.";
    }
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
  bool KeyBoardINT() {
    bool zf = reg.get_flag(Flag::ZF);
    switch (reg.AH) {
      case 0x00:
        reg.AX = gui.get_key();
        break;
      case 0x01:
        if (gui.check_key()) {
          zf = false;
        } else {
          zf = true;
        }
        break;
      default:
        LOG(FATAL) << "NotImplemented KeyBoardINT: " << hex2str(reg.AH);
    };
    reg.set_flag(Flag::ZF, zf);
    return zf;
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
  void read_deasm(const string &line) {
    // address | opcode | note
    uint32_t addr = get_addr(reg.CS, reg.IP);
    int i;
    uint32_t code_addr = 0;
    for (i = 0; i < 8; ++i) {
      const char c = line[i];
      if (c == ' ') return;
      code_addr += hex2dec(c) * (1 << ((7 - i) * 4));
    }
    addr += code_addr;
    source_code[code_addr] = line;
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
  template <typename T>
  string hex2str(const T &c) {
    string res;
    int len = sizeof(T);
    const uint8_t *p = reinterpret_cast<const uint8_t *>(&c);
    for (int i = len - 1; i >= 0; --i) {
      uint8_t num = *(p + i);
      res += hexch[(num >> 4) & 0xF];
      res += hexch[num & 0xF];
    }
    return res;
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
    char *s = reinterpret_cast<char *>(&mem.get<uint16_t>(reg.CS, str_p));
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
    CHECK(fin) << filename;
    fin.seekg(0, fin.end);
    int length = fin.tellg();
    fin.seekg(0, fin.beg);
    char *ptr = reinterpret_cast<char *>(&mem.get(seg, offset));
    fin.read(ptr, length);
    RET();
  }
  void QQTUpdate() {
    Update();
    RET();
    uint8_t zero;
    POPb(zero);
    CHECK_EQ(zero, 0);
  }
  void QQTAI() {
    AI();
    RET();
    uint8_t zero;
    POPb(zero);
    CHECK_EQ(zero, 0);
  }

 private:
  void PrintHistory() {
    while (!history.empty()) {
      cout << history.front() << endl;
      history.pop();
    }
    cout << endl;
  }
  string GetCurrentState() {
    static Registers last_register;
    uint32_t addr = get_addr(reg.CS, reg.IP);
    uint32_t code_addr = addr - base_addr;
    auto p = source_code.find(code_addr);
    // if (p == source_code.end()) return {};
    string code_name = p != source_code.end() ? p->second : hex2str(code_addr);

    auto reg_state = [&](uint16_t value, uint16_t old) -> string {
      if (value == old) return hex2str(value);
      return "\033[31m" + hex2str(value) + "\033[0m";
    };

#define REG_STATE(name) reg_state(reg.name, last_register.name)

    stringstream ss;
    ss << hex2str(addr - 0x7e00) << " OP:" << code_name << endl
       << " AX:" << REG_STATE(AX) << " BX:" << REG_STATE(BX)
       << " CX:" << REG_STATE(CX) << " DX:" << REG_STATE(DX)
       << " SP:" << REG_STATE(SP) << " BP:" << REG_STATE(BP)
       << " SI:" << REG_STATE(SI) << " DI:" << REG_STATE(DI) << endl
       << " CS:" << REG_STATE(CS) << " DS:" << REG_STATE(DS)
       << " SS:" << REG_STATE(SS) << " ES:" << REG_STATE(ES)
       << " IP:" << REG_STATE(IP) << endl
       << " ZF:" << reg.get_flag(Flag::ZF) << " CF:" << reg.get_flag(Flag::CF)
       << endl;
    last_register = reg;
#undef REG_STATE
    return ss.str();
  }
  void PrintState() { cout << GetCurrentState() << endl; }

 private:
  uint16_t *pre_seg = nullptr;

 private:
  bool recording = false;
  Registers reg;
  Memory mem;
  SharedMemory shared_mem;
  unordered_map<uint32_t, string> source_code;
  const char hexch[16] = {'0', '1', '2', '3', '4', '5', '6', '7',
                          '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
  map<uint32_t, std::function<void()>> inject_functions;
  queue<uint8_t> key_buffer;
  time_point last_int08h_time;
  bool use_32bits_mode = false;
  queue<string> history;
  array<uint8_t, 256> num_1bits;
};
