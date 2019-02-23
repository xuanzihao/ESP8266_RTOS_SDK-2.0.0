#ifndef __mystorage__
#define __mystorage__

#include "esp_common.h"


#include "upgrade.h"
#include "spi_flash.h"


/* NOTICE---this is for 512KB spi flash.
 * you can change to other sector if you use other size spi flash. */
#define ESP_PARAM_START_SEC		0x3f8

#define XZH_PARAM_START_SEC		0xFC//0x7cflash数据保存的扇区地址


#define HSJ_PARAM_START_SEC		0xFC//0x7cflash数据保存的扇区地址

#define ESP_PARAM_SAVE_0    1
#define ESP_PARAM_SAVE_1    2
#define ESP_PARAM_FLAG      3

#define packet_size   (2 * 1024)

#define STORAGE_INIT 0xBB

struct  esp_platform_sec_flag_param {
    uint8 flag; 
    uint8 pad[3];
};
struct esp_platform_saved_param {
    uint8 devkey[40];
    uint8 token[40];
    uint8 activeflag;
    uint8 pad[3];
};

typedef struct	
{
	uint8_t init_flag;
	uint8_t ssid[32];
	uint8_t passowrd[64];
}Storage_list;



Storage_list storage_list;




#endif

