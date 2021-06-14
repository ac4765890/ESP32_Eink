#include "./Header/common.h"
#include "driver/i2c.h"

static bool firstInit = true;
static u8 port = 0;
static const char *tag = "i2c Model";

void i2cInit(const u8 _port, const u8 sda, const u8 scl, const u32 freq)
{
    ESP_LOGD(tag, "i2cInit()-> Init i2c bus now");
    if (firstInit){
        firstInit = false;
        port = _port;
        i2c_config_t conf;
        conf.mode = I2C_MODE_MASTER;
        conf.sda_io_num = sda;
        conf.scl_io_num = scl;
        conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
        conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
        conf.master.clk_speed = freq;
        i2c_param_config(port, &conf);
        i2c_driver_install(port, I2C_MODE_MASTER, false, false, false);
    } else {
        ESP_LOGW(tag, "i2cInit()-> Call i2cInit() more than once");
    }
}

//one step function

esp_err_t i2cStart()
{
    i2c_cmd_handle_t handle = i2c_cmd_link_create();

    i2c_master_start(handle);

    esp_err_t ret = i2c_master_cmd_begin(port, handle, pdMS_TO_TICKS(10));
    i2c_cmd_link_delete(handle);

    return ret;
}

esp_err_t i2cRecvByte(u8* byte, const bool isNack)
{
    i2c_cmd_handle_t handle = i2c_cmd_link_create();

    i2c_master_read_byte(handle, byte, isNack);

    esp_err_t ret = i2c_master_cmd_begin(port, handle, pdMS_TO_TICKS(10));
    i2c_cmd_link_delete(handle);

    return ret;
}

esp_err_t i2cSendByte(u8* byte, const bool isCheckNack)
{
    i2c_cmd_handle_t handle = i2c_cmd_link_create();

    i2c_master_write_byte(handle, *byte, isCheckNack);

    esp_err_t ret = i2c_master_cmd_begin(port, handle, pdMS_TO_TICKS(10));
    i2c_cmd_link_delete(handle);

    return ret;
}

esp_err_t i2cStop()
{
    i2c_cmd_handle_t handle = i2c_cmd_link_create();

    i2c_master_stop(handle);

    esp_err_t ret = i2c_master_cmd_begin(port, handle, pdMS_TO_TICKS(10));
    i2c_cmd_link_delete(handle);

    return ret;
}

//cmd line

esp_err_t i2cReadByte(u8* byte, const u8 addr)
{
    i2c_cmd_handle_t handle = i2c_cmd_link_create();

    i2c_master_start(handle);
    i2c_master_write_byte(handle, addr | Read, true);
    i2c_master_read_byte(handle, byte, NACK);
    i2c_master_stop(handle);

    esp_err_t ret = i2c_master_cmd_begin(port, handle, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(handle);

    //ESP_LOGD(tag, "i2cReadByte()-> status: %s", esp_err_to_name(ret));
    return ret;
}

esp_err_t i2cReadString(u8* string, const u16 size, const u8 addr)
{
    i2c_cmd_handle_t handle = i2c_cmd_link_create();

    i2c_master_start(handle);
    i2c_master_write_byte(handle, addr | Read, true);
    i2c_master_read(handle, string, size-1, ACK);
    i2c_master_read_byte(handle, string+size-1, NACK);
    i2c_master_stop(handle);

    esp_err_t ret = i2c_master_cmd_begin(port, handle, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(handle);

    //ESP_LOGD(tag, "i2cReadString()-> status: %s", esp_err_to_name(ret));
    return ret;
}

esp_err_t i2cWriteByte(const u8 byte, const u8 addr)
{
    i2c_cmd_handle_t handle = i2c_cmd_link_create();

    i2c_master_start(handle);
    i2c_master_write_byte(handle, addr | Write, true);
    i2c_master_write_byte(handle, byte, true);
    i2c_master_stop(handle);

    esp_err_t ret = i2c_master_cmd_begin(port, handle, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(handle);

    //ESP_LOGD(tag, "i2cWriteByte()-> status: %s", esp_err_to_name(ret));
    return ret;
}

esp_err_t i2cWriteString(u8* string, const u16 size, const u8 addr)
{
    i2c_cmd_handle_t handle = i2c_cmd_link_create();

    i2c_master_start(handle);
    i2c_master_write_byte(handle, addr | Write, true);
    i2c_master_write(handle, string, size-1, true);
    i2c_master_write_byte(handle, *(string+size-1), false);
    i2c_master_stop(handle);

    esp_err_t ret = i2c_master_cmd_begin(port, handle, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(handle);

    //ESP_LOGD(tag, "i2cWriteString()-> status: %s", esp_err_to_name(ret));
    return ret;
}