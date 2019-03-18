//============================================================================//
//  * @file           RF.H
//  * @author         YUNBAO 
//  * @version        V1.0
//  * @date           02/4/2016
//  * @brief          YB24 communication interface
//  * @modify user:   
//  * @modify date:   
//  * website :   www.ybdzkj.com  长沙云宝电子科技有限公司
//============================================================================//
#include "RF.H"

const unsigned char  TX_ADDRESS_DEF[5] = {0xCC,0xCC,0xCC,0xCC,0xCC};    		//RF 地址：接收端和发送端需一致

/******************************************************************************/
//            SPI_init
//               init spi pin and IRQ  CE input/out mode
/******************************************************************************/
void SPI_init(void)
{
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4); 
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5); 
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12); 
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13); 
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14); 
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15); 
	os_printf("\r\n spi_init_ok\r\n");

/*    GPIO_Init( GPIOD, GPIO_Pin_0, GPIO_Mode_In_PU_No_IT);                       //IRQ  input pulling high without interrupt
    GPIO_Init( GPIOB, GPIO_Pin_1, GPIO_Mode_Out_PP_Low_Fast);                   //CE   output Low pulling push    

    GPIO_Init( GPIOB, GPIO_Pin_4, GPIO_Mode_Out_PP_High_Fast);                  //CSN  output High pulling push
    GPIO_Init( GPIOB, GPIO_Pin_5, GPIO_Mode_Out_PP_Low_Fast);                   //SCK  output Low  pulling push 
    GPIO_Init( GPIOB, GPIO_Pin_6, GPIO_Mode_Out_PP_High_Fast);                  //MOSI output High pulling push
    GPIO_Init( GPIOB, GPIO_Pin_7, GPIO_Mode_In_PU_No_IT);                       //MISO input pull high*/
}
void delay_10us(unsigned char time)
{
	//unsigned char i;
	//for(i = 0; i<200; i++);
	//_asm("nop");
	//vTaskDelay(1/portTICK_RATE_MS);
	os_delay_us(10*time);
}
void delay_ms(unsigned char time)
{
	unsigned char i,k;

	os_delay_us(time*1000);
	/*
	for(k = 0; k< time; k++)
	{
		for(i = 0; i<200; i++)
		{
			;
			//_asm("nop");
			//_asm("nop");
		}
	}	
	*/
	
	//vTaskDelay(1/portTICK_RATE_MS);
}
/******************************************************************************/
//            SPI_RW
//                SPI Write/Read Data
//            SPI写入一个BYTE的同时，读出一个BYTE返回
/******************************************************************************/
 unsigned char SPI_RW( unsigned char	 R_REG)
{
    unsigned char	  i;
    for(i = 0; i < 8; i++)
    {
        SCK_LOW;
        if(R_REG & 0x80)
        {
            MOSI_HIGH;
        }
        else
        {
            MOSI_LOW;
        }
        R_REG = R_REG << 1;
        SCK_HIGH;
        if( MISO_STATUS )
        {
          R_REG = R_REG | 0x01;
        }
    }
    SCK_LOW;
    return R_REG;
}

/******************************************************************************/
//            RF_WriteReg
//                Write Data(1 Byte Address ,1 byte data)
/******************************************************************************/
void RF_WriteReg( unsigned char reg,  unsigned char wdata)
{
    CSN_LOW;
    SPI_RW(reg);
    SPI_RW(wdata);
    CSN_HIGH;
}


/******************************************************************************/
//            RF_ReadReg
//                Read Data(1 Byte Address ,1 byte data return)
/******************************************************************************/
 unsigned char	ucRF_ReadReg( unsigned char reg)
{
     unsigned char tmp;
    
    CSN_LOW;
    SPI_RW(reg);
    tmp = SPI_RW(0);
    CSN_HIGH;
    
    return tmp;
}

/******************************************************************************/
//            RF_WriteBuf
//                Write Buffer
/******************************************************************************/
void RF_WriteBuf( unsigned char reg, unsigned char *pBuf, unsigned char length)
{
     unsigned char j;
    CSN_LOW;
    j = 0;
    SPI_RW(reg);
    for(j = 0;j < length; j++)
    {
        SPI_RW(pBuf[j]);
    }
    j = 0;
    CSN_HIGH;
}

/******************************************************************************/
//            RF_ReadBuf
//                Read Data(1 Byte Address ,length byte data read)
/******************************************************************************/
void RF_ReadBuf( unsigned char reg, unsigned char *pBuf,  unsigned char length)
{
    unsigned char byte_ctr;

    CSN_LOW;                    		                               			
    SPI_RW(reg);       		                                                		
    for(byte_ctr=0;byte_ctr<length;byte_ctr++)
    	pBuf[byte_ctr] = SPI_RW(0);                                                 		
    CSN_HIGH;                                                                   		
}



