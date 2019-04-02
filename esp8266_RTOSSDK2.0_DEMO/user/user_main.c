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






static os_timer_t timer;

LOCAL key_typedef_t * singleKey[GPIO_KEY_NUM];								///< 定义单个按键成员数组指针
LOCAL keys_typedef_t keys;

LOCAL key0_status=0;
LOCAL key1_status=0;
LOCAL key2_status=0;





 


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

void setKey(uint8_t keynum,uint8_t keyvalue)
{
	memset(&tcp_send_buffer,0,sizeof(tcp_send_buffer));
	switch(keynum)
	{
		case 0:
		if(keyvalue==0)
		{
			memcpy(&tcp_send_buffer,",;key0,0",8);
			GPIO_OUTPUT_SET(GPIO_ID_PIN(KEY_0O_IO_NUM), 0); 
		}
		else
		{
			GPIO_OUTPUT_SET(GPIO_ID_PIN(KEY_0O_IO_NUM), 1); 
			memcpy(&tcp_send_buffer,",;key0,1",8);
		}
		break;
		case 1:
		if(keyvalue==0)
		{
			GPIO_OUTPUT_SET(GPIO_ID_PIN(KEY_1O_IO_NUM), 0); 
			memcpy(&tcp_send_buffer,",;key1,0",8);
		}
		else
		{
			GPIO_OUTPUT_SET(GPIO_ID_PIN(KEY_1O_IO_NUM), 1); 
			memcpy(&tcp_send_buffer,",;key1,1",8);
		}

		break;
		case 2:
		if(keyvalue==0)
		{
			GPIO_OUTPUT_SET(GPIO_ID_PIN(KEY_2O_IO_NUM), 0); 
			memcpy(&tcp_send_buffer,",;key2,0",8);
		}
		else
		{
			GPIO_OUTPUT_SET(GPIO_ID_PIN(KEY_2O_IO_NUM), 1); 
			memcpy(&tcp_send_buffer,",;key2,1",8);
		}
		break;
	}
	tcp_client_state=tcp_client_state_SENDDATA;
	

}


void key0LongPress()
{
	os_printf("\r\n key0LongPress! \r\n");
	//GPIO_OUTPUT_SET(GPIO_ID_PIN(KEY_0O_IO_NUM), key0_status); 
	setKey(0,key0_status);
}
void key0ShortPress()
{
	os_printf("\r\n key0ShortPress! \r\n");
	key0_status=!key0_status;
	//GPIO_OUTPUT_SET(GPIO_ID_PIN(KEY_0O_IO_NUM), key0_status); 
	setKey(0,key0_status);
}
void key1LongPress()
{
	os_printf("\r\n key1LongPress! \r\n");

	memset(&storage_list,0,sizeof(storage_list));
	system_param_save_with_protect(XZH_PARAM_START_SEC,(void *)&storage_list,sizeof(storage_list));
	printf("\r\n system_param_save_with_protect storage_list ok sec=%d len=%d \r\n",XZH_PARAM_START_SEC,sizeof(storage_list));
	system_restore();		//恢复出厂设置，清除保存的WiFi信息
	system_restart();		//系统重启

	
}
void key1ShortPress()
{
	key1_status=!key1_status;
	os_printf("\r\n key1ShortPress! \r\n");
	//GPIO_OUTPUT_SET(GPIO_ID_PIN(KEY_1O_IO_NUM), key1_status); 
	setKey(1,key1_status);
}
void key2LongPress()
{
	os_printf("\r\n key2LongPress! \r\n");
	//GPIO_OUTPUT_SET(GPIO_ID_PIN(KEY_2O_IO_NUM), key2_status); 
	setKey(2,key2_status);

	
}
void key2ShortPress()
{
	key2_status=!key2_status;
	os_printf("\r\n key2ShortPress! \r\n");
	//GPIO_OUTPUT_SET(GPIO_ID_PIN(KEY_2O_IO_NUM), key2_status); 
	setKey(2,key2_status);
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
#if 1
	singleKey[0] = keyInitOne(KEY_0_IO_NUM, KEY_0_IO_MUX, KEY_0_IO_FUNC,
	                            key0LongPress, key0ShortPress);
	singleKey[1] = keyInitOne(KEY_1_IO_NUM, KEY_1_IO_MUX, KEY_1_IO_FUNC,
	                            key1LongPress, key1ShortPress);
	singleKey[2] = keyInitOne(KEY_2_IO_NUM, KEY_2_IO_MUX, KEY_2_IO_FUNC,
	                            key2LongPress, key2ShortPress);

	keys.singleKey = singleKey;
	keyParaInit(&keys);
	PIN_FUNC_SELECT(KEY_0O_IO_MUX, KEY_0O_IO_FUNC); 
	PIN_FUNC_SELECT(KEY_1O_IO_MUX, KEY_1O_IO_FUNC); 
	PIN_FUNC_SELECT(KEY_2O_IO_MUX, KEY_2O_IO_FUNC); 

	GPIO_OUTPUT_SET(GPIO_ID_PIN(KEY_0O_IO_NUM), key0_status); 
	GPIO_OUTPUT_SET(GPIO_ID_PIN(KEY_1O_IO_NUM), key1_status); 
	GPIO_OUTPUT_SET(GPIO_ID_PIN(KEY_2O_IO_NUM), key2_status); 

#else 
	PIN_FUNC_SELECT(KEY_0_IO_MUX, KEY_0_IO_FUNC); 
	PIN_FUNC_SELECT(KEY_1_IO_MUX, KEY_1_IO_FUNC); 
	PIN_FUNC_SELECT(KEY_2_IO_MUX, KEY_2_IO_FUNC); 
	
	PIN_FUNC_SELECT(KEY_0O_IO_MUX, KEY_0O_IO_FUNC); 
	PIN_FUNC_SELECT(KEY_1O_IO_MUX, KEY_1O_IO_FUNC); 
	PIN_FUNC_SELECT(KEY_2O_IO_MUX, KEY_2O_IO_FUNC); 
	os_printf("\r\nKey Init Success!\r\n");
#endif
	os_printf("\r\nKey Init Success!\r\n");


}

