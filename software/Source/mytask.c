#include "Header/common.h"
#include "Header/eink.h"
#include "Header/sdio.h"
#include "Header/font.h"
#include "Header/ds3231.h"
#include "Header/htu21d.h"
#include "Header/pca9551.h"
#include "Header/i2cmaster.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_intr_alloc.h"
#include "3rd/arduinoEsp32/Header/esp32-hal-gpio.h"
#include "esp_task_wdt.h"

#include <string.h>
#include <time.h>
#include <stdlib.h>

#define linePixelCount      800
#define pixelDepth          4
#define flagPicture         0
#define flagTime        1
#define flagData        2
#define flagTemp        3
#define flagHum         4
#define flagCal         5

extern TaskHandle_t i2cHandle;
extern TaskHandle_t displayHandle;
portMUX_TYPE mmux = portMUX_INITIALIZER_UNLOCKED;

u8 lineBuff[400];
u8 displayBuff[200];
u8 grayClass[16];
u8 firstRun = true;

const unsigned char timA[16] =
{
// 1  2  3  4  5  6  7  8  9 10 11  12  13  14  15
   2, 2, 3, 3, 4, 4 ,4, 5, 5, 6, 6,  8, 8, 10, 14, 18
};

u32 ebmFileCount;
gpio_config_t io;

//picture   time    date    temp    hum     cal
u8 displayFlag[] = {1, 1, 1, 1, 1, 0};  
char dataTime[] = "12:34";     //xx:xx, 38,680
char dataDate[] = "2026-08-17";    //xxxx-xx-xx
char dataTemp[] = " 23";     //xx℃
char dataHum[] = " 50%";      //xx%
char dataCal[] = {0, 0, 0};      //start day data
u16 year = 2000;
u8 mon = 12;

static const char *tag = "myTask Model";

void i2cTask();

//----------assist function----------//

void initRandom()
{
    if (firstRun){
        firstRun = false;
        srand(time(NULL)); 
    }
} 

u32 getRamdom(const u32 max)
{
    return rand() % max;
}   

u16 grayTimeCount(const u8 startGray, const u8 endGray)
{
    u16 count = 0;
    for (int i = startGray; i < endGray; i++){
        count += timA[i];
    }
    return count;
}

void reverseString(char *string)
{
    memset(lineBuff, 0x00, 200);
    const u8 length = strlen(string);
    ESP_LOGD(tag, "row data: %s", string);
    for (u8 i = 0; i < length; i++){
        lineBuff[length-i-1] = string[i];
    }
}

int square(const int num, const u8 times)
{
    if (times == 0)
        return 1;

    int temp = num;
    for (int i = 1; i < times; i++){
        temp *= num;
    }
    return temp;
}

void double2string(double data, char *string, u8 size)
{
    if (data <= -1E-7)
        string[0] = '-';
    else
        string[0] = ' ';
    int temp = data;
    for (int i = 0; i < size; i++){
        string[i+1] = (temp / square(10, size-i-1))%10 + 0x30;
        //ESP_LOGD(tag, "double2string: %c", string[i]);
    }
}

void int2string(char data, char* string, u8 size)
{
    ESP_LOGD(tag, "int2String rowData: %d", data);
    int i = 0;
    if (data < 10){
        string[0] = '0';
        i++;
    }
    for (; i < size; i++){
        string[i] = (data / square(10, size-i-1))%10 + 0x30;
        ESP_LOGD(tag, "int2string: %c", string[i]);
    }
}

void ds3231RowData2DisplayData(u8 *string)
{
    //s min h day date mo y
    int2string(string[6], dataDate+2, 2);
    int2string(string[5], dataDate+5, 2);
    int2string(string[4], dataDate+8, 2);
    dataCal[1] = string[3];
    dataCal[2] = string[4];
    int2string(string[2], dataTime, 2);
    int2string(string[1], dataTime+3, 2);

    dataCal[0] = (string[3]-(string[4]%7))%7+1;
    year = 2000+string[6];
    mon = string[5];
}

bool checkLeapYear(u16 year)
{
    return (!(year%4) && (year%100)) || !(year%400);
}

u8 getMonDay(bool isLeapYear, u8 month)
{
    switch (month){
    case 1:case 3:case 5:case 7:case 8:case 10:case 12:
        return 31;
    case 2:
        if(isLeapYear)
            return 29;
        else
            return 28;
    default:
        return 30;
    }
}

