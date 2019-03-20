#include "myota.h"

#define host_ota "115.231.71.233"
//#define host_ota "119.75.217.109"
/*********************************************/

char ota_host_buf[100];
char filename_buf[100];

char* ota_host=(char *)&ota_host_buf;	
char* filename=(char *)&filename_buf;
int32 port;
xTaskHandle hsj_task_hdl;
/********************************************/
/*********************global param define start ******************************/
bool latest_version_flag = false;
//static char ota_server_version[16] = {0};
 char ota_server_version[16] = {0};
static os_timer_t upgrade_timer;
static uint32_t totallength = 0;
static uint32_t sumlength = 0;
static int percent_temp = 0;
static bool flash_erased = false;
static xTaskHandle* ota_task_handle = NULL;


/*********************global param define end *******************************/

/******************************************************************************
 * FunctionName : upgrade_recycle
 * Description  : recyle upgrade task, if OTA finish switch to run another bin
 * Parameters   :
 * Returns      : none
*******************************************************************************/
static void hilink_upgrade_recycle(void)
{
    totallength = 0;
    sumlength = 0;
    percent_temp = 0;
    flash_erased = false;
    bzero(ota_server_version, 16);
    system_upgrade_deinit();
    os_timer_disarm(&upgrade_timer);
    //reopen sleep mode when get OTA task finish
    wifi_set_sleep_type(MODEM_SLEEP_T);

    printf("[hilink_upgrade_recycle] : system_upgrade_flag_check()=0x%d\n", system_upgrade_flag_check());
    //if OTA success ,reboot to new image and run it,otherwise the APP will notify client the OTA task failed ,
    //the device will delete the ota task,and need client restart the OTA task through Hilink APP

    if (system_upgrade_flag_check() == UPGRADE_FLAG_FINISH) {
        vTaskDelay(100 / portTICK_RATE_MS);
        ota_task_handle = NULL;
		free(filename);
		free(ota_host);
        system_upgrade_reboot();
    } else {
       // hilink_ota_rpt_prg(101, 120);
        ota_task_handle = NULL;
        vTaskDelete(NULL);
    }
}


