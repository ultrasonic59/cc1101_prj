#ifndef __PRINTK_H__
#define __PRINTK_H__
#define __DEBUG

////=============================================
#ifdef __DEBUG
#include <stdio.h>

////  extern int _printk(const char *format, ...);
  #define printk	printf
#else
  extern int _printk_dummy(const char *format, ...);
  #define printk	_printk_dummy1
#endif

////=============================================
#endif ////__PRINTK_H__



	
