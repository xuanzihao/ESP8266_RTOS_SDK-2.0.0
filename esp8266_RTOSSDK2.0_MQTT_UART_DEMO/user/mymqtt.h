#ifndef __mymqtt__
#define __mymqtt__
#include "esp_common.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

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




#define MQTT_BROKER  "mnifdv.cn"  /* MQTT Broker Address*/
#define MQTT_PORT    1883             /* MQTT Port*/

void ICACHE_FLASH_ATTR mqtt_start();


#endif

