#pragma once
#include "const.h"
#include  <stdint.h> 

enum limit {
  HL = 0b10,
  LH = 0b01,
  HH = 0b11,
  LL = 0b00
};

enum message_code  : uint8_t{
  MC_LIMIT = 1,
  MC_ZERO = 2,
  MC_SEGDONE = 3,
  MC_POLL = 4,
};

typedef struct {
  uint8_t limit_states[N_AXES] = {0}; // 6 
  enum message_code code; // 1
  int8_t axis_index=0; // 0 means none; valid indexes are stored with +1, so index 0 is 1 // 1
  int32_t positions[N_AXES] = {0}; // 24
} EncoderReport;