/******************************************************************************
 * FunctionName : upgrade_download
 * Description  : parse http response ,and download remote data and write in flash
 * Parameters   : char *pusrdata : remote data
 *                length         : data length
 * Returns      : int
*******************************************************************************/
static int upgrade_download(char* pusrdata, unsigned short length)
{
    char* ptr = NULL;
	char* otaptr=NULL;
    char* ptmp2 = NULL;
    char lengthbuffer[32];

	if( (otaptr = (char*)strstr(pusrdata, "HTTP")) != NULL)
	{
		printf("\r\n%s",otaptr);
	}
	
#if 0
	printf("totallenght:%d",totallength);
	printf("lenght:%d",length);
	if(((char*)strstr(pusrdata, "\r\n\r\n")) != NULL)
		{
			printf("\r\n %d",1);
		}
	else
		{
			printf("\r\n %d",2);
		}
	
	if(  ( (char*)strstr(pusrdata, "Content-Length")) != NULL)
		{
			printf("\r\n %d",3);
		}
	else
		{
			printf("\r\n %d",4);
		}
#endif

    if (totallength == 0 && (ptr = (char*)strstr(pusrdata, "\r\n\r\n")) != NULL &&
            (ptr = (char*)strstr(pusrdata, "Content-Length")) != NULL) {
	
        ptr = (char*)strstr(pusrdata, "\r\n\r\n");
        length -= ptr - pusrdata;
        length -= 4;
        printf("upgrade file download start.\n");
        //parser the http head
        ptr = (char*)strstr(pusrdata, "Content-Length: ");

        if (ptr != NULL) {
            ptr += 16;
            ptmp2 = (char*)strstr(ptr, "\r\n");

            if (ptmp2 != NULL) {
                memset(lengthbuffer, 0, sizeof(lengthbuffer));
                memcpy(lengthbuffer, ptr, ptmp2 - ptr);
                sumlength = atoi(lengthbuffer);
                printf("sumlength = %d\n", sumlength);

                if (sumlength > 0) {
                    if (false == system_upgrade(pusrdata, sumlength)) {
                        system_upgrade_flag_set(UPGRADE_FLAG_IDLE);
                        return -1;
                    }

                    flash_erased = true;
                    ptr = (char*)strstr(pusrdata, "\r\n\r\n");

                    if (false == system_upgrade(ptr + 4, length)) {
                        system_upgrade_flag_set(UPGRADE_FLAG_IDLE);
                        return -1;
                    }

                    totallength += length;
                    printf("totallen = %d\n", totallength);
                    return 0;
                }
            } else {
                printf("sumlength failed\n");
                system_upgrade_flag_set(UPGRADE_FLAG_IDLE);
                return -1;
            }
        } else {

            printf("Content-Length: failed\n");
            system_upgrade_flag_set(UPGRADE_FLAG_IDLE);
            return -1;
        }
    } else if (sumlength != 0) {

        totallength += length;
	printf("\r\ntotallength:%d",totallength);
        if (false == system_upgrade(pusrdata, length)) {
            system_upgrade_flag_set(UPGRADE_FLAG_IDLE);
            return -1;
        }

        // need upload progress when download the image,and when OTA is finished,
        // delay some time wait the 100% progress is upload successful then reboot
        int percent_u = (totallength * 20) / sumlength;

        if (percent_temp != percent_u) {
        /*
     		uint8_t pyload[30];
     		sprintf(pyload,"ota---%d",percent_u*5);
			FIFO_WriteData(&mqtt_fifo,pyload,strlen((char *)pyload));
			*/

            percent_temp = percent_u;
        }

        if (percent_temp == 20) {
        /*
     		uint8_t pyload[30];
     		sprintf(pyload,"ota---%d",100);
			FIFO_WriteData(&mqtt_fifo,pyload,strlen((char *)pyload));
		*/
            percent_temp = 0;
           
        }

        if (totallength == sumlength) {
            printf("upgrade file download finished.\n");

            if (upgrade_crc_check(system_get_fw_start_sec(), sumlength) != true) {
                printf("upgrade crc check failed !\n");
                system_upgrade_flag_set(UPGRADE_FLAG_IDLE);
                return -1;
            }

            system_upgrade_flag_set(UPGRADE_FLAG_FINISH);
            return 0;
        } else {
            return 0;
        }
    } else {
    	printf("\r\ndata error!");
        return 0;
    }
}
char* otaIp=NULL;
void InterNet_Dns_Cb(const char *name, struct ip_addr *ipaddr, void *arg)
{
	uint8_t ip[4];

	ip[0] = ipaddr->addr>>24; 
	ip[1] = ipaddr->addr>>16;
	ip[2] = ipaddr->addr>>8; 
	ip[3] = ipaddr->addr; 
	printf("\r\nckw\r\n");
	printf("%d.%d.%d.%d\n",ip[3], ip[2], ip[1], ip[0]); 
	sprintf(otaIp,"%d.%d.%d.%d",ip[3], ip[2], ip[1], ip[0]);
	
}
int sta_socket;
static void hilink_ota_start(void* param)//开始远程升级
{
	int recbytes;
	int sin_size;
	int ota_socket;
	char* recv_buf = NULL;
	sint8 erro=1;
	//ip_addr_t addr;
	struct ip_addr addr;
	otaIp=(char*)malloc(20);
	bzero(otaIp, 20);
	
	recv_buf=(char*)malloc(1400);
	bzero(recv_buf, 1400);

	vTaskDelay(1000/portTICK_RATE_MS);

	
	//disable sleep when OTA
	wifi_set_sleep_type(NONE_SLEEP_T);
	struct sockaddr_in remote_ip;
	printf("Hello, welcome to client!\r\n");

	system_upgrade_flag_set(UPGRADE_FLAG_START);
	system_upgrade_init();

	ota_socket = socket(PF_INET, SOCK_STREAM, 0);

	if (-1 == ota_socket) {
	    close(ota_socket);
	    printf("socket fail !\r\n");
	    vTaskDelete(NULL);
	}

	printf("socket ok!\r\n");
	bzero(&remote_ip, sizeof(struct sockaddr_in));

	remote_ip.sin_family = AF_INET;
	printf("\r\nota host:%s",ota_host);
	printf("\r\nabc\r\n");
	u8 dnscount=0;
	while((otaIp[0] == 0)&&(dnscount<=20))
		{
			dnscount++;
			printf("\r\nOTA DNS ERRO! count:%d",dnscount);
			erro=dns_gethostbyname(ota_host, &addr,  InterNet_Dns_Cb, NULL);
			vTaskDelay(50 / portTICK_RATE_MS);
			//vTaskDelay(1000 / portTICK_RATE_MS);
		}
	printf("\r\n error:%d",erro);
	
	remote_ip.sin_addr.s_addr = inet_addr(otaIp);
	printf("\r\notaIP:%s",otaIp);
	
	remote_ip.sin_port = htons(port);
	printf("\r\nota port:%d\n",port);
	
	if (0 != connect(ota_socket, (struct sockaddr*)(&remote_ip), sizeof(struct sockaddr))) {
	    printf("connect fail!\r\n");
	    system_upgrade_flag_set(UPGRADE_FLAG_IDLE);
		free(recv_buf);
	    goto recycle;
	}

	printf("connect ok!\r\n");
	char* pbuf=NULL;
	pbuf=(char*)malloc(1400);
	bzero(pbuf, 1400);

	sprintf(pbuf, "GET /%s HTTP/1.0\r\nHost: %s:%d\r\n"OTA_HEADER_BUF"",
	   filename, 
	   ota_host,
	   port,
	   MASTER_KEY
	   );

	printf("\r\nurl:%s",pbuf);
	//send this packet to iot.espresif.cn for downloading image
	//发送升级请求包到自己的云平台进行升级
	if (write(ota_socket, pbuf, strlen(pbuf)) < 0) {
		printf("\r\nsend ota url fail!");
		free(recv_buf);
		free(pbuf);
		goto recycle;
	}
	else
	{
		printf("\r\nsend ota url succeed!\n");
	}
	free(pbuf);
	
	totallength=0;
	while ((system_upgrade_flag_check() != UPGRADE_FLAG_FINISH) &&((recbytes = read(ota_socket, recv_buf, 1400)) > 0)) {
		
		if (0 != upgrade_download(recv_buf, recbytes)) {
			free(recv_buf);
		    goto recycle;
		}
		//vTaskDelay(50 / portTICK_RATE_MS);
	}

	if (system_upgrade_flag_check() == UPGRADE_FLAG_FINISH) {
	    close(ota_socket);
		free(recv_buf);
	    hilink_upgrade_recycle();
	    vTaskDelete(NULL);
	}

	if (recbytes <= 0) {
	    printf("read data fail!\r\n");
	    system_upgrade_flag_set(UPGRADE_FLAG_IDLE);
	    free(recv_buf);
	    goto recycle;
	}

	recycle:
	close(ota_socket);
	hilink_upgrade_recycle();
	vTaskDelete(NULL);
}



