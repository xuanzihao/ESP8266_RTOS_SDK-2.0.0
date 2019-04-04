#include "mywebserver.h"
#include "esp_common.h"
#include "mytcp.h"



/*********************
 *      DEFINES
 *********************/
#define INDEX_SIZE  4410
#define WEBCONFIG_SIZE  4526
#define WIFIDONE_SIZE   4266

/**********************
 *      TYPEDEFS
 **********************/

static void webconfig_get_wifi_ssid_pwd(char* urlparam);
static bool parse_url(char *precv, URL_Frame *purl_frame);
static void webserver_recv(void *arg, char *pusrdata, unsigned short length);
static void data_send(void *arg, bool responseOK, char *psend);
static bool parse_url(char *precv, URL_Frame *purl_frame);


/////////////////////////扫描AP/////////////////////////////

//bss列表
bool scand_done_flag=false;
struct bss_info *bss_link;
struct apInfo all_bss_link[30];//应该不会超30个吧。。。
struct apInfoFormat all_bss_link_format[30];
u8 bss_link_Cont=0;
//
void ICACHE_FLASH_ATTR scan_done(void *arg, STATUS status) {
if (status == OK)
{
	uint8 ssid[33];
	char temp[128];
	struct station_config stationConf;

	bss_link_Cont=0;

	//struct bss_info *bss_link = (struct bss_info *)arg;
	bss_link= (struct bss_info *)arg;
	bss_link = bss_link->next.stqe_next;//ignore first

	//if(bss_link->next)
	{
	//	os_printf("%s"bss_link->ssid);
	//	bss_link=bss_link->next;
	}
	while (bss_link != NULL)
	{
		memset(ssid, 0, 33);
		if (strlen(bss_link->ssid) <= 32)
		{
			memcpy(ssid, bss_link->ssid, strlen(bss_link->ssid));
		}
		else
		{
			memcpy(ssid, bss_link->ssid, 32);
		}
		sprintf(temp,"+CWLAP:(认证方式 %d,\"ssid %s\",rssi %d,\""MACSTR"\",信道 %d,加密组 %d,加密 %d)\r\n",
		bss_link->authmode, ssid, bss_link->rssi,
		MAC2STR(bss_link->bssid),bss_link->channel,bss_link->group_cipher,bss_link->pairwise_cipher);
		os_printf("%s",temp);

		memcpy(all_bss_link[bss_link_Cont].ssid,ssid,33);
		memcpy(all_bss_link_format[bss_link_Cont].ssid,ssid,33);
		all_bss_link[bss_link_Cont].channel=bss_link->channel;
		all_bss_link[bss_link_Cont].rssi=bss_link->rssi;
		all_bss_link[bss_link_Cont].authmode=bss_link->authmode;
		all_bss_link[bss_link_Cont].pairwise_cipher=bss_link->pairwise_cipher;

		all_bss_link_format[bss_link_Cont].channel=bss_link->channel;
		all_bss_link_format[bss_link_Cont].rssi=bss_link->rssi;
		//格式化一下认证方式
		switch(all_bss_link[bss_link_Cont].authmode)
		{
			case AUTH_OPEN:
			strcpy(all_bss_link_format[bss_link_Cont].authmode,"OPEN");
			break;
			case AUTH_WEP:
			strcpy(all_bss_link_format[bss_link_Cont].authmode,"WEP");
			break;
			case AUTH_WPA_PSK:
			strcpy(all_bss_link_format[bss_link_Cont].authmode,"WPA-PSK");
			break;
			case AUTH_WPA2_PSK:
			strcpy(all_bss_link_format[bss_link_Cont].authmode,"WPA-PSK/WPA2-PSK");
			break;
			case AUTH_WPA_WPA2_PSK:
			strcpy(all_bss_link_format[bss_link_Cont].authmode,"SHARED");
			break;
			default:
			strcpy(all_bss_link_format[bss_link_Cont].authmode,"AUTH_UNKNOWN");
			break;
		}
		//格式化一下加密方式
		switch(all_bss_link[bss_link_Cont].pairwise_cipher)
		{
			case CIPHER_NONE:
			strcpy(all_bss_link_format[bss_link_Cont].pairwise_cipher,"NONE");
			break;
			case CIPHER_WEP40:
			strcpy(all_bss_link_format[bss_link_Cont].pairwise_cipher,"WEP40");
			break;
			case CIPHER_WEP104:
			strcpy(all_bss_link_format[bss_link_Cont].pairwise_cipher,"WEP104");
			break;
			case CIPHER_TKIP:
			strcpy(all_bss_link_format[bss_link_Cont].pairwise_cipher,"TKIP");
			break;
			case CIPHER_CCMP:
			strcpy(all_bss_link_format[bss_link_Cont].pairwise_cipher,"AES");
			break;
			case CIPHER_TKIP_CCMP:
			strcpy(all_bss_link_format[bss_link_Cont].pairwise_cipher,"TKIP_CCMP");
			break;
			case CIPHER_UNKNOWN:
			break;
			default:
			strcpy(all_bss_link_format[bss_link_Cont].pairwise_cipher,"CIPHER_UNKNOWN");
			break;
		}
		bss_link_Cont++;
		bss_link = bss_link->next.stqe_next;
	}
	//扫描完成以后就开始连接WiFi了
	}
	else
	{
		//os_sprintf(temp,"err, scan status %d\r\n", status);
		//uart0_sendStr(temp);
		os_printf("%s","Error");
	}
#if 0
	os_timer_disarm(&sendApTimer);//发送Ap
	os_timer_setfn(&sendApTimer,(ETSTimerFunc *)sendApTimerCb,NULL); 
	os_timer_arm(&sendApTimer,200,0);//
	extern u8 sendLinkCon;
	sendLinkCon=0; 
#endif
	scand_done_flag=true;
}

