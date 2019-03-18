#include "mysntp.h"

/*******************************************************************************
* Function Name  : str_compare
* Description    : 比对String大小
* Input          : buf：待加密数据；len：加密数据长度
* Output         : None
* Return         : -1	:	data1<data2;
					1	:	data1>data2;
					0	:	data1=data2;
* Attention		   : None
*******************************************************************************/
u8 ICACHE_FLASH_ATTR str_compare(u8 *data1,u8 *data2,u8 len)
{
	u8 i;
	for(i=0;i<len;i++){
		if(data1[i] <data2[i]){
			return -1;
		}
		else if(data1[i]>data2[i]){
			return 1;	
		}
		else{
			continue;
		}
	}
	return 0;
}

/*******************************************************************************
* Function Name  : Sntp_Init
* Description    :初始化化SNTP
* Input          : None
* Output         : None
* Return         : none
* Attention		   : None
*******************************************************************************/
void ICACHE_FLASH_ATTR mysntp_init(void)
{
	sntp_setservername(0,"ntp1.aliyun.com");//设置SNTP主务器
	//sntp_setservername(0,"0.cn.pool.ntp.org");//设置SNTP主务器
	sntp_setservername(1,"0.cn.pool.ntp.org");//
	sntp_setservername(2,"1.cn.pool.ntp.org");
	//sntp_setservername(0,"ewad.com");
	//sntp_setservername(1,"ewad.com");
	//sntp_setservername(2,"ewad.com");
	sntp_init();
	os_printf("\r\n    SNTP INIT OK!");
}

/*******************************************************************************
* Function Name  : Sntp_GetTime
* Description    :获取SNTP时间
* Input          : None
* Output         : None
* Return         : 获取到的时间戳
* Attention		   : None
*******************************************************************************/
u32 ICACHE_FLASH_ATTR	sntp_gettime(void)
{
	uint32 current_stamp=0;
	
	current_stamp = sntp_get_current_timestamp();//获取时间戳
	if(current_stamp == 0)
	{
		os_printf("\r\nSntp Get time fail!\r\n");
	}
	else
	{
		os_printf("sntp: %d, %s \n",current_stamp,sntp_get_real_time(current_stamp));//转换成北京时间的时间戳
	}

	return current_stamp;
}

/*******************************************************************************
* Function Name  : smartlock_gettime
* Description    :将获取到的时间戳转换成年月日
* Input          : timeStamp - 获取到的时间戳
* Output         : Time_data - 转换后的年月日
* Return         : none
* Attention	: None
*******************************************************************************/
void ICACHE_FLASH_ATTR mysntp_gettime(u32 timeStamp,u8 Time_data[6],u8 *outdata)
{
	u8 i;
	char cur_time[30];
	char *p_mounth = "JanFebMarAprMayJunJulAugSepOctNovDec";
	if(timeStamp>0)
	{	
		memcpy((char *)&cur_time[0],(char *)sntp_get_real_time(timeStamp),30);	//将打印出来
		
		//os_printf("\r\ncur_time = %s\r\n",cur_time);

		/*打印解析出来的时间*/
		//os_printf("\r\nsntyear: ");
		for(i=0;i<4;i++){
			//os_printf("%c",cur_time[20+i]);
		}
		Time_data[0] = (u8)(1000*(cur_time[20]-'0')+100*(cur_time[21]-'0')+10*(cur_time[22]-'0')+(cur_time[23]-'0')-2000);
		//os_printf("\tjiexipyear:%d\r\n",Time_data[0]);
		/*当前月份*/
		//os_printf("\r\nsntpmounth: ");
		for(i=0;i<3;i++){
			//os_printf("%c",cur_time[4+i]);
		}
		for(i=0;i<12;i++){
			if(str_compare(cur_time+4,p_mounth+3*i,3) == 0){
				Time_data[1] = i+1;
				//os_printf("\tjieximounth:%x\r\n",Time_data[1]);		
			}
		}
		/*当前天*/
		//os_printf("\r\nsntpday:%c%c",cur_time[8],cur_time[9]);
		Time_data[2] = (u8)(10*(cur_time[8]-'0')+(cur_time[9]-'0'));
		//os_printf("\tjiexiday:%d \r\n",Time_data[2]);
		/*当前小时*/
		//os_printf("\r\nsntphour:%c%c",cur_time[11],cur_time[12]);
		Time_data[3] = (u8)(10*(cur_time[11]-'0')+(cur_time[12]-'0'));
		//os_printf("\tjiexihour:%d \r\n",Time_data[3]);
		/*当前分钟*/
		//os_printf("\r\nsntpminute:%c%c",cur_time[14],cur_time[15]);
		Time_data[4] = (u8)(10*(cur_time[14]-'0')+(cur_time[15]-'0'));
		//os_printf("\tjieximinute:%d \r\n",Time_data[4]);
		/*当前秒*/
		//os_printf("\r\nsntpsecond:%c%c",cur_time[17],cur_time[18]);
		Time_data[5] = (u8)(10*(cur_time[17]-'0')+(cur_time[18]-'0'));
		//os_printf("\tjiexisecond:%d \r\n",Time_data[5]);
		os_printf("\r\n#jiexitime->year:20%d,mounth:%d,day:%d,hour:%d,mintue:%d,second:%d\r\n",Time_data[0],Time_data[1],Time_data[2],Time_data[3],Time_data[4],Time_data[5]);

		sprintf(outdata,"20%02d-%002d-%02d %02d:%02d:%02d",Time_data[0],Time_data[1],Time_data[2],Time_data[3],Time_data[4],Time_data[5]);
		os_printf("\r\n outdata \r\n%s",(char *)outdata);

	}
	
}