/******************************************************************************/
//            RF_TxMode
//                Set RF into TX mode
/******************************************************************************/
void RF_TxMode(void)
{
    CE_LOW;
    RF_WriteReg(W_REGISTER + CONFIG,  0X8E);							// 将RF设置成TX模式
    delay_10us(1);
    
}


/******************************************************************************/
//            RF_RxMode
//            将RF设置成RX模式，准备接收数据
/******************************************************************************/
void RF_RxMode(void)
{
    CE_LOW;
    RF_WriteReg(W_REGISTER + CONFIG,  0X8F );							// 将RF设置成RX模式
    CE_HIGH;											// Set CE pin high 开始接收数据
    delay_ms(2);
}

/******************************************************************************/
//            RF_GetStatus
//            read RF IRQ status,3bits return
/******************************************************************************/
unsigned char ucRF_GetStatus(void)
{
    return ucRF_ReadReg(STATUS)&0x70;								//读取RF的状态 
}

/******************************************************************************/
//            RF_ClearStatus
//                clear RF IRQ
/******************************************************************************/
void RF_ClearStatus(void)
{
    RF_WriteReg(W_REGISTER + STATUS,0x70);							//清除RF的IRQ标志 
}

/******************************************************************************/
//            RF_ClearFIFO
//                clear RF TX/RX FIFO
/******************************************************************************/
void RF_ClearFIFO(void)
{
    RF_WriteReg(FLUSH_TX, 0);			                                		//清除RF 的 TX FIFO		
    RF_WriteReg(FLUSH_RX, 0);                                                   		//清除RF 的 RX FIFO	
}

/******************************************************************************/
//            RF_SetChannel
//                Set RF TX/RX channel:Channel
/******************************************************************************/
void RF_SetChannel( unsigned char Channel)
{    
    CE_LOW;
    RF_WriteReg(W_REGISTER + RF_CH, Channel);
}

/******************************************************************************/
//            发送数据：
//            参数：
//              1. ucPayload：需要发送的数据首地址
//              2. length:  需要发送的数据长度
//              Return:
//              1. MAX_RT: TX Failure  (Enhance mode)
//              2. TX_DS:  TX Successful (Enhance mode)
//              note: Only use in Tx Mode
//              length 通常等于 PAYLOAD_WIDTH
/******************************************************************************/
unsigned char ucRF_TxData( unsigned char *ucPayload,  unsigned char length)
{
    unsigned char   Status_Temp;
    
    RF_WriteBuf(W_TX_PAYLOAD, ucPayload, length);                               		//write data to txfifo        
    CE_HIGH;                                                                    		//rf entery tx mode start send data 
    delay_10us(4);                                                              		//keep ce high at least 16us
    CE_LOW;                                                                     		//rf entery stb3
    while(IRQ_STATUS)
    {
    	vTaskDelay(10/portTICK_RATE_MS); 
    	os_printf("\r\n 等待irq\r\n");
    }//waite irq pin low 
    Status_Temp = ucRF_ReadReg(STATUS) & 0x70;                                                  //读取发送完成后的status
    RF_WriteReg(W_REGISTER + STATUS, Status_Temp);                                 		//清除Status
    RF_WriteReg(FLUSH_TX,0);                                                   			//清 FIFO

    return Status_Temp;
}

/******************************************************************************/
//            ucRF_DumpRxData
//            读出接收到的数据：
//            参数：
//              1. ucPayload：存储读取到的数据的Buffer
//              2. length:    读取的数据长度
//              Return:
//              1. 0: 没有接收到数据
//              2. 1: 读取接收到的数据成功
//              note: Only use in Rx Mode
//              length 通常等于 PAYLOAD_WIDTH
/******************************************************************************/
unsigned char ucRF_DumpRxData( unsigned char *ucPayload,  unsigned char length)
{
    if(IRQ_STATUS)
    {
      return 0;                                                                 		//若IRQ PIN为高，则没有接收到数据
    }
    CE_LOW;
    RF_ReadBuf(R_RX_PAYLOAD, ucPayload, length);                                		//将接收到的数据读出到ucPayload，且清除rxfifo
    RF_WriteReg(FLUSH_RX, 0);	
    RF_WriteReg(W_REGISTER + STATUS, 0x70);                                     		//清除Status
    CE_HIGH;                                                                    		//继续开始接收
    
    return 1;
}


////////////////////////////////////////////////////////////////////////////////

//          以下部分与RF通信相关，不建议修改
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
//            PN006_Initial
//                Initial RF
/******************************************************************************/

