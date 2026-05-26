#ifndef _RINGBUFFER_H__
#define _RINGBUFFER_H__
#include <stdint.h>
///=======================================================
typedef struct _ringbuffer
{
uint8_t  *buffer;
uint16_t put_index;
uint16_t get_index;
uint16_t size;
uint16_t cnt;
}ringbuffer_t;

extern uint8_t*  ringbuffer_init(ringbuffer_t *rbuffer,int size);
extern void  ringbuffer_clr(ringbuffer_t *rbuffer);

extern ///inline 
int ringbuffer_putc(ringbuffer_t* rbuffer,char ch);
extern ///inline 
int ringbuffer_getc(ringbuffer_t *rbuffer ,uint8_t *o_ch);
extern ////inline 
uint16_t get_ringbuffer_cnt(ringbuffer_t *rbuffer);

#if 0
extern 
inline 
uint16_t get_ringbuffer_cnt(ringbuffer_t *rbuffer)
{
///int level;
uint16_t cnt;
///    level = rt_hw_interrupt_disable();
cnt = rbuffer->cnt;
////    rt_hw_interrupt_enable(level);
return cnt;
}
#endif
///======================================================
#endif