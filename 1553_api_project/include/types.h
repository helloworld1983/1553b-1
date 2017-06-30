/*	
 * types.h
 * ���ݻ������͵�һЩ���Ͷ���
 * \date 2013-6-5
 * \author xiaoqing.lu
 */

#ifndef _TYPES_H
#define _TYPES_H
//#define DUMP_RAM 1

/******************************************************************************
                                    ��������
******************************************************************************/

typedef unsigned short        U16BIT;
typedef unsigned long long U64BIT;
typedef short        S16BIT;
typedef unsigned int        U32BIT;
typedef int        S32BIT;

//typedef unsigned char  BYTE;                    /* �޷���8λ�� */
//typedef unsigned char  byte;               		/* �޷���8λ�� */
//typedef signed   char  CHAR;                    /* �з���8λ�� */
//typedef unsigned int WORD;                    /* �޷���16λ�� */
//typedef signed   short SHORT;                   /* �з���16λ�� */
/* DSP6203��long��8�ֽ�, by WANG Zhiqiang, 2009-03-25, long->int */
//typedef unsigned int  DWORD;                   /* �޷��ų������� */
/* DSP6203��long��8�ֽ�, by WANG Zhiqiang, 2009-03-25 */
//typedef signed   long  LONG;                    /* �з��ų������� */
//typedef signed   int  INT;						/* ������ */
//typedef float   FLOAT;                          /* �����ȸ����������� */
//typedef double  DOUBLE;                         /* ˫���ȸ����������� */

/* WANG Zhiqiang, 2009-03-26 */
//typedef unsigned char  BOOLEAN;
//typedef unsigned char  INT8U;                    /* Unsigned  8 bit quantity        */
//typedef signed   char  INT8S;                    /* Signed    8 bit quantity        */
//typedef unsigned short INT16U;                   /* Unsigned 16 bit quantity        */
//typedef signed   short INT16S;                   /* Signed   16 bit quantity        */
//typedef unsigned int   INT32U;                   /* Unsigned 32 bit quantity        */
//typedef signed   int   INT32S;                   /* Signed   32 bit quantity        */
//typedef float          FP32;                     /* Single precision floating point */
//typedef double         FP64;                     /* Double precision floating point */

/* ------- zhanghao add.(begin) ----- */
#define ASSERT_TRUE(x) if(!(x)) return

//#define TRUE true
//#define FALSE false

//typedef  int			STATUS;
typedef  unsigned short	WORD;
typedef  unsigned short	UINT16;
typedef  unsigned int	UINT32;
//typedef  bool			BOOL;
typedef  short			SHORT;
typedef  unsigned short USHORT;
typedef  short			S16BIT;
typedef  unsigned short	U16BIT;
//typedef  	size_t		ssize_t;                            //qiubenqi zhu
typedef long long int64_t;   //zhanghao add.
/* ------- zhanghao add.(end) ----- */

#define RET_1553B_OK    0
#define RET_1553B_ERR   1

#define _DECL 

#endif /* _TYPES_H */
