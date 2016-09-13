#include "stdafx.h"

PARALLEL_SUFFIX_MOVE_MASK32
ParallelSuffix32HighBitFromEveryOtherByte = {
    0xaaaaaaaa, //  Mask    (0b10101010101010101010101010101010)
    0x22222222, //  Move0   (0b00100010001000100010001000100010)
    0x18181818, //  Move1   (0b00011000000110000001100000011000)
    0x07800780, //  Move2   (0b00000111100000000000011110000000)
    0x007f8000, //  Move3   (0b00000000011111111000000000000000)
    0x80000000, //  Move4   (0b10000000000000000000000000000000)
    0xffffffff, //  Unused5
    0x00000000  //  Unused6
};

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
