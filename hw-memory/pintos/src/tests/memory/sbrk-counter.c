/* Tests asymmetric allocations and deallocations with sbrk. */

#include <string.h>
#include "tests/lib.h"
#include "tests/main.h"

#define INITIAL_OFFSET 1000000
#define NUM_DELTAS 301
static const intptr_t deltas[NUM_DELTAS] = {
    INITIAL_OFFSET, -81722, -61013, 5768,   -70351, -82053, -42749, 23499,  59617,  72099,  90138,
    80342,          37103,  35989,  -20452, 96959,  45415,  -38548, 47055,  -99920, 6764,   -85394,
    -98578,         906,    -14773, -85641, 31199,  -83810, -59906, 67326,  -41543, 60662,  27363,
    36383,          71396,  91224,  76149,  -99074, -67785, 67936,  -22349, -52350, 74301,  82989,
    -25696,         -96465, 27719,  -40138, -97016, 62843,  -75202, 29769,  699,    98211,  21920,
    63650,          -9626,  80,     24825,  -71337, 17948,  -96641, 26493,  38058,  -45303, 34839,
    84805,          -97293, -33828, -54255, -71870, 9098,   11633,  86955,  62789,  -72063, 38412,
    91306,          83205,  619,    -28270, 27303,  -8555,  24594,  -4856,  -47729, -70702, -91761,
    43213,          -72311, -25740, -45786, -27024, -49180, 93891,  -6714,  8747,   -11818, -57612,
    10624,          -9938,  16554,  -41287, 46513,  92363,  -24759, -52179, -51701, -28788, -25271,
    38379,          -33809, 6060,   -51784, 43892,  67731,  81756,  48834,  85620,  -77579, -93315,
    -82163,         23101,  40318,  -35941, -52026, -12453, 10014,  38286,  -72093, 91106,  1116,
    -33759,         40114,  73763,  26516,  -63048, 36864,  88934,  -5277,  -89336, 22688,  -81205,
    -54930,         -70648, -4435,  -55425, -47050, -95165, -48712, -85846, -40568, 49775,  24575,
    -15947,         60581,  -62598, -19243, -22361, -12455, -44525, -19926, -80651, 2151,   59153,
    -64236,         23956,  46502,  55923,  -96898, 65728,  16912,  40460,  74453,  -11543, -90866,
    -64510,         -40144, 13891,  -73304, -61371, 58677,  50283,  -34582, 79233,  93982,  -84483,
    35233,          48419,  16117,  -60717, 98501,  -26481, -73714, 14390,  -70416, -64749, 75120,
    18687,          66932,  78997,  -77102, 95414,  -91043, -52036, -34024, 48732,  -76989, 40517,
    -21728,         44064,  82740,  45922,  86606,  24623,  97468,  90333,  57758,  -57430, -12701,
    97888,          -3612,  33134,  -6713,  -31990, 42720,  68182,  -46922, -24847, -82664, -86457,
    6196,           -94472, -3153,  45314,  -27354, -46580, -55297, 66350,  4324,   79982,  8842,
    -5802,          -73555, -85114, 87131,  -32961, 52591,  74920,  52839,  45393,  -96968, 26261,
    12254,          -23021, -80983, -82077, 61844,  -37668, -17742, -87889, 37959,  86997,  99375,
    13171,          96492,  83750,  65203,  -35810, 89834,  49404,  -66497, -66033, 19406,  -22173,
    8300,           -90341, -7229,  53318,  56240,  76883,  54361,  -62987, 32332,  53455,  -30722,
    67129,          55595,  -89371, 65459,  97615,  9040,   -16184, 43814,  -87583, 55179,  84930,
    -63016,         -82693, 73646,  -44881};

static void test_delta(unsigned char* base, intptr_t delta, size_t expected_total) {
  unsigned char* brk = sbrk(delta);
  ASSERT(brk != (void*)-1);
  ASSERT(((intptr_t)expected_total - delta) == (brk - base));

  /* Make sure that the necessary memory is allocated. */
  memset(base, 162, expected_total);
  for (size_t i = 0; i != expected_total; i++) {
    ASSERT(base[i] == 162);
  }
}

void test_main(void) {
  unsigned char* heap = sbrk(0);
  int offset = 0;
  for (int i = 0; i != NUM_DELTAS; i++) {
    offset += deltas[i];
    ASSERT(offset >= 0);
    test_delta(heap, deltas[i], (size_t)offset);
  }
  msg("Done counting; about to access memory out of bounds");
  msg("Accessed out-of-bounds memory: %d", (int)heap[offset + 4096]);
}

int main(int argc UNUSED, char* argv[] UNUSED) {
  test_name = "sbrk-counter";
  msg("begin");
  test_main();
  msg("end");
  return 0;
}