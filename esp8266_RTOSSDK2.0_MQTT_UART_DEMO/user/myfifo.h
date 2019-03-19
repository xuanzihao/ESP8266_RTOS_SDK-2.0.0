#ifndef __myfifo__
#define __myfifo__

#include "esp_common.h"

#define enter_critical()            {;}//GLOBAL_INT_DISABLE()
#define exit_critical()             {;}//GLOBAL_INT_RESTORE()


typedef struct  __fifo_t
{
    uint8_t    *start;
    uint8_t    *end;
    uint8_t    *input_point;
    uint8_t    *output_point;
    int32_t     free_size;
} fifo_t;


uint8_t   FIFO_Init( fifo_t *fifo, uint8_t *buf, uint32_t buf_size );
uint8_t   FIFO_WriteData( fifo_t *fifo, uint8_t *dat, uint16_t len );
uint16_t  FIFO_ReadData( fifo_t *fifo, uint8_t *dat );
uint8_t   FIFO_IsDataExit( fifo_t *fifo );






#endif

