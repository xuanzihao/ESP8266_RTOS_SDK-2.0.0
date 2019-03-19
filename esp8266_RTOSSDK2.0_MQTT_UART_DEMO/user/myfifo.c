#include "myfifo.h"

uint8_t  FIFO_Init( fifo_t *fifo, uint8_t *buf, uint32_t buf_size )
{
    if( fifo == NULL )
        return 0;
    if( buf == NULL )
        return 0;
    if( buf_size < 3 )
        return 0;

    fifo->start        = buf;
    fifo->end          = buf + buf_size -1;
    fifo->input_point  = buf;
    fifo->output_point = buf;
    fifo->free_size    = buf_size;

    while( buf_size )
        buf[--buf_size] = 0;

    return 1;
}

uint8_t  FIFO_WriteData( fifo_t *fifo, uint8_t *dat, uint16_t len )
{
    uint8_t   *p;

    if( len == 0 )
        return 0;

    enter_critical();

    if( len > fifo->free_size - 2 - 1 )     // Leave 1 byte blank to prevent input_point == output_point
    {
        exit_critical();
        return 0;
    }

    fifo->free_size = fifo->free_size - len - 2;

    p = fifo->input_point;

    *p++ = len >> 8;
    if( p > fifo->end )
        p = fifo->start;

    *p++ = len & 0x00FF;
    if( p > fifo->end )
        p = fifo->start;

    while( len-- )
    {
        *p++ = *dat++;
        if( p > fifo->end )
            p = fifo->start;
    }

    fifo->input_point = p;

    exit_critical();
    return 1;
}

uint16_t  FIFO_ReadData( fifo_t *fifo, uint8_t *dat )
{
    uint8_t   *p;
    uint16_t   i, len;

    enter_critical();

    if( fifo->output_point == fifo->input_point )
    {
        exit_critical();
        return 0;
    }

    p = fifo->output_point;

    len = (uint16_t)*p << 8;
    *p++ = 0;
    if( p > fifo->end )
        p = fifo->start;

    len += *p;
    *p++ = 0;
    if( p > fifo->end )
        p = fifo->start;

    i = len;
    while( i-- )
    {
        *dat++ = *p;
        *p++   = 0;
        if( p > fifo->end )
            p = fifo->start;
    }

    fifo->output_point = p;
    fifo->free_size = fifo->free_size + len + 2;

    exit_critical();
    return len;
}

uint8_t FIFO_IsDataExit( fifo_t *fifo )
{
    enter_critical();
    if( fifo->output_point != fifo->input_point )
    {
        exit_critical();
        return 1;
    }
    else
    {
        exit_critical();
        return 0;
    }
}

