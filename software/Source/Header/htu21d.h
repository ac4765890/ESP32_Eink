#ifndef __HTU21D_H__
#define __HTU21D_H__

#include "common.h"

extern I2C_STAUTS_TYPE htu21dRestart();
extern I2C_STAUTS_TYPE htu21dMeasure(const BOOL isTem);
extern I2C_STAUTS_TYPE htu21dRead(const BOOL isTem, const BOOL isBlock, double *date);
extern I2C_STAUTS_TYPE htu21dWriteUserReg(uint8_t byte);
extern I2C_STAUTS_TYPE htu21dReadUserReg(uint8_t* recv);

#endif // !__HTU21D_H__
