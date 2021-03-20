#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>
#include <string>

const std::unordered_map<std::string, uint16_t> SYMBOLS = {
  {"DRAW", 0x52f},
  {"DrawPlayer", 0x00000466},
  {"BombIsPassed", 0x00000C0F},
  {"BombIsPassedEnd", 0x00000C36},
  {"PlayerIsPassed", 0x00000C3B},
  {"PlayerIsPassedRtn", 0x00000C6E},
  {"DrawSegment", 0x00000F7E},
  {"DrawRectW", 0x00000F80},
  {"DrawRectH", 0x00000F82},
  {"PLAYER_X", 0x00000F8A},
  {"PLAYER_Y", 0x00000F8C},

  // extern variables
  {"Players", 0x00000F84},
  {"PASSED_DATA", 0xab6d},
  {"STATE_DATA", 0xac71 - 0x7e00},
  {"Bombs", 0x8da8 - 0x7e00},
  {"PlayerHP", 0xa8f5 - 0x7e00},
  {"BossHP", 0xa8f6 - 0x7e00},
  {"PlayerNOHARM", 0xa8f7 - 0x7e00},
  {"BossNOHARM", 0xa8f9 - 0x7e00},
};