void wifi_scand_start()
{
	scand_done_flag=false;
	wifi_station_scan(NULL, scan_done);
}



/**********************

/**********************
 *   STATIC FUNCTIONS
 **********************/
/*
 * softAP模式初始化代码
 */
void ICACHE_FLASH_ATTR softAP_init(void)
{
    struct softap_config soft_ap_Config;

    wifi_set_opmode_current(STATIONAP_MODE);//设置为AP模式，不保存到flash
//  wifi_set_opmode(SOFTAP_MODE);//设置为AP模式，并保存到flash

    soft_ap_Config.ssid_len = 14;//热点名称长度，与你实际的名称长度一致就好
    strcpy(soft_ap_Config.ssid,"XZHCMKG");//实际热点名称设置，可以根据你的需要来
    strcpy(soft_ap_Config.password,"12345678");//热点密码设置
    soft_ap_Config.authmode = AUTH_WPA2_PSK;//设置权限模式，AUTH_WPA2_PSK这是一种加密算法
    soft_ap_Config.beacon_interval = 100;//信标间隔，默认为100
    soft_ap_Config.channel = 1;//信道，共支持1~13个信道
    soft_ap_Config.max_connection = 2;//最大连接数量，最大支持四个，默认四个
    soft_ap_Config.ssid_hidden = 0;//隐藏SSID，0：不隐藏  1：隐藏

    wifi_softap_set_config_current(&soft_ap_Config);//设置 Wi-Fi SoftAP 接口配置，不保存到 Flash
//  wifi_softap_set_config(&soft_ap_Config);//设置 Wi-Fi SoftAP 接口配置，保存到 Flash

    os_printf("\r\nSSID: %s\r\nPWD: %s\r\n",soft_ap_Config.ssid,soft_ap_Config.password);

	struct ip_info info;
	wifi_softap_dhcps_stop();
	info.ip.addr=ipaddr_addr("192.168.99.1");
	info.gw.addr=ipaddr_addr("192.168.99.1");
//	IP4_ADDR(&info.ip, 192, 30, 28, 250);
 // IP4_ADDR(&info.gw, 192, 30, 28, 250);
	IP4_ADDR(&info.netmask, 255, 255, 255, 0);
	
	wifi_set_ip_info(SOFTAP_IF, &info);
	wifi_softap_dhcps_start();

}



