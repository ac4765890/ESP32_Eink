#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include "esp_err.h"
#include "esp_log.h"

#define u8          unsigned char
#define u16         unsigned short
#define u32         unsigned int
#define LSBFIRST    0
#define MSBFIRST    1
#define ACK         0x00
#define NACK        0x01
#define Write       0x00
#define Read        0x01
#define Low         0x00
#define High        0x01

#define shiftRegSHCP    5
#define shiftRegDS      18
#define shiftRegSTCP1   17
#define shiftRegSTCP2   16
#define i2cScl          21
#define i2cSda          22
#define einkCkv         26
#define einkCkh         25
#define einkSpv         32
#define einkOE          33
#define einkLE          27
#define einkSph         23
#define sdD0            2
#define sdD1            4
#define sdD2            12
#define sdD3            13
#define sdCmd           15
#define interSqwMask    (1ULL<<35)
#define interSqwPin     35

#define pixel8      16
#define pixel12     24
#define pixel24     48

#endif // !__COMMON_H__
