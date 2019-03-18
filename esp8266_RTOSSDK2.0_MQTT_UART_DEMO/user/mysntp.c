#include "mysntp.h"

/*******************************************************************************
* Function Name  : str_compare
* Description    : �ȶ�String��С
* Input          : buf�����������ݣ�len���������ݳ���
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
* Description    :��ʼ����SNTP
* Input          : None
* Output         : None
* Return         : none
* Attention		   : None
*******************************************************************************/
void ICACHE_FLASH_ATTR mysntp_init(void)
{
	sntp_setservername(0,"ntp1.aliyun.com");//����SNTP������
	//sntp_setservername(0,"0.cn.pool.ntp.org");//����SNTP������
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
* Description    :��ȡSNTPʱ��
* Input          : None
* Output         : None
* Return         : ��ȡ����ʱ���
* Attention		   : None
*******************************************************************************/
u32 ICACHE_FLASH_ATTR	sntp_gettime(void)
{
	uint32 current_stamp=0;
	
	current_stamp = sntp_get_current_timestamp();//��ȡʱ���
	if(current_stamp == 0)
	{
		os_printf("\r\nSntp Get time fail!\r\n");
	}
	else
	{
		os_printf("sntp: %d, %s \n",current_stamp,sntp_get_real_time(current_stamp));//ת���ɱ���ʱ���ʱ���
	}

	return current_stamp;
}

/*******************************************************************************
* Function Name  : smartlock_gettime
* Description    :����ȡ����ʱ���ת����������
* Input          : timeStamp - ��ȡ����ʱ���
* Output         : Time_data - ת�����������
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
		memcpy((char *)&cur_time[0],(char *)sntp_get_real_time(timeStamp),30);	//����ӡ����
		
		//os_printf("\r\ncur_time = %s\r\n",cur_time);

		/*��ӡ����������ʱ��*/
		//os_printf("\r\nsntyear: ");
		for(i=0;i<4;i++){
			//os_printf("%c",cur_time[20+i]);
		}
		Time_data[0] = (u8)(1000*(cur_time[20]-'0')+100*(cur_time[21]-'0')+10*(cur_time[22]-'0')+(cur_time[23]-'0')-2000);
		//os_printf("\tjiexipyear:%d\r\n",Time_data[0]);
		/*��ǰ�·�*/
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
		/*��ǰ��*/
		//os_printf("\r\nsntpday:%c%c",cur_time[8],cur_time[9]);
		Time_data[2] = (u8)(10*(cur_time[8]-'0')+(cur_time[9]-'0'));
		//os_printf("\tjiexiday:%d \r\n",Time_data[2]);
		/*��ǰСʱ*/
		//os_printf("\r\nsntphour:%c%c",cur_time[11],cur_time[12]);
		Time_data[3] = (u8)(10*(cur_time[11]-'0')+(cur_time[12]-'0'));
		//os_printf("\tjiexihour:%d \r\n",Time_data[3]);
		/*��ǰ����*/
		//os_printf("\r\nsntpminute:%c%c",cur_time[14],cur_time[15]);
		Time_data[4] = (u8)(10*(cur_time[14]-'0')+(cur_time[15]-'0'));
		//os_printf("\tjieximinute:%d \r\n",Time_data[4]);
		/*��ǰ��*/
		//os_printf("\r\nsntpsecond:%c%c",cur_time[17],cur_time[18]);
		Time_data[5] = (u8)(10*(cur_time[17]-'0')+(cur_time[18]-'0'));
		//os_printf("\tjiexisecond:%d \r\n",Time_data[5]);
		os_printf("\r\n#jiexitime->year:20%d,mounth:%d,day:%d,hour:%d,mintue:%d,second:%d\r\n",Time_data[0],Time_data[1],Time_data[2],Time_data[3],Time_data[4],Time_data[5]);

		sprintf(outdata,"20%02d-%002d-%02d %02d:%02d:%02d",Time_data[0],Time_data[1],Time_data[2],Time_data[3],Time_data[4],Time_data[5]);
		os_printf("\r\n outdata \r\n%s",(char *)outdata);

	}
	
}

