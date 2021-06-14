#ifndef __PCA9551_H__
#define __PCA9551_H__

#include "common.h"

extern void pca9551SetRegAddr(const u8 _isAI, const u8 regAddr);
extern void pca9551SetPcs(const bool is0, const u8 freq);
extern void pca9551SetPwm(const bool is0, const u8 pwm);
extern void pca9551SetLedMask(const bool is0, const u8 mask);
extern void pca9551SetResPin(const u8 pin);
extern void pca9551Reset();

#endif // !__PCA9551_H__
