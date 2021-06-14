#ifndef __I2CMASTER_H__
#define __I2CMASTER_H__

#include "common.h"

//run once
extern void i2cInit(const u8 port, const u8 sda, const u8 scl, const u32 freq);    //freq use HZ
//cmd line
extern esp_err_t i2cReadByte(u8* byte, const u8 addr);
extern esp_err_t i2cReadString(u8* string, const u16 size, const u8 addr);
extern esp_err_t i2cWriteByte(const u8 byte, const u8 addr);
extern esp_err_t i2cWriteString(u8* string, const u16 size, const u8 addr);
//one step function
extern esp_err_t i2cStart();
extern esp_err_t i2cRecvByte(u8* byte, const bool isNack);
extern esp_err_t i2cSendByte(u8* byte, const bool isCheckNack);
extern esp_err_t i2cStop();

#endif // !__I2CMASTER_H__