void creatCal(u8 lineCount, u8 start)
{
    memset(lineBuff, 0x00, 18*2);
    for (int i = 0; i < lineCount; i++){
        if (start < 10){
            lineBuff[i++] = start++ + 0x30;
            lineBuff[i] = '0';
        } else {
            lineBuff[i++]= start/10 + 0x30;
            lineBuff[i] = start++%10 + 0x30;
        }
    }
}
//------------------------------------//

void displayStart()
{
    einkPowerUp();
}

void displayEnd()
{
    einkPowerDown();
}

void displayClearRect(const u16 x, u16 y, const u16 wight, const u16 hight, const u8 times)
{
    y += 2;
    memset(displayBuff, 0xFF, 200);
    memset(displayBuff+x/4, 0xAA, wight/4);
    for (int i = 0; i < times; i++){
        einkResetPinStatus();
        einkStartFrame();
        einkSkipRow(y);
        einkSendRowData(displayBuff, hight);
        einkEndFrame();
    }
}

void displayRect(const u16 x, u16 y, const u16 wight, const u16 hight, const u8 times)
{
    y += 2;
    memset(displayBuff, 0xFF, 200);
    memset(displayBuff+x/4, 0x55, wight/4);
   
    for (int i = 0; i < times; i++){
        einkResetPinStatus();
        einkStartFrame();
        einkSkipRow(y);
        einkSendRowData(displayBuff, hight);
        einkEndFrame();
    }
}

void displayRectGray(const u16 x, u16 y, const u16 wight, const u16 hight, const u8 startGray, const u8 endGray)
{
    y += 2;
    einkStartFrame();
    einkSkipRow(y);
    einkSendRowDataGray(displayBuff, grayTimeCount(startGray, endGray), 0, hight);
    einkEndFrame();
}

void displayString(const u16 x, const u16 y, const u16 hight, char *string)
{
    reverseString(string);
    const u8 startByte = x/4;
    const u8 stringLength = strlen(string);
    const u8 charWight = (hight == pixel8) ? 10 : (hight == pixel12 ? 16 : 26);
    const u8 charRowByteCount = (hight == pixel8) ? 1 : (hight == pixel12 ? 2 : 3);
    const u8 charByteCount = (hight == pixel8) ? 16 : (hight == pixel12 ? 48 : 144);
    const u8 *font = (hight == pixel8) ? Font_Ascii_8X16E : (hight == pixel12 ? Font_Ascii_12X24E : Font_Ascii_24X48E);

    displayStart();
    for (char i = 0; i < 2; i++){
        displayRect(x, y, stringLength*charWight, hight, 64);
        displayClearRect(x, y, stringLength*charWight, hight, 128);
    }

    u16 temp;
    u8 index, mask;
   
    for (int line = 0; line < hight; line++){
        index = 0;
        memset(displayBuff, 0xFF, 200);
        for (int place = 0; place < stringLength; place++){
            for (int byte = 0; byte < charRowByteCount; byte++){
                mask = 0x80;
                temp = 0;

                if (hight == pixel8){
                    for (u8 i = 0; i < 8; i++){
                        temp |= (font[(lineBuff[place]-0x1F)*charByteCount - line*charRowByteCount + byte] & mask) ? 0x0001 : 0x00;
                        if (i != 7){
                            temp <<= 2;
                            mask >>= 1;
                        }
                    }
                } else {
                    for (u8 i = 0; i < 8; i++){
                        temp |= (font[(lineBuff[place]-0x1F)*charByteCount - line*charRowByteCount + byte] & mask) ? 0x4000 : 0x00;
                        if (i != 7){
                            temp >>= 2;
                            mask >>= 1;
                        }
                    }
                }

                displayBuff[startByte + index++] = temp >> 8;
                displayBuff[startByte + index++] = temp;
            }
        }
        displayRectGray(0, y+line, stringLength*charWight, 1, 0, 16);
    }
    
    displayEnd();
}

void displayMenu()
{

}

void imageReadLine(FILE *file, const u16 line)
{
    u16 lineByteCount = linePixelCount/(8/pixelDepth);
    fseek(file, lineByteCount*500-lineByteCount*(line+1), SEEK_SET);
    fread(lineBuff, 1, 400, file);
}

