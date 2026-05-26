#include <stdio.h>
#include <stdint.h>

////extern void my_uart_tx(uint8_t idat);
extern int send_char_dbg(int c) ;

size_t __write(int handle, const unsigned char * buffer, size_t size)
{
#if 1
    int i;
////    UNUSED_PARAMETER(handle);
    for (i = 0; i < size; i++)
    {
///     my_uart_tx(*buffer++);
     send_char_dbg(*buffer++);
     
    }
#endif
    return size;
}