/*从URL中提取域名和端口、文件函数*/
void  http_parse_request_url(char *URL,char *host,char *filename,int32_t *port){
	char *PA=(char*)os_malloc(strlen(URL));
	char *PB=(char*)os_malloc(strlen(URL));
	memset(host,0,sizeof(host));//给host初始化
	memset(filename,0,sizeof(filename));//给filename初始化
	*port=0;
	if(!(*URL))return;//如果url为空就返回
		PA=URL;
	if(!strncmp(PA,"http://",strlen("http://"))){//判断协议名
		PA=URL+strlen("http://");//移动PA
		}
	if(!strncmp(PA,"https://",strlen("https://"))){
		PA=URL+strlen("https://");
		}
	PB=strchr(PA,'/');//strchr字符查找?返回值是要查找的字符的位置
	if(PB){
		memcpy(host,PA,strlen(PA)-strlen(PB));
		if(PB+1){
			memcpy(filename,PB+1,strlen(PB-1));
			filename[strlen(PB)-1]=0;
		}
		host[strlen(PA)-strlen(PB)]=0;
	}else{
		memcpy(host,PA,strlen(PA));
		host[strlen(PA)]=0;
	}
	PA=strchr(host,':');
	if(PA){
		//*port=atoi(PA+1);
		*port=_EXFUN(atoi,(PA+1));
		}
		
	else
		*port=80;
	free(PA);
	PA=NULL;
	free(PB);
	PB=NULL;
}

void ota_start()
{
	char url[]="http://file2.hxjiot.com/firmware/C85F7DD0B2B000012E82246C1D3C8DD0.bin";
	printf("\r\nparseurl\r\n");
	//http_parse_request_url((char *)&url,ota_host,filename,&port);//解析获取到的URL
	strcpy(ota_host,"appweb.hxjiot.com");
	strcpy(filename,"Hilink_428---425_user1.bin");
	port=80;
	printf("\r\nparseurlok\r\n");
	if (!ota_task_handle) {
		xTaskCreate(hilink_ota_start, "hilink_ota_start_task", 4096, NULL, 5, ota_task_handle);//将优先级2改为4要注意
		os_timer_disarm(&upgrade_timer);
		os_timer_setfn(&upgrade_timer, (os_timer_func_t*)hilink_upgrade_recycle, NULL);
		os_timer_arm(&upgrade_timer, OTA_TIMEOUT, 0);
	} else {
		printf("ota task already start\n");
	}

}