void RF_Init(void)
{
    unsigned char  BB_cal_data[]    = {0x0A,0x6D,0x67,0x9C,0x46}; 
    unsigned char  RF_cal_data[]    = {0xF6,0x37,0x5D};
    unsigned char  RF_cal2_data[]   = {0x45,0x21,0xef,0x2C,0x5A,0x50};
    unsigned char  Dem_cal_data[]   = {0x01};  
    unsigned char  Dem_cal2_data[]  = {0x0b,0xDF,0x02};   
    SPI_init();
    CE_LOW;
                    
    RF_WriteReg(RST_FSPI, 0x5A);								//Software Reset    			
    RF_WriteReg(RST_FSPI, 0XA5); 
    RF_WriteReg(FLUSH_TX, 0);									// CLEAR TXFIFO		    			 
    RF_WriteReg(FLUSH_RX, 0);									// CLEAR  RXFIFO
    RF_WriteReg(W_REGISTER + STATUS, 0x70);							// CLEAR  STATUS	
    RF_WriteReg(W_REGISTER + EN_RXADDR, 0x01);							// Enable Pipe0
    RF_WriteReg(W_REGISTER + SETUP_AW,  0x03);							// address witdth is 5 bytes
    RF_WriteReg(W_REGISTER + RF_CH,     DEFAULT_CHANNEL);                                       // 2478M HZ
    RF_WriteReg(W_REGISTER + RX_PW_P0,  PAYLOAD_WIDTH);						// 8 bytes
    RF_WriteBuf(W_REGISTER + TX_ADDR,   ( unsigned char*)TX_ADDRESS_DEF, sizeof(TX_ADDRESS_DEF));	// Writes TX_Address to PN006
    RF_WriteBuf(W_REGISTER + RX_ADDR_P0,( unsigned char*)TX_ADDRESS_DEF, sizeof(TX_ADDRESS_DEF));	// RX_Addr0 same as TX_Adr for Auto.Ack   
    RF_WriteBuf(W_REGISTER + BB_CAL,    BB_cal_data,  sizeof(BB_cal_data));
    RF_WriteBuf(W_REGISTER + RF_CAL2,   RF_cal2_data, sizeof(RF_cal2_data));
    RF_WriteBuf(W_REGISTER + DEM_CAL,   Dem_cal_data, sizeof(Dem_cal_data));
    RF_WriteBuf(W_REGISTER + RF_CAL,    RF_cal_data,  sizeof(RF_cal_data));
    RF_WriteBuf(W_REGISTER + DEM_CAL2,  Dem_cal2_data,sizeof(Dem_cal2_data));
    RF_WriteReg(W_REGISTER + DYNPD, 0x00);					
    RF_WriteReg(W_REGISTER + FEATURE, 0x00);
    RF_WriteReg(W_REGISTER + RF_SETUP,  RF_POWER);						// 13DBM  		
  
    
#if(TRANSMIT_TYPE == TRANS_ENHANCE_MODE)      
    RF_WriteReg(W_REGISTER + SETUP_RETR,0x03);							//  3 retrans... 	
    RF_WriteReg(W_REGISTER + EN_AA,     0x01);							// Enable Auto.Ack:Pipe0  	
#elif(TRANSMIT_TYPE == TRANS_BURST_MODE)                                                                
    RF_WriteReg(W_REGISTER + SETUP_RETR,0x00);							// Disable retrans... 	
    RF_WriteReg(W_REGISTER + EN_AA,     0x00);							// Disable AutoAck 
#endif

if(PAYLOAD_WIDTH <33)											
{
	RF_WriteReg(W_REGISTER +FEATURE, 0x00);							//切换到32byte模式
}
else
{
	RF_WriteReg(W_REGISTER +FEATURE, 0x18);							//切换到64byte模式	   
}

}


/******************************************************************************/
//            		进入载波模式
/******************************************************************************/
void RF_Carrier( unsigned char ucChannel_Set)
{
    unsigned char BB_cal_data[]    = {0x0A,0x6D,0x67,0x9C,0x46}; 
    unsigned char RF_cal_data[]    = {0xF6,0x37,0x5D};
    unsigned char RF_cal2_data[]   = {0x45,0x21,0xEF,0xAC,0x5A,0x50};
    unsigned char Dem_cal_data[]   = {0xE1}; 								
    unsigned char Dem_cal2_data[]  = {0x0B,0xDF,0x02};    
    CE_LOW;
    RF_WriteReg(RST_FSPI, 0x5A);								//Software Reset    			
    RF_WriteReg(RST_FSPI, 0XA5);
    delay_ms(200);
    RF_WriteReg(W_REGISTER + RF_CH, ucChannel_Set);						//单载波频点	   
    RF_WriteReg(W_REGISTER + RF_SETUP, RF_POWER);      						//13dbm
    RF_WriteBuf(W_REGISTER + BB_CAL,    BB_cal_data,  sizeof(BB_cal_data));
    RF_WriteBuf(W_REGISTER + RF_CAL2,   RF_cal2_data, sizeof(RF_cal2_data));
    RF_WriteBuf(W_REGISTER + DEM_CAL,   Dem_cal_data, sizeof(Dem_cal_data));
    RF_WriteBuf(W_REGISTER + RF_CAL,    RF_cal_data,  sizeof(RF_cal_data));
    RF_WriteBuf(W_REGISTER + DEM_CAL2,  Dem_cal2_data,sizeof(Dem_cal2_data));
}

/***************************************end of file ************************************/