/******************************************************************************
 * FunctionName : parse_url
 * Description  : parse the received data from the server
 * Parameters   : precv -- the received data
 *                purl_frame -- the result of parsing the url
 * Returns      : none
*******************************************************************************/
static void ICACHE_FLASH_ATTR webserver_recv(void *arg, char *pusrdata, unsigned short length)
{
    URL_Frame *pURL_Frame = NULL;
    char *pParseBuffer = NULL;
    char *html = NULL;
    SpiFlashOpResult ret = 0;

    os_printf("\r\n\r\nlength:%d\r\n", length);
    os_printf("recv:%s", pusrdata);

    pURL_Frame = (URL_Frame *)os_zalloc(sizeof(URL_Frame));

    parse_url(pusrdata, pURL_Frame);
    os_printf("\r\nType[%d]\r\n", pURL_Frame->Type);
    os_printf("pSelect[%s]\r\n", pURL_Frame->pSelect);
    os_printf("pCommand[%s]\r\n", pURL_Frame->pCommand);
    os_printf("pFilename[%s]\r\n", pURL_Frame->pFilename);

    switch (pURL_Frame->Type) {
        case GET:
            os_printf("We have a GET request.\n");
                if(pURL_Frame->pFilename[0] == 0)
                {
                    html = (char *)os_zalloc(INDEX_SIZE);
                    if(html == NULL){
                        os_printf("os_zalloc error!\r\n");
                        goto _temp_exit;
                    }
                    #if 0
                    // Flash read/write has to be aligned to the 4-bytes boundary
                    ret = spi_flash_read(508*4096, (uint32 *)html, INDEX_SIZE);  // start address:0x10000 + 0xC0000
                    if(ret != SPI_FLASH_RESULT_OK){
                        os_printf("spi_flash_read err:%d\r\n", ret);
                        os_free(html);
                        html = NULL;
                        goto _temp_exit;
                    }
                    #else
                    
					memset(html,0,INDEX_SIZE);
					strcpy(html,(char *)WEB_INDEX);
                    
                    #endif
                    html[INDEX_SIZE] = 0;   // put 0 to the end

                    
                    data_send(arg, true, html);
                    os_free(html);
                    html = NULL;
                }
                if(strncmp(pURL_Frame->pFilename, "WebConfig.html", strlen("WebConfig.html")) == 0)
                {
                    html = (char *)os_zalloc(WEBCONFIG_SIZE);
                    if(html == NULL){
                        os_printf("os_zalloc error!\r\n");
                        goto _temp_exit;
                    }
                    #if 0
                    // Flash read/write has to be aligned to the 4-bytes boundary
                    ret = spi_flash_read(510*4096, (uint32 *)html, WEBCONFIG_SIZE);  // start address:0x10000 + 0xC0000
                    if(ret != SPI_FLASH_RESULT_OK){
                        os_printf("spi_flash_read err:%d\r\n", ret);
                        os_free(html);
                        html = NULL;
                        goto _temp_exit;
                    }
                    #else 

                    
                    wifi_scand_start();
                    while(scand_done_flag==false)vTaskDelay(100);
					memset(html,0,INDEX_SIZE);
					//strcpy(html,WEB_CONFIG);
					strcat(html,WEB_CONFIG_HEAD);

					uint8_t i;
					strcat(html,"<br/>当前环境存在的路由器列表：<br/>");
					strcat(html,"<table border=\"1\">");
					strcat(html,"<tr><td>路由器ssid</td><td>信道</td><td>信号强度</td><td>认证方式</td><td>加密方式</td></tr>");
					uint8_t bss_temp[300]={};
					for(i=0;i<bss_link_Cont-1;i++)
					{
						sprintf(bss_temp,"<tr><td>%s</td><td>%d</td><td>%d</td><td>%s</td><td>%s</td></tr>",
						all_bss_link_format[i].ssid,all_bss_link_format[i].channel,all_bss_link_format[i].rssi,
						all_bss_link_format[i].authmode,all_bss_link_format[i].pairwise_cipher);	
						strcat(html,bss_temp);	
					}
					
					strcat(html,"</table>");

					strcat(html,WEB_CONFIG_TAIL);
                    #endif

             
                    html[WEBCONFIG_SIZE] = 0;   // put 0 to the end
                    data_send(arg, true, html);
                    os_free(html);
                    html = NULL;
                }
            break;

        case POST:
            os_printf("We have a POST request.\r\n");
            if(strncmp(pURL_Frame->pCommand, "connect-wifi", strlen("connect-wifi")) == 0){
                os_printf("connect wifi\r\n");
                webconfig_get_wifi_ssid_pwd(pusrdata);
                html = (char *)os_zalloc(WIFIDONE_SIZE);
                if(html == NULL){
                    os_printf("os_zalloc error!\r\n");
                    goto _temp_exit;
                }
                #if 0
                ret = spi_flash_read(512*4096, (uint32 *)html, WIFIDONE_SIZE);  // start address:0x10000 + 0xC0000
                if(ret != SPI_FLASH_RESULT_OK){
                    os_printf("spi_flash_read err:%d\r\n", ret);
                    os_free(html);
                    html = NULL;
                    goto _temp_exit;
                }
                #else
				memset(html,0,INDEX_SIZE);
				strcpy(html,WEB_WIFI_CONFIG);
                #endif

                
                html[WIFIDONE_SIZE] = 0;   // put 0 to the end
                data_send(arg, true, html);
                os_free(html);
                html = NULL;
            }
            break;
    }
    _temp_exit:
        ;
    if(pURL_Frame != NULL){
        os_free(pURL_Frame);
        pURL_Frame = NULL;
    }

}



