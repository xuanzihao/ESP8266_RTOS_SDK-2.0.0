/*
 * ESPRSSIF MIT License
 *
 * Copyright (c) 2017 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "esp_common.h"
#include "espressif/esp_system.h"
#include "user_config.h"
#include "myuart.h"
#include "mysntp.h"
#include "mytcp.h"

#include "mystorage.h"
#include "hal_key.h"
//#include "smartconfig.h"
#include "espressif/espconn.h"
#include "espressif/airkiss.h"

#include "RF.h"

#include "mymqtt.h"



static os_timer_t timer;

LOCAL key_typedef_t * singleKey[GPIO_KEY_NUM];								///< 定义单个按键成员数组指针
LOCAL keys_typedef_t keys;



/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABCCC
 *                A : rf cal
 *                B : rf init data
 *                C : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
*******************************************************************************/
uint32 user_rf_cal_sector_set(void)
{
    flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;
        case FLASH_SIZE_64M_MAP_1024_1024:
            rf_cal_sec = 2048 - 5;
            break;
        case FLASH_SIZE_128M_MAP_1024_1024:
            rf_cal_sec = 4096 - 5;
            break;
        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}

LOCAL void ICACHE_FLASH_ATTR wait_for_connection_ready(uint8 flag)
{
    os_timer_disarm(&timer);
    if(wifi_station_connected()){
        os_printf("connected\n");
    } else {
        os_printf("reconnect after 2s\n");
        os_timer_setfn(&timer, (os_timer_func_t *)wait_for_connection_ready, NULL);
        os_timer_arm(&timer, 2000, 0);
    }
}

LOCAL void ICACHE_FLASH_ATTR on_wifi_connect(){
    os_timer_disarm(&timer);
    os_timer_setfn(&timer, (os_timer_func_t *)wait_for_connection_ready, NULL);
    os_timer_arm(&timer, 100, 0);
}

LOCAL void ICACHE_FLASH_ATTR on_wifi_disconnect(uint8_t reason){
    os_printf("disconnect %d\n", reason);
}


void key0LongPress()
{
	os_printf("\r\n key0LongPress! \r\n");
	memset(&storage_list,0,sizeof(storage_list));
	system_param_save_with_protect(XZH_PARAM_START_SEC,(void *)&storage_list,sizeof(storage_list));
	printf("\r\n system_param_save_with_protect storage_list ok sec=%d len=%d \r\n",XZH_PARAM_START_SEC,sizeof(storage_list));
	system_restore();		//恢复出厂设置，清除保存的WiFi信息
	system_restart();		//系统重启
}
void key0ShortPress()
{

	os_printf("\r\n key0ShortPress! \r\n");
}

/*******************************************************************************
* Function Name  : keyInit
* Description    : 按键初始化函数
* Input          : none
* Output         : None
* Return         : none
* Attention		   : None
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR keyInit(void)
{
#if 0


#else 
	singleKey[0] = keyInitOne(KEY_0_IO_NUM, KEY_0_IO_MUX, KEY_0_IO_FUNC,
	                            key0LongPress, key0ShortPress);

	keys.singleKey = singleKey;
	keyParaInit(&keys);
#endif
	os_printf("\r\nKey Init Success!\r\n");


}



/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void user_init(void)
{
	//my_uart_init_new();
	keyInit();
	uart_init_new();
    printf("SDK version:%s\n", system_get_sdk_version());
    vTaskDelay(2000/portTICK_RATE_MS);
    

	if(system_param_load(XZH_PARAM_START_SEC,0,(void *)&storage_list,sizeof(storage_list))==true)
	{
		printf("\r\n system_param_load storage_list ok sec=%d len=%d \r\n",XZH_PARAM_START_SEC,sizeof(storage_list));
	}
	
	if(storage_list.init_flag!=STORAGE_INIT)
	{
		storage_list.init_flag=STORAGE_INIT;
		system_param_save_with_protect(XZH_PARAM_START_SEC,(void *)&storage_list,sizeof(storage_list));
		printf("\r\n system_param_save_with_protect storage_list ok sec=%d len=%d \r\n",XZH_PARAM_START_SEC,sizeof(storage_list));
	} 
    set_on_station_connect(on_wifi_connect);
    set_on_station_disconnect(on_wifi_disconnect);
    init_esp_wifi();//STATION mode



	mqtt_start();
    
    //stop_wifi_ap();
	//tcp_client_start();
	//xTaskCreate(rf_task, "rf_task", 4096, NULL, 6, NULL);
}
