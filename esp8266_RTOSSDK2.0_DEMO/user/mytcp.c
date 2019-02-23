#include "mytcp.h"
#include "edpkit.h"
#include "mysntp.h"
#include "mystorage.h"
#include "gpio.h"

extern void setKey(uint8_t keynum,uint8_t keyvalue);

xTaskHandle  wifiClientHandle=NULL;
static volatile int tcp_fd;

uint32_t clienttcp_task_cont=0;
int clienttcp_status=0;

int clilenttcp_get_answer=0;
uint32_t clilenttcp_get_answer_cont=0;

static int tcp_max_size=1024;
static unsigned char tcp_reveive_buffer[1024];

unsigned char tcp_send_buffer[1024];


uint8_t tcp_client_isconnect_flag;

int tcp_client_state;
int tcp_client_restart_cont;

static int ICACHE_FLASH_ATTR new_tcp_state(int sock)
{
    int errcode = 0;
    int tcp_fd = sock;
    if(tcp_fd < 0) {
        return -1;
    }
  #if 1  
    fd_set rset, wset;
    int ready_n;

    FD_ZERO(&rset);
    FD_SET(tcp_fd, &rset);
    wset = rset;

    struct timeval timeout;
    timeout.tv_sec = 3;
    timeout.tv_usec = 0;


	/*Ê¹ÓÃselect»úÖÆÅÐ¶ÏtcpÁ¬½Ó×´Ì¬*/
    ready_n = select(tcp_fd + 1, &rset, &wset, NULL, &timeout);
    if(0 == ready_n)
    {
       	printf("select_error");
        errcode = -1;
    }
    else if(ready_n < 0)
    {
        printf("select_error");
        errcode = -1;
    }
    else
    {
    //    ESP_LOGI(TAG,"FD_ISSET(tcp_fd, &rset):%d\n FD_ISSET(tcp_fd, &wset):%d\n",
//                        (int)FD_ISSET(tcp_fd, &rset) , (int)FD_ISSET(tcp_fd, &wset));
         // test in linux environment,kernel version 3.5.0-23-generic
         // tcp server do not send msg to client after tcp connecting
        int ret;
        socklen_t len = sizeof(int);
        if(0 != getsockopt (tcp_fd, SOL_SOCKET, SO_ERROR, &ret, (socklen_t*)&len))
        {
            printf("getsocketopt failed\r\n");
            errcode = -1;
        }
       	// ESP_LOGI(TAG,"getsocketopt ret=%d errno %d\r\n",ret, errno);
        if(0 != ret)
        {
        	printf("getsocketopt ret=%d errno %d\r\n",ret, errno);
            errcode = -1;
        }
    }
#endif
	setsockopt(tcp_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
	setsockopt(tcp_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

	/*
	int ret;
	socklen_t len = sizeof(int);
	if(0 != getsockopt (tcp_fd, SOL_SOCKET, SO_ERROR, &ret, (socklen_t*)&len))
	{
		printf("getsocketopt failed\r\n");
		errcode = -1;
	}
	// ESP_LOGI(TAG,"getsocketopt ret=%d errno %d\r\n",ret, errno);
	if(0 != ret)
	{
		printf("getsocketopt ret=%d errno %d\r\n",ret, errno);
		errcode = -1;
	}
	*/


    return errcode;
}
static void ICACHE_FLASH_ATTR  new_tcp_disconnect(int tcp_fd)
{
    close(tcp_fd);
}
static int ICACHE_FLASH_ATTR new_tcp_connect(in_addr_t srcip,const char* dst, unsigned short port)
{
	struct sockaddr_in servaddr;
	int tcp_fd;
	int flags;
	int reuse;

	if(NULL == dst)
	{
		return -1;
	}

	tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(tcp_fd < 0)
	{
		printf("creat socket tcp_fd failed\n");
		return -1;
	}

	/*ÉèÖÃ·Ç×èÈûÄ£Ê½*/
	flags = fcntl(tcp_fd, F_GETFL, 0);
	if(flags < 0 || fcntl(tcp_fd, F_SETFL, flags | O_NONBLOCK) < 0)
	{
		printf("fcntl: %s\n", strerror(errno));
		close(tcp_fd);
		return -1;
	}

	reuse = 1;
	/*
	if(setsockopt(tcp_fd, SOL_SOCKET, SO_REUSEADDR,
					(const char *) &reuse, sizeof( reuse ) ) != 0 )
	{
		close(tcp_fd);
		printf("set SO_REUSEADDR failed\n");
		return -1;
	}
	*/

	memset(&servaddr, 0, sizeof(struct sockaddr_in));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(dst);
	servaddr.sin_port = htons(port);



	struct sockaddr_in src;
	memset(&src,0,sizeof(struct sockaddr_in));
	src.sin_addr.s_addr = srcip;//Ê¹ÓÃstaµÄ ip
	src.sin_family = AF_INET;
	src.sin_port=htons(10000);
	//destAddr.sin_port = htons(PORT);
	
	//°ó¶¨Ò»ÏÂ ip
	//bind(tcp_fd,&src,sizeof(struct sockaddr));

	

	if(connect(tcp_fd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr_in)) == 0)
	{
		return tcp_fd;
	}
	else
	{
		if(errno == EINPROGRESS)
		{
			printf("tcp conncet noblock\n");
			return tcp_fd;
		}
		else
		{
			close(tcp_fd);
			return -1;
		}
	}
}
static int ICACHE_FLASH_ATTR new_tcp_read(int tcp_fd, unsigned char* buf, unsigned short len)
{
    int ret = -1;

    if(buf == NULL)
    {
        return -1;
    }

    ret = (int)(recv(tcp_fd, buf, len, MSG_DONTWAIT));
    if(ret <= 0)
    {
    	
        if(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
        {
            return 0;//OK
        }
        else
        {
        	printf("tcp errono=%d",errno);
            return -1;
        }
    }
	
    return ret;
}


int ICACHE_FLASH_ATTR new_tcp_send(int tcp_fd, const unsigned char* buf, unsigned short len)
{
    int ret = -1;

    if(buf == NULL)
    {
        return -1;
    }
	uint16_t i; 
	printf("\r\n ESP8266 Send to M2M \r\n");
	for(i=0;i<len;i++)
	{
		printf("%02x ",buf[i]);
	}
	printf("\r\n");
	
    ret = (int)(send(tcp_fd, buf, len, MSG_DONTWAIT));
    if(ret < 0)
    {
        if(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
        {
            return 0;
        }
        else
        {
            return -1;
        }
    }
	printf("\r\n new_tcp_send=%d \r\n",ret);

    return ret;
}
/*
 * buffer°´Ê®Áù½øÖÆÊä³ö
 */
void hexdump(const unsigned char *buf, uint32 num)
{
    uint32 i = 0;
    for (; i < num; i++) 
    {
        printf("%02X ", buf[i]);
        if ((i+1)%8 == 0) 
            printf("\n");
    }
    printf("\n");
}

/*EDP*/
RecvBuffer* recv_buf;
EdpPacket* pkg;
char* src_devid;
char* push_data;
uint32 push_datalen;
char* ds_id;
double dValue = 0;
int iValue = 0;
char* cValue = NULL;

cJSON* desc_json;
char* desc_json_str;
char* save_bin; 
uint32 save_binlen;
unsigned short msg_id=0;
unsigned char save_date_ret;





void ICACHE_FLASH_ATTR tcp_client_parse(char *buffer ,int len)
{
	uint8 mtype,jsonorbin;
	int rtn;
	char* simple_str = NULL;
	char cmd_resp[] = "ok";
	unsigned cmd_resp_len = 0;
	DataTime stTime = {0};
	FloatDPS* float_data = NULL;
	
	char* cmdid;
    uint16 cmdid_len;
    char*  cmd_req;
    uint32 cmd_req_len;
    EdpPacket* send_pkg;
	
	recv_buf = NewBuffer();
	hexdump((const unsigned char *)buffer, len);
	/* ³É¹¦½ÓÊÕÁËn¸ö×Ö½ÚµÄÊý¾Ý */
	WriteBytes(recv_buf, buffer, len);

	/* »ñÈ¡Ò»¸öÍê³ÉµÄEDP°ü */
	if ((pkg = GetEdpPacket(recv_buf)) == 0)
	{
		printf("need more bytes...\n");
		return ;	
	}
	/* »ñÈ¡Õâ¸öEDP°üµÄÏûÏ¢ÀàÐÍ */
	mtype = EdpPacketType(pkg);

	struct UpdateInfoList* up_info = NULL;
	int i = 0;
	int count = 0;

	switch(mtype)
	{
		case CONNRESP:
		{
			rtn = UnpackConnectResp(pkg);
			printf("recv connect resp, rtn: %d\n", rtn);
			if(0==rtn)
			{
				tcp_client_state=tcp_client_state_HEART;
			}
			
		}
		break;
		case PINGRESP:
		{
			//heart rsp
			UnpackPingResp(pkg);
			printf("recv ping resp\n");
		}
		break;
		case PUSHDATA:
		{
		
			UnpackPushdata(pkg, &src_devid, &push_data, &push_datalen);
			printf("recv push data, src_devid: %s, push_data: %s, len: %d\n",
				   src_devid, push_data, push_datalen);
			free(src_devid);
			free(push_data);

		}
		break;
		case UPDATERESP:
		{

			UnpackUpdateResp(pkg, &up_info);
			while (up_info){
				printf("name = %s\n", up_info->name);
				printf("version = %s\n", up_info->version);
				printf("url = %s\nmd5 = ", up_info->url);
				for (i=0; i<32; ++i){
					printf("%c", (char)up_info->md5[i]);
				}
				printf("\n");
				up_info = up_info->next;
			}
			FreeUpdateInfolist(up_info);
		}
		break;
		case SAVEDATA:
		{
			printf("\r\n SAVEDATA \r\n");
			if (UnpackSavedata(pkg, &src_devid, &jsonorbin) == 0)
			{
				if (jsonorbin == kTypeFullJson
					|| jsonorbin == kTypeSimpleJsonWithoutTime
					|| jsonorbin == kTypeSimpleJsonWithTime)
				{
					printf("json type is %d\n", jsonorbin);
					/* ½âÎöEDP°ü - jsonÊý¾Ý´æ´¢ */
					/* UnpackSavedataJson(pkg, &save_json); */
					/* save_json_str=cJSON_Print(save_json); */
					/* printf("recv save data json, src_devid: %s, json: %s\n", */
					/*	   src_devid, save_json_str); */
					/* free(save_json_str); */
					/* cJSON_Delete(save_json); */
		
					/* UnpackSavedataInt(jsonorbin, pkg, &ds_id, &iValue); */
					/* printf("ds_id = %s\nvalue= %d\n", ds_id, iValue); */
		
					UnpackSavedataDouble(jsonorbin, pkg, &ds_id, &dValue);
					printf("ds_id = %s\nvalue = %f\n", ds_id, dValue);
		
					/* UnpackSavedataString(jsonorbin, pkg, &ds_id, &cValue); */
					/* printf("ds_id = %s\nvalue = %s\n", ds_id, cValue); */
					/* free(cValue); */
		
					free(ds_id);
			
				}
				else if (jsonorbin == kTypeBin)
				{/* ½âÎöEDP°ü - binÊý¾Ý´æ´¢ */
					printf("\r\n kTypeBin \r\n");
					UnpackSavedataBin(pkg, &desc_json, (uint8**)&save_bin, &save_binlen);
					desc_json_str=cJSON_Print(desc_json);
					printf("recv save data bin, src_devid: %s, desc json: %s, bin: %s, binlen: %d\n",
						   src_devid, desc_json_str, save_bin, save_binlen);
					free(desc_json_str);
					cJSON_Delete(desc_json);
					free(save_bin);
				}
				else if (jsonorbin == kTypeString ){
					UnpackSavedataSimpleString(pkg, &simple_str);
					printf("\r\n kTypeString \r\n");
					printf("%s\n", simple_str);

					
					free(simple_str);
				}else if (jsonorbin == kTypeStringWithTime){
					printf("\r\n kTypeStringWithTime \r\n");
					UnpackSavedataSimpleStringWithTime(pkg, &simple_str, &stTime);
			
					printf("time:%u-%02d-%02d %02d-%02d-%02d\nstr val:%s\n", 
						stTime.year, stTime.month, stTime.day, stTime.hour, stTime.minute, stTime.second, simple_str);
					free(simple_str);
				}else if (jsonorbin == kTypeFloatWithTime){

					printf("\r\n kTypeFloatWithTime \r\n");
					if(UnpackSavedataFloatWithTime(pkg, &float_data, &count, &stTime)){
						printf("UnpackSavedataFloatWithTime failed!\n");
					}
		
					printf("read time:%u-%02d-%02d %02d-%02d-%02d\n", 
						stTime.year, stTime.month, stTime.day, stTime.hour, stTime.minute, stTime.second);
					printf("read float data count:%d, ptr:[%p]\n", count, float_data);
					
					for(i = 0; i < count; ++i){
						printf("ds_id=%u,value=%f\n", float_data[i].ds_id, float_data[i].f_data);
					}
		
					free(float_data);
					float_data = NULL;
					
				}
				free(src_devid);
			}else{
				printf("error\n");
			}

		}
		break;
        case SAVEACK:
        {
			UnpackSavedataAck(pkg, &msg_id, &save_date_ret);
			printf("save ack, msg_id = %d, ret = %d\n", msg_id, save_date_ret);
        }
        break;

		case CMDREQ:
			printf("\r\n CMDREQ \r\n");
			if (UnpackCmdReq(pkg, &cmdid, &cmdid_len,
							 &cmd_req, &cmd_req_len) == 0)
			{
				char id[50]={};
				char cmd[50]={};
				memset(&id ,0,sizeof(id));
				memset(&cmd ,0,sizeof(cmd));

				memcpy(&id,cmdid,cmdid_len);
				memcpy(&cmd,cmd_req,cmd_req_len);
				
				printf("\r\n id=%s \r\n",id);
				printf("\r\n id_len=%d \r\n",cmdid_len);
				printf("\r\n cmd=%s \r\n",cmd);
				printf("\r\n cmd_len=%d \r\n",cmd_req_len);


				if(strncmp(cmd,"key0:0",6)==0)
				{
					setKey(0,0);
				}
				else if(strncmp(cmd,"key0:1",6)==0)
				{
					setKey(0,1);
				}
				else if(strncmp(cmd,"key1:0",6)==0)
				{
					setKey(1,0);
				}				
				else if(strncmp(cmd,"key1:1",6)==0)
				{
					setKey(1,1);
				}
				else if(strncmp(cmd,"key2:0",6)==0)
				{
					setKey(2,0);
				}
				else if(strncmp(cmd,"key2:1",6)==0)
				{
					setKey(2,1);
				}

				cmd_resp_len = strlen(cmd_resp);
				send_pkg = PacketCmdResp(cmdid, cmdid_len,
										 cmd_resp, cmd_resp_len);

				
				new_tcp_send(tcp_fd, (const char*)send_pkg->_data, send_pkg->_write_pos);
				DeleteBuffer(&send_pkg);
		
				free(cmdid);
				free(cmd_req);
			}
			break;

        
		
		default:
			printf("recv failed...\n");
			break;

	}
	DeleteBuffer(&recv_buf);//É¾µôÕâ¸öbuf
	DeleteBuffer(&pkg);
}

struct station_config s_staconf;//è·¯ç”±å™¨ä¿¡æ¯ç»“æž„ä½“
uint8 phone_ip[4] = {0,0,0,0};//ç”¨æ¥ä¿å­˜æ‰‹æœºIPï¼Œåœ¨mDNSä¸­ç»™æ‰‹æœºè¿›è¡Œé‰´åˆ«ï¼›
void ICACHE_FLASH_ATTR smartconfig_done(sc_status status, void *pdata)
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
            struct station_config *sta_conf = pdata;//è®¾ç½®WiFi station çš„å‚æ•°ï¼Œå¹¶ä¿å­˜åˆ° flash
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

static void ICACHE_FLASH_ATTR tcp_client_task(void *pvParameters)
{
	int send_len = -1;
	int recv_len = -1;
	int recv_flag = 0;
	int input_len;
	int tcp_connect_flag = 0;
	uint8_t sntp_init_flag=0;

	EdpPacket* send_pkg;
    if(storage_list.ssid[0]==0&&storage_list.passowrd[0]==0)//æ²¡ip
    {
    	wifi_set_mode(STATION_MODE);
		smartconfig_stop();
		smartconfig_set_type(SC_TYPE_ESPTOUCH_AIRKISS); //SC_TYPE_ESPTOUCH,SC_TYPE_AIRKISS,SC_TYPE_ESPTOUCH_AIRKISS
		smartconfig_start(smartconfig_done);
		printf("\r\n smartconfig \r\n");

    }
    else
    {
    	start_wifi_station(storage_list.ssid, storage_list.passowrd);
    	printf("\r\n start_wifi_station \r\n");
    }
    

	for(;;)
	{
		printf("\r\n waitforip \r\n");
		vTaskDelay(1000 / portTICK_RATE_MS);
		if(true==wifi_station_connected())
		{
			mysntp_init();
			for(;;)
			{
				printf("\r\n got ip \r\n");
				vTaskDelay(1000 / portTICK_RATE_MS);
				tcp_fd = new_tcp_connect( (in_addr_t)staip.addr,Host, Port);
				if(tcp_fd < 0)
				{
					new_tcp_disconnect(tcp_fd);
					printf("\r\n tcp connect failed \r\n");
					break;
				}
				tcp_client_isconnect_flag=1;
				tcp_connect_flag=0;
				sntp_init_flag=0;
				tcp_client_state=tcp_client_state_CLR;

				while(1)
				{
					//select one time
					if(tcp_connect_flag == 0)
					{
						if(new_tcp_state(tcp_fd)!= 0)
						{
							printf("\r\n tcp not connect \r\n");
							break;
						//  continue;
						}
						else
						{
							printf("\r\n tcp connect successuflly \r\n");
							tcp_connect_flag = 1;
						}
						vTaskDelay(1000/portTICK_RATE_MS);
					}
					/*
					fd_set read_set; 
					struct timeval t_o; 
					int ret;

					FD_ZERO(&read_set); 
					FD_SET(tcp_fd,&read_set); 
					t_o.tv_sec = 1; 
					ret = select(tcp_fd + 1,&read_set,NULL,NULL,&t_o); 
					if(ret == 1) 
					{  
					if(FD_ISSET(tcp_fd,&read_set))
					{
					count = recv(lSockFd,buf,LEN,0); 
					if((count == 0)||(count == -1)) 
					{ 
					//Ê§°Ü
					}

					}
					}
					*/

					switch(tcp_client_state)
					{
						case tcp_client_state_CLR://
						{
							tcp_client_state=tcp_client_state_CLREND;//chekc answer timeout
							clienttcp_task_cont=0;//start to cont
							send_pkg = PacketConnect1("508861595", "dUmdTtj1ccVP3THeRMUXQ9qi6LU=");
							printf("\r\n PacketConnect1 ok \r\n");
							send_len=new_tcp_send(tcp_fd, (const char*)send_pkg->_data, send_pkg->_write_pos);
							if(send_len < 0)
							{
								printf("\r\n send tcp packet failed \r\n");
								goto TCP_CLIENT_END;
							}
							DeleteBuffer(&send_pkg);
						}
						break;
						case tcp_client_state_CLREND:
						{
							if(clienttcp_task_cont==300)//
							{
								printf("\r\n tcp_client_state_CLREND error retry \r\n ");
								goto TCP_CLIENT_END;
							}
						}
						break;
						case tcp_client_state_HEART:
						{
							
							if(clienttcp_task_cont%400==399)
							{
								os_printf("\r\n heapsize:%d \r\n ",system_get_free_heap_size());
								printf("\r\n tcp_client_state_HEART \r\n ");

								uint32_t time;
								uint8_t time_buf[6]={};
								uint8_t outtime_buf[30]={};
								time=sntp_gettime();
								mysntp_gettime(time,time_buf,outtime_buf);//sntp

								send_pkg = PacketPing();
								printf("\r\n PacketPing ok \r\n");
								send_len=new_tcp_send(tcp_fd, (const char*)send_pkg->_data, send_pkg->_write_pos);
								if(send_len < 0)
								{
									printf("\r\n send tcp packet failed \r\n");
									goto TCP_CLIENT_END;
									break;
								}
								DeleteBuffer(&send_pkg);
								clilenttcp_get_answer_cont=1;//start to check

							}

						}
						break;

						case tcp_client_state_SENDDATA:
						{

							printf("\r\n tcp_client_state_SENDDATA \r\n ");
							send_pkg = PacketSavedataSimpleString("", (char *)&tcp_send_buffer, msg_id++);//TYPE=5 ,;feild0;feild1;â€¦;feildn
							printf("\r\n PacketSavedataSimpleString ok \r\n");
							send_len=new_tcp_send(tcp_fd, (const char*)send_pkg->_data, send_pkg->_write_pos);
							if(send_len < 0)
							{
								printf("\r\n send tcp packet failed \r\n");
								goto TCP_CLIENT_END;
								break;
							}
							DeleteBuffer(&send_pkg);
							clilenttcp_get_answer_cont=1;//
							clienttcp_task_cont=0;//
							memset(&tcp_send_buffer,0,sizeof(tcp_send_buffer));

							tcp_client_state=tcp_client_state_HEART;

						}
						break;
						
					}

					clienttcp_task_cont++;
					if(clienttcp_task_cont==50002)
					clienttcp_task_cont=0;

					//if(FD_ISSET(tcp_fd , &read_set))
					{
					recv_len = new_tcp_read(tcp_fd, tcp_reveive_buffer, tcp_max_size-1);
					if(recv_len > 0)
					{
						recv_flag = 0;

						uint16_t i; 
						printf("\r\n M2M Send to ESP8266 \r\n");
						for(i=0;i<recv_len;i++)
						{
							printf("%02x ",tcp_reveive_buffer[i]);
						}
						printf("\r\n");
						
						printf("\r\n recv data:%s,length:%d \n", tcp_reveive_buffer, recv_len);
					//  InterNet_Receive(&tcp_reveive_buffer,recv_len);
						tcp_client_parse((char *)&tcp_reveive_buffer,recv_len);
						clilenttcp_get_answer=1;
					}
					else if(recv_len == 0)
					{
					// ESP_LOGI(TAG,"recv no data\n");
					//	continue;
					}
					else
					{
						printf("\r\n tcp read failed \r\n");
						break;
					}  

					}

					if(clilenttcp_get_answer_cont>0||(clilenttcp_get_answer))
					{

						if(clilenttcp_get_answer_cont++==200)//6S heart
						{
							clilenttcp_get_answer_cont=0;
							if(!clilenttcp_get_answer)
							{
								printf("\r\n  get no answer \r\n");
								//if(clienttcp_status==clienttcp_status_CLREND)
								break;

							}
						}
						if(clilenttcp_get_answer==1)
						{
							clilenttcp_get_answer_cont=0;
							clilenttcp_get_answer=0;
							printf("\r\n get M2M answer \r\n ");
						}

					}

					if(false == wifi_station_connected())
					break;

					vTaskDelay(30/portTICK_RATE_MS);
				}
TCP_CLIENT_END:
				new_tcp_disconnect(tcp_fd);
				tcp_client_isconnect_flag=0;
				if(false == wifi_station_connected())
				break;
			}
		}
	//CLIENT_END:
	}
}

xTaskHandle  ICACHE_FLASH_ATTR tcp_client_handle_get()
{
	return 	wifiClientHandle;
}

void ICACHE_FLASH_ATTR tcp_client_start()
{
	xTaskCreate(tcp_client_task, "tcp_client", 4096, NULL, 5, tcp_client_handle_get());
}

uint8_t ICACHE_FLASH_ATTR tcp_client_isconnect()
{
	return tcp_client_isconnect_flag;
}




