#ifndef __USB_CMD_H_
#define __USB_CMD_H_ 
///========================================
#include <stdint.h>

#define CDC_CMD_PUT			0x3		/// 
#define CDC_CMD_REQ			0x4		/// 

#define NUM_SET_AIR			0x11		/// 
#define NUM_SET_GAZ			0x12		/// 
#define NUM_SET_CL3			0x13		/// 
#define NUM_SET_CL7			0x14		/// 
#define NUM_STOP_ALL			0x15		
#define NUM_SET_RED			0x16		/// 
#define NUM_SET_GREEN			0x17		/// 
#define NUM_SET_CALIBR                  0x18        /// 

#define GET_TEMP_GAZ			0x21
#define GET_TEMP_AIR			0x22
#define GET_RASH_AIR			0x23
#define GET_STAT			0x24
#define GET_ALL_DATA                    0x25

///========== for debug ==================
#define SET_REQ_ADDR                      0x32
#define GET_PCHV_COIL_DATA                0x33
#define GET_PCHV_WORD_DATA                0x34
#define SET_PCHV_COIL_DATA                0x35
#define SET_PCHV_WORD_DATA                0x36
///=========================================

#endif ///__USB_CMD_H_
