/* 
  SPI.h - SPI library for esp8266

  Copyright (c) 2015 Hristo Gochkov. All rights reserved.
  This file is part of the esp8266 core for Arduino environment.
 
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#ifndef _SPI_H_INCLUDED
#define _SPI_H_INCLUDED

#include "esp32-hal-spi.h"

typedef struct {
    uint32_t _clock;
    uint8_t  _bitOrder;
    uint8_t  _dataMode;
}SPISettings;

extern void eSpiBegin(int8_t sck, int8_t miso, int8_t mosi, int8_t ss);
extern void eSpiEnd();
extern void eSpiSetHwCs(bool use);
extern void eSpiSetBitOrder(uint8_t bitOrder);
extern void eSpiSetDataMode(uint8_t dataMode);
extern void eSpiSetFrequency(uint32_t freq);
extern void eSpiSetClockDivider(uint32_t clockDiv);
extern uint32_t eSpiGetClockDivider();
extern void eSpiBeginTransaction(SPISettings settings);
extern void eSpiEndTransaction(void);
extern uint8_t eSpiTransfer(uint8_t data);
extern uint16_t eSpiTransfer16(uint16_t data);
extern uint32_t eSpiTransfer32(uint32_t data);
extern void eSpiTransferBytes(uint8_t * data, uint8_t * out, uint32_t size);
extern void eSpiTransferBits(uint32_t data, uint32_t * out, uint8_t bits);
extern void eSpiWrite(uint8_t data);
extern void eSpiWrite16(uint16_t data);
extern void eSpiWrite32(uint32_t data);
extern void eSpiWriteBytes(uint8_t * data, uint32_t size);
extern void eSpiWritePixels(const void * data, uint32_t size);//ili9341 compatible
extern void eSpiWritePattern(uint8_t * data, uint8_t size, uint32_t repeat);
extern spi_t * eSpiBus();

#endif
