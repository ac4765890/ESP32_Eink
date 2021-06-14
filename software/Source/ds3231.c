#include "i2c.h"

#define ds3231Addr 0xD0

bool is12Model = false;

u8 ds3231DecToBcd(const u8 val) 
{
    return (u8)((val/10*16) + (val%10));
}

u8 ds3231BcdToDec(const u8 val) 
{
    return (u8)((val>>4)*10 + (val&0x0f));
}

u8 ds3231ReadControlByte(bool which) 
{
    u8 re;
    which ? i2cWriteByte(0x0f, ds3231Addr) : i2cWriteByte(0x0e, ds3231Addr);
    i2cReadByte(&re, ds3231Addr);
    
    return re;
}

void ds3231WriteControlByte(bool which, u8 control) 
{
    u8 temp[2] = {0, control};
    temp[0] = which ? 0x0f : 0x0e;
    i2cWriteString(temp, 2, ds3231Addr);
}

void ds3231EnableINTCN(bool INT)
{
    u8 temp_buffer = ds3231ReadControlByte(0);
    temp_buffer = INT ? (temp_buffer | 0x04) : (temp_buffer & 0xfb);

    ds3231WriteControlByte(temp_buffer, 0);
}

bool ds3231CheckINTCN()
{
    u8 temp_buffer = ds3231ReadControlByte(0);
    return (temp_buffer & 0x04) ? false : true;
}

bool ds3231CheckEOSC()
{
    u8 temp_buffer = ds3231ReadControlByte(0);
    return (temp_buffer & 0xf0) ? false : true;
}

bool ds3231OscillatorCheck()
{
    u8 temp_buffer = ds3231ReadControlByte(1);
    return (temp_buffer & 0xf0) ? false : true;
}

void ds3231Enable32kHz(bool TF)
{
    u8 temp_buffer = ds3231ReadControlByte(1);
    temp_buffer = TF ? (temp_buffer | 0x0f) : (temp_buffer & 0xf7);
    ds3231WriteControlByte(temp_buffer, 1);
}

void ds3231EnableOscillator(bool TF, bool battery, u8 frequency)
{
    if (frequency > 3) frequency = 3; 
    
    u8 temp_buffer = ds3231ReadControlByte(0) & 0xe7;
    temp_buffer = battery ? (temp_buffer | 0x40) : (temp_buffer & 0xbf);
    temp_buffer = TF ? (temp_buffer & 0xfb) : (temp_buffer | 0xf0);
    
    frequency <<= 3;
    temp_buffer |= frequency;
    
    ds3231WriteControlByte(temp_buffer, 0);
}

bool ds3231CheckIfAlarm(const bool isAlarm0) 
{
    u8 temp_buffer = ds3231ReadControlByte(1);
    u8 re = isAlarm0 ? (temp_buffer & 0x02) : (temp_buffer & 0x01);
    temp_buffer &= isAlarm0 ? 0xfd : 0xfe;

    ds3231WriteControlByte(temp_buffer, 1);
    return re;
}

bool ds3231checkAlarmEnabled(const bool isAlarm0)
{
	u8 temp_buffer = ds3231ReadControlByte(0);
	u8 result = isAlarm0 ? temp_buffer & 0x01 : temp_buffer & 0x02;
	return result;
}

void ds3231SetAlarmStatus(const bool on, const bool isAlarm0)
{
    u8 temp_buffer = ds3231ReadControlByte(0);
    if(on){
        temp_buffer |= isAlarm0 ? 0x06 : 0x05;
    } else {
        temp_buffer &= isAlarm0 ? 0xfd : 0xfe;
    }
    
    ds3231WriteControlByte(temp_buffer, 0);
}

void ds3231GetTime(u8 *string)
{
    i2cWriteByte(0x00, ds3231Addr);
    i2cReadString(string, 7, ds3231Addr);

    string[2] &= (string[2] & 0x40) ? 0x1f : 0x3f;
    string[5] &= 0x7f;
    for (u8 i = 0; i < 7; i++)
        string[i] = ds3231BcdToDec(string[i]);
}

void ds3231SetTime(u8 *string) // s m h dw d mo y
{
    u8 temp_buffer;
    bool h12;

    i2cWriteByte(0x02, ds3231Addr);
    i2cReadByte(&temp_buffer, ds3231Addr);
    h12 = temp_buffer & 0x40;

    if (h12){
        if (string[2] > 12){
            string[2] = ds3231DecToBcd(string[2] - 0x0c) | 0x60;
        } else {
            string[2] = ds3231DecToBcd(string[2]) & 0xdf;
        }
    } else {
        string[2] = ds3231DecToBcd(string[2]) & 0xbf;
    }

    i2cWriteByte(0x00, ds3231Addr);
    temp_buffer = string[2];
    for (u8 i = 0; i < 7; i++)
        string[i] = ds3231DecToBcd(string[i]);
    string[2] = temp_buffer;
    i2cWriteString(string, 7, ds3231Addr);
    
    temp_buffer = ds3231ReadControlByte(1);
    ds3231WriteControlByte((temp_buffer & 0x7f), 1);
}

void ds3231SetClockMode(bool h12)
{   
    u8 temp_buffer;
    is12Model = !h12;
    i2cWriteByte(0x02, ds3231Addr);
    i2cReadByte(&temp_buffer, ds3231Addr);

    
    if (h12){
        temp_buffer = temp_buffer | 0x40;
    } else {
        temp_buffer = temp_buffer & 0xbf;
    }

    u8 temp[2] = {0x02, temp_buffer};
    i2cWriteString(temp, 2, ds3231Addr);
}