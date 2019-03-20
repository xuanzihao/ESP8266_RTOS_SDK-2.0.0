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


///////////////////////////////////////////////////////////////////////////////
//                  根据用户应用，以下部分可能需要修改                       //
///////////////////////////////////////////////////////////////////////////////
/******************** Control I/O *****************************/
#include<STM8S103F.h>
//#include "stm8l15x_gpio.h"

//#define         IRQ_STATUS       GPIOD->IDR &  (unsigned char)GPIO_Pin_0
//#define         CE_HIGH          GPIOB->ODR |= GPIO_Pin_1
//#define         CE_LOW           GPIOB->ODR &= (~GPIO_Pin_1)
//#define         CSN_HIGH         GPIOB->ODR |= GPIO_Pin_4
//#define         CSN_LOW          GPIOB->ODR &= (~GPIO_Pin_4)
//#define         SCK_HIGH         GPIOB->ODR |= GPIO_Pin_5
//#define         SCK_LOW          GPIOB->ODR &= (~GPIO_Pin_5)
//#define         MOSI_HIGH        GPIOB->ODR |= GPIO_Pin_6;
//#define         MOSI_LOW         GPIOB->ODR &= (~GPIO_Pin_6)
//#define         MISO_STATUS      GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_7)


////////////////////////////////////////////////////////////////////////////////
//                    以下为声明部分，不建议修改                              //
////////////////////////////////////////////////////////////////////////////////
#define         TRANS_ENHANCE_MODE    1                                         //传输类型增强型
#define         TRANS_BURST_MODE      2                                         //传输类型普通型

#define         RF13dBm                         0x3F                            // 13dBm 
#define         RF10dBm                         0X0F                            // 10dBm 
#define         RF8dBm                          0x15                            // 8dbm      
#define         RF7dBm                          0x07                            // 7dbm   
#define         RF5dBm                          0x2c                            // 5dbm   
#define         RF4dBm                          0x06                            // 4dbm   
#define         RF2dBm                          0x05                            // 2dbm  
#define         RF0dBm                          0X0B                            // 0dBm  
#define         RF_3dBm                         0x04                            // -3dBm     
#define         RF_6dBm                         0x0A                            // -6dBm 
#define         RF_10dBm                        0x02                            // -10dBm 
#define         RF_18dBm                        0x01                            // -18dBm 
#define         RF_30dBm                        0x00                            // -30dBm 


/********************SPI  REGISTER  ********************/
#define		R_REGISTER			0x00                            //SPI read RF data
#define		W_REGISTER			0x20                            //SPI write RF data
#define		R_RX_PAYLOAD		        0x61                            //Read RX Payload
#define		W_TX_PAYLOAD		        0xA0                            //Write TX Payload
#define		FLUSH_TX			0xE1                            //Flush RX FIFO
#define		FLUSH_RX			0xE2                            //Flush TX FIFO
#define		REUSE_TX_PL			0xE3                            //Reuse TX Payload
#define		ACTIVATE			0x50                            //ACTIVATE
#define		DEACTIVATE			0x50                            //DEACTIVATE
#define		R_RX_PL_WID			0x60                            //Read width of RX data 
#define		W_ACK_PAYLOAD		        0xA8                            //Data with ACK
#define		W_TX_PAYLOAD_NOACK	        0xB0                            //TX Payload no ACK Request
#define		CE_FSPI_ON	                0xFD                            // CE HIGH
#define		CE_FSPI_OFF	                0xFC                            // CE LOW
#define		RST_FSPI	                0x53                            // RESET
#define		NOP_N				0xFF
             
/******************CONTROL  REGISTER*******************/
#define		CONFIG                          0x00            
#define		EN_AA				0x01
#define		EN_RXADDR			0x02
#define		SETUP_AW			0x03
#define		SETUP_RETR			0x04
#define		RF_CH				0x05
#define		RF_SETUP			0x06
#define		STATUS				0x07
#define		OBSERVE_TX			0x08
#define		RPD			        0x09
#define		RX_ADDR_P0			0x0A
#define		RX_ADDR_P1			0x0B
#define		RX_ADDR_P2			0x0C
#define		RX_ADDR_P3			0x0D
#define		RX_ADDR_P4			0x0E
#define		RX_ADDR_P5			0x0F
#define		TX_ADDR				0x10
#define		RX_PW_P0			0x11
#define		RX_PW_P1			0x12
#define		RX_PW_P2			0x13
#define		RX_PW_P3			0x14
#define		RX_PW_P4			0x15
#define		RX_PW_P5			0x16
#define		FIFO_STATUS			0x17
#define		DEM_CAL				0x19
#define         RF_CAL2				0x1A
#define         DEM_CAL2			0x1B
#define		DYNPD				0x1C
#define		FEATURE				0x1D	
#define		RF_CAL				0x1E
#define		BB_CAL				0x1F

/////////////////////////////////////////////////////////////////////////////////////
/*************************CONTROL CMD****************************************/
#define         DR_1M              0X00				//通信速率 1Mbps
#define         DR_2M              0X40				//通信速率 2Mbps
#define         DR_250K            0XC0      //通信速率 250Kbps
#define         RX_DR_FLAG         0X40      // 接收中断标志位
#define         TX_DS_FLAG         0X20      // 发送完成中断标志位
#define         RX_TX_CMP_FLAG     0X60  //发送接收完成中断标志位，ack_payload 模式下使用
#define         MAX_RT_FLAG        0X10   //发送重传超时中断标志位
#define         TRANS_ENHANCE_MODE 1      //传输类型普通型

///////////////////////////////////////////////////////////////////////////////
//                  根据用户应用，以下部分可能需要修改                       //
///////////////////////////////////////////////////////////////////////////////
/******************* Function declare *******************/
unsigned char 	SPI_RW( unsigned char	 R_REG);
unsigned char 	RF_ReadReg( unsigned char reg);
void 		RF_WriteReg( unsigned char reg,  unsigned char wdata);
void 		RF_ReadBuf( unsigned char reg,  unsigned char *pBuf,  unsigned char length);
void 		RF_WriteBuf( unsigned char reg,  unsigned char *pBuf,  unsigned char length);
void 		RF_Init(void);                                                     
void 		RF_TxMode(void);
void	 	RF_RxMode(void);
unsigned char 	ucRF_GetStatus(void);
void 		RF_ClearStatus(void);
void 		RF_ClearFIFO(void);
void 		RF_SetChannel( unsigned char Channel);
unsigned char 	ucRF_TxData( unsigned char *ucPayload,  unsigned char length);
unsigned char 	ucRF_DumpRxData( unsigned char *ucPayload,  unsigned char length);
void 		RF_Carrier( unsigned char ucChannel_Set);


 unsigned char	ucRF_ReadReg( unsigned char reg);
void delay_ms(unsigned char time);
/******************Parameter define **********************/
        

#define         DEFAULT_CHANNEL                78				//初始化时的频率： 2478 MHz           
#define         PAYLOAD_WIDTH                  10				//Payload宽度：     8bytes				
#define         TRANSMIT_TYPE                  TRANS_BURST_MODE 		//使用增强型模式           TRANS_BURST_MODE    TRANS_ENHANCE_MODE
#define         DATA_RATE                      DR_1M                            //通信速率2Mbps            DR_2M    DR_1M
#define         RF_POWER                      (RF13dBm |DATA_RATE)		//发射功率13dBm 


/****************************************end of file********************************************/