static void ICACHE_FLASH_ATTR
webconfig_get_wifi_ssid_pwd(char* urlparam)
{
    char *p = NULL, *q = NULL;
    char ssid[10], pass[15];

    memset(ssid, 0, sizeof(ssid));
    memset(pass, 0, sizeof(pass));

    p = (char *)strstr(urlparam, "SSID=");
    q = (char *)strstr(urlparam, "PASSWORD=");
    if ( p == NULL || q == NULL ){
        return;
    }
    memcpy(ssid, p + 5, q - p - 6);
    memcpy(pass, q + 9, strlen(urlparam) - (q - urlparam) - 9);
    os_printf("ssid[%s]pass[%s]\r\n", ssid, pass);

	/*
    wifi_set_opmode(STATION_MODE);
    struct station_config stConf;
    stConf.bssid_set = 0;
    memset(&stConf.ssid, 0, sizeof(stConf.ssid));
    memset(&stConf.password, 0, sizeof(stConf.password));

    memcpy(&stConf.ssid, ssid, strlen(ssid));
    memcpy(&stConf.password, pass, strlen(pass));

    wifi_station_set_config(&stConf);
    //重启
    system_restart();
    */
}

/******************************************************************************
 * FunctionName : parse_url
 * Description  : parse the received data from the server
 * Parameters   : precv -- the received data
 *                purl_frame -- the result of parsing the url
 * Returns      : none
*******************************************************************************/
static bool ICACHE_FLASH_ATTR
parse_url(char *precv, URL_Frame *purl_frame)
{
    char *str = NULL;
    uint8 length = 0;
    char *pbuffer = NULL;
    char *pbufer = NULL;

    if (purl_frame == NULL || precv == NULL) {
        return false;
    }

    pbuffer = (char *)strstr(precv, "Host:");

    if (pbuffer != NULL) {
        length = pbuffer - precv;
        pbufer = (char *)os_zalloc(length + 1);
        pbuffer = pbufer;
        memcpy(pbuffer, precv, length);
        memset(purl_frame->pSelect, 0, URLSize);
        memset(purl_frame->pCommand, 0, URLSize);
        memset(purl_frame->pFilename, 0, URLSize);

        if (strncmp(pbuffer, "GET ", 4) == 0) {
            purl_frame->Type = GET;
            pbuffer += 4;
        } else if (strncmp(pbuffer, "POST ", 5) == 0) {
            purl_frame->Type = POST;
            pbuffer += 5;
        }else{
            return false;
        }

        pbuffer ++;
        str = (char *)strstr(pbuffer, "HTTP");

        if (str != NULL) {
            length = str - pbuffer - 1;
            memcpy(purl_frame->pFilename, pbuffer, length);
        }

        os_free(pbufer);
    }

    pbuffer = (char *)strstr(precv, "SSID");
    if (pbuffer != NULL) {
        purl_frame->Type = POST;
        memcpy(purl_frame->pCommand, "connect-wifi", strlen("connect-wifi"));
        os_free(pbufer);
    }

}



/******************************************************************************
 * FunctionName : data_send
 * Description  : processing the data as http format and send to the client or server
 * Parameters   : arg -- argument to set for client or server
 *                responseOK -- true or false
 *                psend -- The send data
 * Returns      :
*******************************************************************************/
static void ICACHE_FLASH_ATTR
data_send(void *arg, bool responseOK, char *psend)
{
    uint16 length = 0;
    char *pbuf = NULL;
    char httphead[256];
    //struct espconn *ptrespconn = arg;
    int *pfd=arg;
    int fd=*pfd;
    memset(httphead, 0, 256);

    if (responseOK) {
        sprintf(httphead,
                   "HTTP/1.0 200 OK\r\nContent-Length: %d\r\nServer: lwIP/1.4.0\r\n",
                   psend ? strlen(psend) : 0);

        if (psend) {
            sprintf(httphead + strlen(httphead),
                       "Content-type: text/html; charset=utf-8\r\nPragma: no-cache\r\n\r\n");
            length = strlen(httphead) + strlen(psend);
            pbuf = (char *)os_zalloc(length + 1);
            memcpy(pbuf, httphead, strlen(httphead));
            memcpy(pbuf + strlen(httphead), psend, strlen(psend));
        } else {
            sprintf(httphead + strlen(httphead), "\n");
            length = strlen(httphead);
        }
    } else {
        sprintf(httphead, "HTTP/1.0 400 BadRequest\r\nContent-Length: 0\r\nServer: lwIP/1.4.0\r\n\n");
        length = strlen(httphead);
    }

    if (psend) {
    	new_tcp_send(fd,pbuf,length);
        //espconn_sent(ptrespconn, pbuf, length);
    } else {
		new_tcp_send(fd,httphead,length);
        //espconn_sent(ptrespconn, httphead, length);
    }

    if (pbuf) {
        os_free(pbuf);
        pbuf = NULL;
    }
}

