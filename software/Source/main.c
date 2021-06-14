#include "Header/common.h"
#include "Header/mytask.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"
#include "./Header/eink.h"

TaskHandle_t i2cHandle = NULL;
TaskHandle_t displayHandle = NULL;

static const char* tag = "main";

void app_main()
{
    ESP_LOGD(tag, "start");

    ets_delay_us(1000);

    initAll();
    
    BaseType_t xReturned;

    xReturned = xTaskCreate(displayTask, "displayTask", 20480, NULL, 10, &displayHandle);
    if (xReturned != pdPASS){
        ESP_LOGE(tag, "Fatal Error: creat display task failed");
        return;
    } else {
        ESP_LOGI(tag, "creat display task success");
    }

    xReturned = xTaskCreate(i2cTask, "i2cTask", 20480, NULL, 1, &i2cHandle);
    if (xReturned != pdPASS){
        ESP_LOGE(tag, "Fatal Error: creat i2c task failed");
        return;
    } else {
        ESP_LOGI(tag, "creat i2c task success");
    }

    //test();

    ESP_LOGD(tag, "end");
}   