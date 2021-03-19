#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>
#include <string>

const std::unordered_map<std::string, uint16_t> SYMBOLS = {
  {"DRAW", 0x52f},
  {"DrawPlayer", 0x00000466},
  {"DrawSegment", 0x00000F7E},
  {"DrawRectW", 0x00000F80},
  {"DrawRectH", 0x00000F82},
  {"PLAYER_X", 0x00000F8A},
  {"PLAYER_Y", 0x00000F8C},
};