#define CONFIG_EXAMPLE_IPV4
static void web_server_task(void *pvParameters)
{
    char rx_buffer[1460];
    char addr_str[128];
    int addr_family;
    int ip_protocol;

    while (1) {

#ifdef CONFIG_EXAMPLE_IPV4
        struct sockaddr_in destAddr;
        destAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        destAddr.sin_family = AF_INET;
        destAddr.sin_port = htons(80);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;
        inet_ntoa_r(destAddr.sin_addr, addr_str, sizeof(addr_str) - 1);
#else // IPV6
        struct sockaddr_in6 destAddr;
        bzero(&destAddr.sin6_addr.un, sizeof(destAddr.sin6_addr.un));
        destAddr.sin6_family = AF_INET6;
        destAddr.sin6_port = htons(80);
        addr_family = AF_INET6;
        ip_protocol = IPPROTO_IPV6;
        inet6_ntoa_r(destAddr.sin6_addr, addr_str, sizeof(addr_str) - 1);
#endif

        int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
        if (listen_sock < 0) {
            printf("Unable to create socket: errno %d", errno);
            break;
        }
        printf("Socket created");

		#if 0
        int reuse;
		if(setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR,
						(const char *) &reuse, sizeof( reuse ) ) != 0 )
		{
			close(listen_sock);
			printf("set SO_REUSEADDR failed\n");
			break;
		}
		#endif


        int err = bind(listen_sock, (struct sockaddr *)&destAddr, sizeof(destAddr));
        if (err != 0) {
            printf("Socket unable to bind: errno %d", errno);
            break;
        }
        printf("Socket binded");

        err = listen(listen_sock, 1);
        if (err != 0) {
            printf("Error occured during listen: errno %d", errno);
            break;
        }
        printf("Socket listening");

#ifdef CONFIG_EXAMPLE_IPV6
        struct sockaddr_in6 sourceAddr; // Large enough for both IPv4 or IPv6
#else
        struct sockaddr_in sourceAddr;
#endif
        uint addrLen = sizeof(sourceAddr);
        
		while(1)
		{
        
        int sock = accept(listen_sock, (struct sockaddr *)&sourceAddr, &addrLen);
        if (sock < 0) {
            printf("Unable to accept connection: errno %d", errno);
            break;
        }
        printf("Socket accepted");

        while (1) 
        {
            int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
            // Error occured during receiving
            if (len < 0) {
               printf("recv failed: errno %d", errno);
                break;
            }
            // Connection closed
            else if (len == 0) {
                printf("Connection closed");
                break;
            }
            // Data received
            else {
#ifdef CONFIG_EXAMPLE_IPV6
                // Get the sender's ip address as string
                if (sourceAddr.sin6_family == PF_INET) {
                    inet_ntoa_r(((struct sockaddr_in *)&sourceAddr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
                } else if (sourceAddr.sin6_family == PF_INET6) {
                    inet6_ntoa_r(sourceAddr.sin6_addr, addr_str, sizeof(addr_str) - 1);
                }
#else
                inet_ntoa_r(((struct sockaddr_in *)&sourceAddr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
#endif

                rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
                printf("Received %d bytes from %s:", len, addr_str);
                printf("%s", rx_buffer);

				#if 0
                int err = send(sock, rx_buffer, len, 0);
                if (err < 0) {
                    printf("Error occured during sending: errno %d", errno);
                    break;
                }
                #endif

                webserver_recv(&sock,rx_buffer,strlen(rx_buffer));
            }

            
        }
        if (sock != -1) {
            printf("Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }

		}
        #if 0
        if (sock != -1) {
            printf("Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
        #endif
    }
    vTaskDelete(NULL);
}

void web_server_start()
{
	xTaskCreate(web_server_task, "web_server_task", 6000, NULL, 5, NULL);
}






