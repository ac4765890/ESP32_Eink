#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "sdmmc_cmd.h"
#include "Header/common.h"

static bool runFirst = true;
static const char *tag = "SD Model";

esp_err_t sdInit(const u8 d0, const u8 d1, const u8 d2, const u8 d3, const u8 cmd, const char *mountPoint)
{
    ESP_LOGD(tag, "sdInit()-> Init sd now");
    if (runFirst){
        runFirst = false;
        sdmmc_host_t host = SDMMC_HOST_DEFAULT();
        sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

        gpio_set_pull_mode(d0 ,GPIO_PULLUP_ONLY);
        gpio_set_pull_mode(d1 ,GPIO_PULLUP_ONLY);
        gpio_set_pull_mode(d2 ,GPIO_PULLUP_ONLY);
        gpio_set_pull_mode(d3 ,GPIO_PULLUP_ONLY);
        gpio_set_pull_mode(cmd ,GPIO_PULLUP_ONLY);

        esp_vfs_fat_sdmmc_mount_config_t mount_config = {
            .format_if_mount_failed = true,
            .max_files = 8,
            .allocation_unit_size = 16 * 1024
        };

        sdmmc_card_t* card;
        esp_err_t ret = esp_vfs_fat_sdmmc_mount(mountPoint, &host, &slot_config, &mount_config, &card);
        if (ret != ESP_OK) {
            if (ret == ESP_FAIL) {
                ESP_LOGE(tag, "Failed to mount filesystem.");
            } else {
                ESP_LOGE(tag, "Failed to initialize the card (%s). "
                    "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
            }
            return ret;
        }

        sdmmc_card_print_info(stdout, card);
    } else {
        ESP_LOGW(tag, "sdInit()-> Call sdInit() more then once");
    }
    return ESP_OK;
}

bool sdCheckExistsFile(const char *path)
{
    FILINFO fno;
    FRESULT fr = f_stat(path, &fno);

    switch (fr) {
    case FR_OK:
        ESP_LOGD(tag, "sdCheckExistsFile() -> file exist");
        return true;

    case FR_NO_FILE:
        ESP_LOGD(tag, "sdCheckExistsFile() -> file not exist");
        return false;

    default:
        ESP_LOGD(tag, "An error occured. (%d)\n", fr);
        return false;
    }
}

int sdRenameFile(const char *src, const char *target)
{
    return rename(src, target);
}

int sdDelFile(const char *path)
{
    return unlink(path);
}

FILE *sdOpenfileNumber(const char *rootPath, const char *opr, const u32 number)
{
    FRESULT fr;    
    FF_DIR dj;        
    FILINFO fno;   
    u32 count = 0;

    fr = f_findfirst(&dj, &fno, rootPath, "*.ebm"); 

    char buff[256] = "/sd/";
    while (fr == FR_OK && fno.fname[0]) {      
        if (count == number){
            strcat(buff, fno.fname);
            break;
        }

        fr = f_findnext(&dj, &fno);               
        count++;
    }
    f_closedir(&dj);

    ESP_LOGD(tag, "file path: %s", buff);
    return fopen(buff, opr);
}

u32 sdGetItemListCount(const char* dir, const char* suffix)
{
    FRESULT fr;    
    FF_DIR dj;        
    FILINFO fno;   
    u32 count = 0;

    fr = f_findfirst(&dj, &fno, dir, suffix); 

    while (fr == FR_OK && fno.fname[0]) { 
        ESP_LOGD(tag, "sdGetItemListCount() -> file name %s", fno.fname);  
        fr = f_findnext(&dj, &fno);               
        count++;
    }

    f_closedir(&dj);
    return count;
}

void sdUnmount()
{
    esp_vfs_fat_sdmmc_unmount();
}