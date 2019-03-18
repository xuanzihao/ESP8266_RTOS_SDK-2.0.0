#include "mymqtt.h"

static uint32_t mqtt_task_cont=0;
static uint8_t mqtt_task_status=0;
xTaskHandle  mqtt_task_handle=NULL;

static struct station_config s_staconf;//路由器信息结构体
static uint8 phone_ip[4] = {0,0,0,0};//用来保存手机IP，在mDNS中给手机进行鉴别；
static void ICACHE_FLASH_ATTR smartconfig_done(sc_status status, void *pdata)
{
    switch(status) {
        case SC_STATUS_WAIT:
	       os_printf("\r\nSC_STATUS_WAIT\n");
            break;
        case SC_STATUS_FIND_CHANNEL:
		os_printf("\r\nSC_STATUS_FIND_CHANNEL\n");
            break;
        case SC_STATUS_GETTING_SSID_PSWD:
		os_printf("\r\nSC_STATUS_GETTING_SSID_PSWD\n");
			sc_type *type = pdata;
            if (*type == SC_TYPE_ESPTOUCH) {
		  	 os_printf("\r\nSC_TYPE:SC_TYPE_ESPTOUCH\n");
            } else {
		  	 os_printf("\r\nSC_TYPE:SC_TYPE_AIRKISS\n");
            }
            break;
        case SC_STATUS_LINK:
		os_printf("\r\nSC_STATUS_LINK\n");
            struct station_config *sta_conf = pdata;//设置WiFi station 的参数，并保存到 flash
               //s_staconf = pdata;
            memcpy(&s_staconf,sta_conf,sizeof(struct station_config));
	        wifi_station_set_config(sta_conf);	
	        wifi_station_disconnect();
	        wifi_station_connect();

			memcpy(&storage_list.ssid,sta_conf->ssid,sizeof(sta_conf->ssid));
			memcpy(&storage_list.passowrd,sta_conf->password,sizeof(sta_conf->password));
			system_param_save_with_protect(XZH_PARAM_START_SEC,(void *)&storage_list,sizeof(storage_list));
	        
            break;
        case SC_STATUS_LINK_OVER:
		 os_printf("\r\nSC_STATUS_LINK_OVER\n");
            if (pdata != NULL) {
                memcpy(phone_ip, (uint8*)pdata, 4);
                os_printf("configIp: %d.%d.%d.%d\n",phone_ip[0],phone_ip[1],phone_ip[2],phone_ip[3]);
                os_printf("\r\n");
            }
			smartconfig_stop();
	
            break;
    }
	
}
static void messageArrived(MessageData* data)
{
    printf("Message arrived: %s\n", data->message->payload);
}


static void ICACHE_FLASH_ATTR mqtt_task(void *pvParameters)
{
	
    if(storage_list.ssid[0]==0&&storage_list.passowrd[0]==0)//没ip
    {
    	wifi_set_mode(STATION_MODE);
		smartconfig_stop();
		smartconfig_set_type(SC_TYPE_ESPTOUCH_AIRKISS); //SC_TYPE_ESPTOUCH,SC_TYPE_AIRKISS,SC_TYPE_ESPTOUCH_AIRKISS
		smartconfig_start(smartconfig_done);
		printf("\r\n smartconfig \r\n");

    }
    else
    {
    	wifi_set_mode(STATION_MODE);
    	start_wifi_station(storage_list.ssid, storage_list.passowrd);
    	printf("\r\n start_wifi_station \r\n");
    }

	for(;;)
	{
		printf("\r\n waitforip \r\n");
		vTaskDelay(1000 / portTICK_RATE_MS);
		if(true==wifi_station_connected())
		{
			mqtt_task_status=0;
			mysntp_init();

			
			printf("mqtt client thread starts\n");
			MQTTClient client;
			Network network;
			unsigned char sendbuf[80], readbuf[80] = {0};
			int rc = 0, count = 0;
			MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
			
			pvParameters = 0;
			NetworkInit(&network);
			MQTTClientInit(&client, &network, 30000, sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));
			
			char* address = MQTT_BROKER;
			

			if ((rc = NetworkConnect(&network, address, MQTT_PORT)) != 0) {
				printf("Return code from network connect is %d\n", rc);
			}

			
			connectData.MQTTVersion = 3;
			connectData.clientID.cstring = "ESP8266_sample";
			
			if ((rc = MQTTConnect(&client, &connectData)) != 0) {
				printf("Return code from MQTT connect is %d\n", rc);
				continue;//重复等待
			} else {
				printf("MQTT Connected\n");
			}

			if ((rc = MQTTStartTask(&client)) != pdPASS) {
				printf("Return code from start tasks is %d\n", rc);
			} else {
				printf("Use MQTTStartTask\n");
			}

			
			if ((rc = MQTTSubscribe(&client, "ESP8266", QOS0, messageArrived)) != 0) {
				printf("Return code from MQTT subscribe is %d\n", rc);
				continue;//重复等待
			} else {
				printf("MQTT subscribe to topic \"ESP8266/sample/pub\"\n");
			}

			
			for(;;)
			{
				if(false == wifi_station_connected())
				break;
				if(mqtt_task_cont++==50003)mqtt_task_cont=0;
				vTaskDelay(30/portTICK_RATE_MS);
				switch(mqtt_task_status)
				{
					case 0:
					{
						if(mqtt_task_cont%300==2)
						{
							sntp_gettime();


							MQTTMessage message;
							char payload[30]="";
							
							message.qos = QOS0;
							message.retained = 0;
							message.payload = payload;
							sprintf(payload, "message number %d", ++count);
							message.payloadlen = strlen(payload);
							
							if ((rc = MQTTPublish(&client, "ESP8266/sample/pub", &message)) != 0) {
								printf("Return code from MQTT publish is %d\n", rc);
							} else {
								printf("MQTT publish topic \"ESP8266/sample/pub\", message number is %d\n", count);
							}
							os_printf("freeHeap: %d\n",system_get_free_heap_size());

						}
						
					}


					
					break;
					case 1:
					break;
					case 2:
					break;
					case 99:
					break;
					default:
					break;
				}
			}
			//mqtt断开处理

		}
		else
		{
			
		}
	}
}


void ICACHE_FLASH_ATTR mqtt_start()
{
	xTaskCreate(mqtt_task, "mqtt_task", 4096, NULL, 5, mqtt_task_handle);
}