/*
LOCAL void ICACHE_FLASH_ATTR keyTask(void *pvParameters)
{
	keyInit();
	while(1)
	{
		gpio_output_set((GPIO_INPUT_GET(KEY_0_IO_NUM))<<KEY_0O_IO_NUM, ((~(GPIO_INPUT_GET(KEY_0_IO_NUM)))&0x01)<<KEY_0O_IO_NUM, 1<<KEY_0O_IO_NUM,0);
		gpio_output_set((GPIO_INPUT_GET(KEY_1_IO_NUM))<<KEY_1O_IO_NUM, ((~(GPIO_INPUT_GET(KEY_1_IO_NUM)))&0x01)<<KEY_1O_IO_NUM, 1<<KEY_1O_IO_NUM,0);
		gpio_output_set((GPIO_INPUT_GET(KEY_2_IO_NUM))<<KEY_2O_IO_NUM, ((~(GPIO_INPUT_GET(KEY_2_IO_NUM)))&0x01)<<KEY_2O_IO_NUM, 1<<KEY_2O_IO_NUM,0);
		vTaskDelay(30/portTICK_RATE_MS);
	}

}
*/
LOCAL void ICACHE_FLASH_ATTR rf_task(void *pvParameters)
{
	unsigned char txchr[10]= {0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x6d};
	uint8_t rf_ch=0;
	unsigned char Rxchr[20]= {0};

	//RF_RxMode();
	for(;;)
	{	
		
		rf_ch=0;
		RF_Init();
		vTaskDelay(150/portTICK_RATE_MS);
		
		if((ucRF_GetStatus()&&TX_DS_FLAG))
		{	  
			RF_ClearFIFO();
			RF_ClearStatus ();  
			os_printf("\r\n 发送完成 \r\n");
		} //PC_CR2=0x020; 	
		os_printf("\r\n ucRF_GetStatus:%d \r\n",ucRF_GetStatus());

		RF_TxMode();
		rf_ch=ucRF_ReadReg(RF_CH);
		os_printf("\r\n rf_ch:%d \r\n",rf_ch);
		{
			if (rf_ch==78)
			{
					//os_printf("\r\n rf_ch:%d \r\n",rf_ch);
				ucRF_TxData(txchr,10);
			}
		}

		vTaskDelay(150/portTICK_RATE_MS);
		
		/*
		rf_ch=ucRF_ReadReg(RF_CH);
		os_printf("\r\n rf_ch:%d \r\n",rf_ch);

		if(!IRQ_STATUS)
		{
			RF_ClearStatus ();  
			ucRF_DumpRxData(Rxchr,15);
			RF_ClearFIFO();
			os_printf("\r\nRxchr:%s\r\n",Rxchr);
		}
		vTaskDelay(100/portTICK_RATE_MS);
		*/
		
		
	}

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
//	RF_Init();//test rf 

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
    init_esp_wifi();//
    //stop_wifi_ap();
	tcp_client_start();
//	xTaskCreate(rf_task, "rf_task", 4096, NULL, 6, NULL);
}
