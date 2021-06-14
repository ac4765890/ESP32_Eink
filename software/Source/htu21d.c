#include "common.h"
#include "i2c.h"

#define HTU_DEV_ADDR            0x80
#define HTU_REG_MEA_TEM         0xf3
#define HTU_REG_MEA_HUM         0xf5
#define HTU_REG_RESET           0xfe
#define HTU_REG_WRITE           0xe6
#define HTU_REG_READ            0xe7

I2C_STAUTS_TYPE htu21dRestart()
{
    return I2C_WRITE_BYTE(HTU_REG_RESET, HTU_DEV_ADDR);
}

I2C_STAUTS_TYPE htu21dMeasure(const BOOL isTem)
{
    char temp;
    temp = isTem ? HTU_REG_MEA_TEM : HTU_REG_MEA_HUM;
    return I2C_WRITE_BYTE(&temp, HTU_DEV_ADDR);
}

I2C_STAUTS_TYPE htu21dRead(const BOOL isTem, const BOOL isBlock, double *date)
{
    uint8_t temp[2];
    I2C_STAUTS_TYPE ack;
    do {
        ack = I2C_READ_STRING(temp, 2, HTU_DEV_ADDR);
        if (!isBlock && ack != I2C_OK)
            return ack;
        if (ack == I2C_OK){
            uint16_t re = ((uint16_t)temp[0] << 8) | ((uint16_t)temp[1] & 0x00fc);
            if (isTem)
                *date = re*(175.72 / 65536.0) - 46.85;
            else
                *date = re*(125.0 / 65536.0) - 6.0;
            break;
        }
    } while(isBlock && ack != I2C_OK);
    return ack;
}

I2C_STAUTS_TYPE htu21dWriteUserReg(uint8_t byte)
{
    return I2C_WRITE_BYTE(&byte, HTU_DEV_ADDR);
}

I2C_STAUTS_TYPE htu21dReadUserReg(uint8_t* recv)
{
    I2C_STAUTS_TYPE status;
    if ((status = I2C_WRITE_BYTE(HTU_REG_READ, HTU_DEV_ADDR)) != I2C_OK)
        I2C_READ_BYTE(recv, HTU_DEV_ADDR);
    return status;
}
