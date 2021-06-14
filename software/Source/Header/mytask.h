#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include "common.h"

extern void test();
extern void displayTask();
extern void i2cTask();
extern void configInter();

extern void initAll();
extern void displayRun();
extern void displayStart();
extern void displayEnd();

//vultr row, need to skip
extern void displayClearRect(const u16 x, u16 y, const u16 wight, const u16 hight, const u8 times);
extern void displayRect(const u16 x, u16 y, const u16 wight, const u16 hight, const u8 times);
extern void displayRectGray(const u16 x, u16 y, const u16 wight, const u16 hight, const u8 startGray, const u8 endGray);

extern void displayString(const u16 x, const u16 y, const u16 hight, char *string);
extern void displayMenu();

extern void imageReadLine(FILE *file, const u16 line);
extern void imageRaw2DisplayData(const u8 valve);
extern void imageDisplay(const u32 imageNumber, const char* rootPath);
#endif // !__DISPLAY_H__
