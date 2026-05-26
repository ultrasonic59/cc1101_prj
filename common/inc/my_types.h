#ifndef _MY_TYPES_H_
#define _MY_TYPES_H_

#ifndef MIN
#define MIN(a,b) (a<b?a:b)
#endif
typedef void (*void_fn_void)(void);

typedef void (*void_fn_u8)(uint8_t);
typedef void (*void_fn_int)(int);
typedef void (*void_fn_pvoid)(void*);
typedef int (*int_fn_u8_pvoid)(uint8_t,void *);
typedef int (*int_fn_pvoid)(void*);
typedef int (*int_fn_void)(void);
typedef uint8_t (*u8_fn_void)(void);
typedef uint8_t (*u8_fn_u8)(uint8_t);
typedef uint16_t (*u16_fn_void)(void);
typedef uint16_t (*u16_fn_pu8)(uint8_t*);
typedef uint16_t (*u16_fn_u8_pu8)(uint8_t,uint8_t*);
typedef uint16_t (*u16_fn_u8_u16_pu8)(uint8_t,uint16_t,uint8_t*);
///typedef void (*void_fn_int)(int);
///typedef void (*void_fn_pvoid)(void*);
typedef uint32_t (*u32_fn_void)(void);
typedef int16_t (*s16_fn_void)(void);
typedef uint32_t (*u32_fn_s16)(int16_t);
typedef void* (*pvoid_fn_void)(void);
typedef int32_t (*s32_fn_void)(void);
typedef void (*void_fn_u8_pvoid)(uint8_t,void *);
typedef uint8_t (*u8_fn_pvoid_pu8_u16)(void*,uint8_t*,uint16_t);
typedef uint8_t (*u8_fn_pu8_u16)(uint8_t*,uint16_t);
__packed typedef struct dp_dat_s
{
uint8_t flags;  
int16_t dp_al;
int16_t dp_ar;
int16_t dp_bl;
int16_t dp_br;
}dp_dat_t;

#if 0
__packed typedef struct par_strb_s
{
uint8_t on_strb;
uint8_t	por;
////u16 offs_type;	///
uint8_t dn_up;	///1- down 0 - up
uint8_t on_vibro;	///1-on ,o-off
uint16_t beg;
uint16_t len;
}par_strb_t;
#endif
///====================
#endif	///_ROOK_TYPES_H_