void imageRaw2DisplayData(const u8 gray)
{
    memset(displayBuff, 0x00, 200);
    u8 temp[2];
    for (int i = 399, k = 0; i >= 0; i--, k++){
        temp[0] = lineBuff[i] >> 4, temp[1] = lineBuff[i] & 0x0f;

        grayClass[temp[0]] = (temp[0] > gray) ? 1 : 0;
        grayClass[temp[1]] = (temp[1] > gray) ? 1 : 0;
        displayBuff[k] |= (temp[0] > gray) ? 0x10 : 0x00;
        displayBuff[k] |= (temp[1] > gray) ? 0x40 : 0x00;
        
        i--;
        temp[0] = lineBuff[i] >> 4, temp[1] = lineBuff[i] & 0x0f;

        grayClass[temp[0]] = (temp[0] > gray) ? 1 : 0;
        grayClass[temp[1]] = (temp[1] > gray) ? 1 : 0;
        displayBuff[k] |= (temp[0] > gray) ? 0x01 : 0x00;
        displayBuff[k] |= (temp[1] > gray) ? 0x04 : 0x00;
    }
}

void imageDisplay(const u32 imageNumber, const char* rootPath)
{
    FILE *file = sdOpenfileNumber(rootPath, "rb", imageNumber);

    if (file == NULL){
        ESP_LOGD(tag, "imageReadLine() -> file open failed");
        return;
    }
    
    for (int i = 0; i < 500; i++){
        imageReadLine(file, i);

        int preGray = 0;
        int gray = 0;

       
        while(1){
            imageRaw2DisplayData(gray);

            for (int j = 0;  j < 16; j++){
                if (grayClass[j]){
                    grayClass[j] = 0;
                    gray = j;
                    displayRectGray(0, 100+i, 800, 1, preGray, gray);
                    preGray = gray;
                    goto loop;
                }
                if (j == 15)
                    goto next;
            }
            loop:
            ; 
        }
        next:
        ;
    }
}

void isrHandleSqw()
{
    if (i2cHandle != NULL)
        xTaskResumeFromISR(i2cHandle);
}

void addInterruptSqw()
{
    io.intr_type = GPIO_PIN_INTR_POSEDGE;
    gpio_config(&io);
    gpio_isr_handler_add(interSqwPin, isrHandleSqw, NULL);
}

void removeInterruptSqw()
{
    io.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io);
}

void initInterrupt()
{
    //attachInterrupt(27, isrHandleSqw, RISING);
    io.intr_type = GPIO_PIN_INTR_POSEDGE;
    io.pin_bit_mask = interSqwMask;
    io.mode = GPIO_MODE_INPUT;
    io.pull_up_en = true;

    gpio_config(&io);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(interSqwPin, isrHandleSqw, NULL);
}

void initAll()
{
    esp_err_t re;
    re = sdInit(sdD0, sdD1, sdD2, sdD3, sdCmd, "/sd");
    if (re != ESP_OK){
        displayString(0, 252, pixel8, "Error: SD mount failed");
        ESP_LOGE(tag, "initDisplay() -> init sd fail, message: %s", esp_err_to_name(re));
        return;
    }

    einkInit();
    einkResetPinStatus();
    initRandom();
    initInterrupt();
}

