#include "mymqtt.h"

static uint32_t mqtt_task_cont=0;
static uint8_t mqtt_task_status=0;
xTaskHandle  mqtt_task_handle=NULL;

static struct station_config s_staconf;//路由器信息结构体
static uint8 phone_ip[4] = {0,0,0,0};//用来保存手机IP，在mDNS中给手机进行鉴别；

fifo_t          mqtt_fifo;
uint8_t         mqtt_fifo_send_buf[1024];




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
	uint16_t i;
	uint8_t *pdata=data->message->payload;
	for(i=0;i<data->message->payloadlen;i++)
	{
	    uart_tx_one_char(0,(uint8_t) *(pdata+i));
    }
}

uint8_t mqtt_task_status_get()
{
	return mqtt_task_status;
}
void mqtt_task_status_set(uint8_t status)
{
	mqtt_task_status=status;
}

static void ICACHE_FLASH_ATTR mqtt_task(void *pvParameters)
{
	uint8_t wait_ip_cont=0;
	uint8_t mqtt_err_cont=0;
	uint8_t mqtt_reconnect_flag=0;
	bool task_first_flag = false;
	MQTTClient client;
	Network network;
	unsigned char sendbuf[512], readbuf[512] = {0};
	int rc = 0, count = 0;
	MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;

	pvParameters = 0;
	NetworkInit(&network);
	MQTTClientInit(&client, &network, 30000, sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));

	char* address = MQTT_BROKER;

    if(storage_list.ssid[0]==0&&storage_list.passowrd[0]==0)//没ip
    {
    
    	wifi_set_mode(STATION_MODE);
		smartconfig_stop();
		//vTaskDelay(2000/portTICK_RATE_MS);
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
		vTaskDelay(3000 / portTICK_RATE_MS);
		wait_ip_cont++;
		if(storage_list.ssid[0]==0&&storage_list.passowrd[0]==0)//没ip
		{
			mqtt_task_status=mqtt_task_status_WAITMARTLINK;
			//if(wait_ip_cont==60)system_restart();
		}
		else
		{
			mqtt_task_status=mqtt_task_status_WAITWIFI;
			if(wait_ip_cont==60)system_restart();//60秒没连上路由器
		}
		
		
		if(true==wifi_station_connected())
		{
			mqtt_task_status=mqtt_task_status_WAITM2M;
			wait_ip_cont=0;
			mysntp_init();

			
			printf("mqtt client thread starts\n");


			//ota_start();
			

			if(mqtt_err_cont++==4)
			{
				printf("\r\n mqtt_err_cont 4 restart\r\n");
				system_restart();
			}

			if ((rc = NetworkConnect(&network, address, MQTT_PORT)) != 0) {
				printf("Return code from network connect is %d\n", rc);
				continue;
			}

			
			connectData.MQTTVersion = 3;
			connectData.clientID.cstring = "ESP8266_sample";
			connectData.clientID.cstring = MQTT_CLIENTID;
			connectData.username.cstring = MQTT_USERNAME;
			connectData.password.cstring = MQTT_PASSWORD;
			connectData.keepAliveInterval = MQTT_KEEPALIVEINTERVAL;
			connectData.cleansession = true;

			
			if ((rc = MQTTConnect(&client, &connectData)) != 0) {
				printf("Return code from MQTT connect is %d\n", rc);
				network.disconnect(&network);
				continue;//重复等待
			} else {
				printf("MQTT Connected\n");
			}
			if(task_first_flag==false)//TASK只运行一次
			{
				task_first_flag=true;
				if ((rc = MQTTStartTask(&client)) != pdPASS) {
					printf("Return code from start tasks is %d\n", rc);
					network.disconnect(&network);
					continue;//重复等待
				} else {
					printf("Use MQTTStartTask\n");
				}

			}


			if ((rc = MQTTSubscribe(&client,MQTT_SUBSCRIBE, QOS0, messageArrived)) != 0) {
				printf("Return code from MQTT subscribe is %d\n", rc);
				network.disconnect(&network);
				continue;//重复等待
			} else {
				printf("MQTT subscribe to topic \"ESP8266/sample/pub\"\n");
			}

			mqtt_task_status=mqtt_task_status_IDLE;
			mqtt_err_cont=0;
			
			for(;;)
			{
				if(false == wifi_station_connected())
				break;
				if(mqtt_task_cont++==50003)mqtt_task_cont=0;
				vTaskDelay(30/portTICK_RATE_MS);
				switch(mqtt_task_status)
				{
					case mqtt_task_status_IDLE:
					{

						if((mqtt_task_cont%40==2))//1.2s检查一次mqtt
						{
							if(client.isconnected==0)
							{	
								mqtt_reconnect_flag=1;
							}

						}
						
						if(mqtt_task_cont%2000==2)
						{
							sntp_gettime();

							/*
							MQTTMessage message;
							char payload[30]="";
							
							message.qos = QOS0;
							message.retained = 0;
							message.payload = payload;
							sprintf(payload, "message number %d", ++count);
							message.payloadlen = strlen(payload);
							
							if ((rc = MQTTPublish(&client, "ESP8266/sample/pub", &message)) != 0) {
								printf("Return code from MQTT publish is %d\n", rc);
								network.disconnect(&network);
								break;
							} else {
								printf("MQTT publish topic \"ESP8266/sample/pub\", message number is %d\n", count);
							}
							*/
							printf("\r\nmqtt_client.isconnected=%d\r\n",client.isconnected);
							if(client.isconnected==0)
							{	
								mqtt_reconnect_flag=1;
							}
							os_printf("freeHeap: %d\n",system_get_free_heap_size());

						}
						

						if(FIFO_IsDataExit(&mqtt_fifo))//若FIFO还存在
						{
							uint16_t len;
							uint8_t data_buf[256];
							memset(data_buf,0,sizeof(data_buf));
							len=FIFO_ReadData(&mqtt_fifo,(uint8_t *)&data_buf);//
							if(len>0)
							{
								printf("len=%d,data_buf=%s\r\n",len,data_buf);
								MQTTMessage message;
								message.qos = QOS0;
								message.retained = 0;
								message.payload = data_buf;
								message.payloadlen = len;
								
								if ((rc = MQTTPublish(&client, MQTT_PUBLISH, &message)) != 0) {
									printf("Return code from MQTT publish is %d\n", rc);
									network.disconnect(&network);
									mqtt_reconnect_flag=1;
									
								} else {
									printf("\r\nMQTT publish topic %s\r\n",MQTT_PUBLISH);
								}	
							}
						}
					}
					break;
					case mqtt_task_status_OTA:
					{
						ota_start();
						mqtt_task_status_set(mqtt_task_status_IDLE);
						break;
					}
					default:
					break;
				}
				if(mqtt_reconnect_flag)
				{
					mqtt_reconnect_flag=0;
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
	FIFO_Init( &mqtt_fifo, mqtt_fifo_send_buf, sizeof(mqtt_fifo_send_buf));
	xTaskCreate(mqtt_task, "mqtt_task", 4096, NULL, 6, mqtt_task_handle);
}

