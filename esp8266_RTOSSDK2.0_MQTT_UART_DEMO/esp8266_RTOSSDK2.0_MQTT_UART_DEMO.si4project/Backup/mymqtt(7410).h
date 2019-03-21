#ifndef __mymqtt__
#define __mymqtt__
#include "esp_common.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "airkiss.h"


#include "user_config.h"
#include "wifi_state_machine.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include "mystorage.h"
#include "mqtt/MQTTFreeRTOS.h"
#include "mqtt/MQTTClient.h"
#include "mqtt/MQTTConnect.h"
#include "mqtt/MQTTPublish.h"
#include "mqtt/MQTTFormat.h"
#include "mqtt/MQTTSubscribe.h"
#include "mqtt/MQTTPublish.h"
#include "mqtt/MQTTUnsubscribe.h"
#include "mqtt/StackTrace.h"
#include <stddef.h>

#include <myfifo.h>
#include <myuart.h>




#define MQTT_PUBLISH "ESP8266/PUB"
#define MQTT_SUBSCRIBE "ESP8266/SUB"




#define MQTT_BROKER  "mnifdv.cn"  /* MQTT Broker Address*/
#define MQTT_PORT    1883             /* MQTT Port*/
#define MQTT_CLIENTID     "testid1"
#define MQTT_USERNAME	"usermane"
#define MQTT_PASSWORD	"password"
#define MQTT_KEEPALIVEINTERVAL 15

enum
{
	mqtt_task_status_INIT=0,//初始化
	mqtt_task_status_WAITMARTLINK=1,//等待smartlink
	mqtt_task_status_WAITWIFI,//等待连接wifi
	mqtt_task_status_WAITM2M,//连接wifi成功，等待连接MQTT
	mqtt_task_status_IDLE,//正常，透传
	mqtt_task_status_OTA,//正在OTA
};

fifo_t          mqtt_fifo;
uint8_t         mqtt_fifo_send_buf[1024];

uint8_t mqtt_task_status_get();
void mqtt_task_status_set(uint8_t status);
void ICACHE_FLASH_ATTR mqtt_start();
extern void ota_start();

#endif

