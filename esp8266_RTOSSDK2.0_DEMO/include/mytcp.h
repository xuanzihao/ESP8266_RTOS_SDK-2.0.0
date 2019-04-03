#ifndef __MYTCP_H__
#define __MYTCP_H__

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

extern unsigned char tcp_send_buffer[1024];

extern xTaskHandle tcp_client_handle_get();
void tcp_client_start();

uint8_t tcp_client_isconnect();


int tcp_client_state;
enum
{
	tcp_client_state_CLR=0,
	tcp_client_state_CLREND,
	
	tcp_client_state_HEART,

	tcp_client_state_SENDDATA,
};
int ICACHE_FLASH_ATTR new_tcp_send(int tcp_fd, const unsigned char* buf, unsigned short len);



#endif
