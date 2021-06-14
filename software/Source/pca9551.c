#include "./Header/common.h"
#include "./Header/i2cmaster.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define pca9551Addr         0xC0
#define pca9551AI           0x10
#define pca9551UnAI         0xEF
#define pca9551RegInput     0x00
#define pca9551RegPsc0      0x01
#define pca9551RegPwm0      0x02
#define pca9551RegPsc1      0x03
#define pca9551RegPwm1      0x04
#define pca9551RegLs0       0x05
#define pca9551RegLs1       0x06

static bool isAI = false;
static u8 gpio;

void pca9551SetResPin(const u8 pin)
{
    gpio = pin-1;
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = pin-1;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;

    gpio_config(&io_conf);
    gpio_set_level(gpio, Low);
}

void pca9551Reset()
{
    gpio_set_level(gpio, High);
    vTaskDelay(pdMS_TO_TICKS(1));
    gpio_set_level(gpio, Low);
}

void pca9551SetRegAddr(const u8 _isAI, const u8 regAddr)
{
    isAI = _isAI ? true : false;
    i2cWriteByte((isAI ? pca9551AI : 0) | regAddr, pca9551Addr);
}

void pca9551SetPcs(const bool is0, const u8 freq)
{
    i2cWriteByte((is0 ? pca9551RegPsc0 : pca9551RegPsc1)|(isAI ? pca9551AI : 0), pca9551Addr);
    i2cWriteByte(freq, pca9551Addr);
}

void pca9551SetPwm(const bool is0, const u8 pwm)
{
    i2cWriteByte((is0 ? pca9551RegPwm0 : pca9551RegPsc1)|(isAI ? pca9551AI : 0), pca9551Addr);
    i2cWriteByte(pwm, pca9551Addr);
}

void pca9551SetLedMask(const bool is0, const u8 mask)
{
    i2cWriteByte((is0 ? pca9551RegLs0 : pca9551RegLs1)|(isAI ? pca9551AI : 0), pca9551Addr);
    i2cWriteByte(mask, pca9551Addr); 
}