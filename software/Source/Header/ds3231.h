#ifndef __DS3231_H__
#define __DS3231_H__

#include "common.h"

extern void ds3231EnableINTCN(bool INT);
extern bool ds3231CheckINTCN();
extern bool ds3231CheckEOSC();
extern bool ds3231OscillatorCheck();
extern void ds3231Enable32kHz(bool TF);
extern void ds3231EnableOscillator(bool TF, bool battery, u8 frequency);
extern bool ds3231CheckIfAlarm(const bool isAlarm0) ;
extern bool ds3231checkAlarmEnabled(const bool isAlarm0);
extern void ds3231SetAlarmStatus(const bool on, const bool isAlarm0);
extern void ds3231GetTime(u8 *string);
extern void ds3231SetTime(u8 *string);
extern void ds3231SetClockMode(bool h12);

#endif // !__DS3231_H__