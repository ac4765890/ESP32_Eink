#include "3rd/arduinoEsp32/Header/esp32-hal-gpio.h"
#include "3rd/arduinoEsp32/Header/SPI.h"
#include "Header/common.h"
#include "rom/ets_sys.h"

#define negAddr     0
#define posAddr     1
#define shrAddr     2
#define rlAddr      3
#define mdAddr      4

#define myDelay()                           ets_delay_us(1)
#define setBit(buff, x, status)             (buff = ((buff & ~(1 << x)) | (status << x)))
#define cmdline(addr, status)               digitalWrite(addr, status)
#define shiftRegCmdLine(addr, status)       setBit(shiftRegBuff, addr, status); \
                                            shiftRegSendByte(shiftRegSTCP2, shiftRegBuff)

#define gateClk()   digitalWrite(einkCkv, Low);    \
                    ets_delay_us(1);    \
                    digitalWrite(einkCkv, High)

#define sourceClk()     digitalWrite(einkCkh, High);    \
                        digitalWrite(einkCkh, Low)

#define spvSet(status)      cmdline(einkSpv, status)
#define leSet(status)       cmdline(einkLE, status)
#define oeSet(status)       cmdline(einkOE, status)
#define sphSet(status)      cmdline(einkSph, status)

#define dataSet(status)     shiftRegSendByte(shiftRegSTCP1, status)
#define negSet(status)      shiftRegCmdLine(negAddr, status)
#define posSet(status)      shiftRegCmdLine(posAddr, status)
#define rlSet(status)       shiftRegCmdLine(rlAddr, status)
#define shrSet(status)      shiftRegCmdLine(shrAddr, status)
#define gmodeSet(status)    shiftRegCmdLine(mdAddr, status)
#define pwcSet(status)      negSet(status);     \
                            posSet(status)

static const char *tag = "EPD Model";
//buff for reg 2, connect to power control, eink_md and so on
u8 shiftRegBuff = 0;

void einkPowerUp();
void einkPowerDown();

void shiftRegSendByte(const u8 chipSelectPin, const u8 data)
{
    digitalWrite(shiftRegSHCP, 0);
    digitalWrite(chipSelectPin, 0);
    digitalWrite(shiftRegDS, 0);

    u8 mask = 0x80;
    for (int i = 0; i < 8; i++){
        digitalWrite(shiftRegSHCP, Low);
        //myDelay();
        digitalWrite(shiftRegDS, mask & data);
        //myDelay();
        digitalWrite(shiftRegSHCP, High);
        //myDelay();
        mask >>= 1;
    }

    digitalWrite(chipSelectPin, High);
    //myDelay();
    digitalWrite(chipSelectPin, Low);
}

void einkInit()
{
    //shift reg pin init
    pinMode(shiftRegSHCP, OUTPUT);
    pinMode(shiftRegSTCP1, OUTPUT);
    pinMode(shiftRegSTCP2, OUTPUT);
    pinMode(shiftRegDS, OUTPUT);

    //eink io init
    pinMode(einkCkv, OUTPUT);
    pinMode(einkCkh, OUTPUT);
    pinMode(einkSpv, OUTPUT);
    pinMode(einkOE, OUTPUT);
    pinMode(einkLE, OUTPUT);
    pinMode(einkSph, OUTPUT);
}

void einkResetPinStatus()
{
    //ESP_LOGD(tag, "initPinStatus() -> now init pin status");
    //gate
    gmodeSet(Low);
    digitalWrite(einkCkv, Low);
    spvSet(High);

    //source
    digitalWrite(einkCkh, Low);
    leSet(Low);
    oeSet(High);
    sphSet(High);

    //unknow
    shrSet(Low);
    rlSet(High);
}

void einkSendRowData(const u8* data, const short rowCount)
{
    //ESP_LOGD(tag, "einkSendRowData() -> send data, rowCount%d", rowCount);
    sphSet(Low);
    
    for (int i = 0; i < 200; i++){
        if (i && (data[i] == data[i-1])){
            sourceClk();
        }else{
            dataSet(data[i]);
            sourceClk();
        }
    }

    sphSet(High);
    sourceClk();

    leSet(High);
    sourceClk();

    //ets_delay_us(7);

    leSet(Low);
    sourceClk();

    for (int i = 0; i < rowCount; i++){
        oeSet(High);
        sphSet(Low);

        ets_delay_us(2);

        sphSet(High);
        sourceClk();

        digitalWrite(einkCkv, Low);
        oeSet(Low);
        sourceClk();

        ets_delay_us(2);

        digitalWrite(einkCkv, High);
    }
}

void einkSendRowDataGray(const u8* data, const u32 highTime, const u32 lowTime, const short rowCount)
{
    //oeSet(High);
    sphSet(Low);
    
    for (int i = 0; i < 200; i++){
        if (i && (data[i] == data[i-1])){
            sourceClk();
        }else{
            dataSet(data[i]);
            sourceClk();
        }
    }

    sphSet(High);
    sourceClk();

    leSet(Low);
    sourceClk();

    leSet(High);
    sourceClk();

    for (int i = 0; i < rowCount; i++){
        oeSet(High);
        sphSet(Low);

        sphSet(High);
        sourceClk();

        digitalWrite(einkCkv, High);
        ets_delay_us(highTime*1000);
        digitalWrite(einkCkv, Low);
        ets_delay_us(lowTime*1000);

        oeSet(Low);
        sourceClk();
    }
}

void einkSkipRow(const short rowCount)
{
    //ESP_LOGD(tag, "einkSendRowData() -> skip row, rowCount%d", rowCount);
    if (!rowCount)
        return;

    sphSet(Low);
    
    dataSet(0x00);
    for (int i = 0; i < 200; i++){
        sourceClk();
    }

    sphSet(High);
    sourceClk();

    leSet(High);
    sourceClk();

    //ets_delay_us(7);

    leSet(Low);
    sourceClk();

    for (int i = 0; i < rowCount; i++){
        sphSet(Low);

        ets_delay_us(2);

        sphSet(High);
        sourceClk();

        digitalWrite(einkCkv, Low);
        sourceClk();

        ets_delay_us(2);

        digitalWrite(einkCkv, High);
    }
}

void einkStartFrame()
{
    shrSet(High);
    rlSet(High);

    gmodeSet(High);

    spvSet(High);
    gateClk();

    spvSet(Low);
    gateClk();
    
    spvSet(High);
    gateClk();
}

void einkEndFrame()
{
    gmodeSet(Low);
    gateClk();
}

//       100us        0us      1000us        0us
//poweron   ->  vneg   ->   vee   ->   vpos   ->   vgg
// +3.3          -15        -20        +15         +22
void einkPowerUp()
{
    //boot vneg and vee
    ets_delay_us(100);
    negSet(High);
    //boot vpos and vgg
    ets_delay_us(1000);
    posSet(High);

    ets_delay_us(1000);
}

//all 0us
//vgg   ->   vpos   ->   vee   ->   vneg   ->   powerdown
//+22         +15        -20        -15         +3.3
void einkPowerDown()
{
    pwcSet(Low);
}