void displayTask()
{
    ESP_LOGD(tag, "in display task");
    u8 runFirst = true;
    u8 monDayCount = 0;
    u8 firstLineCount = 0;
    u8 secLineCount = 0;
    u8 thrLineCount = 0;

    displayStart();
    displayRect(0, 0, 800, 600, 64);
    displayClearRect(0, 0, 800, 600, 64);
    displayEnd();

    //while(1), wake from i2c task, stop isr, run once, start isr and hang
    while(1){
        ESP_LOGD(tag, "display task running");
        removeInterruptSqw();
        for (int i = 0; i < 6; i++){
            if (displayFlag[i]){
                displayFlag[i] = 0;
                switch(i){
                case 0:     //picture
                    ESP_LOGD(tag, "display picture");
                    ebmFileCount = sdGetItemListCount("", "*.ebm");
                    displayStart();
                    displayRect(0, 100, 800, 500, 64);
                    displayClearRect(0, 100, 800, 500, 64);
                    imageDisplay(getRamdom(ebmFileCount), "");
                    displayEnd();
                    break;
                case 1:     //time
                    if (runFirst){
                        displayStart();
                        displayRect(0, 0, 800, 100, 64);
                        displayClearRect(0, 0, 800, 100, 64);
                        displayEnd();
                    }
                    ESP_LOGD(tag, "display time");
                    displayString(660, 38, pixel24, dataTime);
                    break;
                case 2:     //date
                    ESP_LOGD(tag, "display data");
                    displayString(640, 14, pixel12, dataDate);
                    break;
                case 3:     //temp
                    ESP_LOGD(tag, "display temp");
                    if (runFirst){
                        runFirst = false;
                        displayString(560, 74, pixel12, "TEMP");
                        displayString(560, 26, pixel12, "HUMI");
                        displayString(0, 74, pixel12, "1 2 3");
                        displayString(80, 74, pixel12, "1 2 3 4 5 6 7 ");
                        displayString(304, 74, pixel12, "1 2 3 4 5 6 7 ");
                    }
                    displayString(576, 50, pixel12, dataTemp);
                    break;
                case 4:     //hum
                    ESP_LOGD(tag, "display hum");
                    displayString(560, 2, pixel12, dataHum);
                    break;
                case 5:     //cal
                    ESP_LOGD(tag, "display cal");
                    /*monDayCount = getMonDay(checkLeapYear(year), mon);
                    firstLineCount = 10+7-dataDate[0]+1;
                    secLineCount = (monDayCount - firstLineCount >= 17) ? 17 : monDayCount - firstLineCount;
                    thrLineCount = monDayCount - firstLineCount - secLineCount;
                    creatCal(firstLineCount, 1);
                    displayStart();
                    displayString(0, 48, pixel12, (u8)lineBuff);
                    creatCal(secLineCount, firstLineCount+1);
                    displayString(0, 24, pixel12, (u8)lineBuff);
                    creatCal(thrLineCount, firstLineCount+secLineCount+1);
                    displayString(((17-thrLineCount)*2-1)*16, 0, pixel12, (u8)lineBuff);
                    displayEnd();*/
                    break;
                }
            }
        }
        ESP_LOGD(tag, "display task hang up");
        addInterruptSqw();
        vTaskSuspend(NULL);
    }
}

void i2cTask()
{
    ESP_LOGD(tag, "in i2c task");
    i2cInit(0, i2cSda, i2cScl, 400*1000);
    ds3231Enable32kHz(false);
    ds3231EnableOscillator(true, false, 0);
    ds3231SetClockMode(true);
    ds3231SetAlarmStatus(false, 0);
    ds3231SetAlarmStatus(false, 1);

    char preTemp = 100;
    char preHumi = 100;
    u8 preMin = 61;
    u8 preData = 32;
    u8 htu21dMesureFlag = false;
    u8 htu21dIsTemFlag = false;
    double data;

    //while(1), wake from isr, update data and wake up display task, hang
    while (1){
        //htu21d task
        ESP_LOGD(tag, "i2c task running");
        if (htu21dMesureFlag){
            if (htu21dRead(htu21dIsTemFlag, true, &data) == ESP_OK){
                if ((u32)data != (htu21dIsTemFlag ? preTemp : preHumi)){
                    displayFlag[htu21dIsTemFlag ? flagTemp : flagHum] = true;
                    htu21dIsTemFlag ? (preTemp = data) : (preHumi = data);
                }
                double2string(data, htu21dIsTemFlag ? dataTemp : dataHum, 2);
                htu21dMesureFlag = false;
                htu21dIsTemFlag = ~htu21dIsTemFlag;
            }
        } else {
            htu21dMesureFlag = true;
            htu21dMeasure(htu21dIsTemFlag);
        }

        //ds3231 task
        memset(lineBuff, 0x00, 8);
        ds3231GetTime(lineBuff);
        ds3231RowData2DisplayData(lineBuff);
        if (preMin != lineBuff[1]){
            displayFlag[flagTime] = 1;
            preMin = lineBuff[1];
        }
        if (preData != lineBuff[4]){
            displayFlag[flagData] = 1;
            displayFlag[flagCal] = 1;
            preData = lineBuff[4];
        }
        
        for (int i = 0; i < 6; i++)
            if (displayFlag[i]){
                ESP_LOGD(tag, "i2c reusume display");
                vTaskResume(displayHandle);
                break;
            }
        ESP_LOGD(tag, "i2c task hang up");
        vTaskSuspend(NULL);
    }
}

void test()
{
    displayStart();
    while(1){
    displayRect(0, 0, 800, 600, 64);
    //ets_delay_us(1000*1000*1);
    displayClearRect(0, 0, 800, 600, 64);
    //ets_delay_us(1000*1000*1);
    //displayEnd();
    }
}