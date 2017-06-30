/**
 * actel1553io.h
 * IP�˻���������������ĺ�����������س�������
 * \date 2013-6-5
 * \author xiaoqing.lu
 */
#ifndef __AT_IO_H__
#define __AT_IO_H__

#include <vector>
#include <map>
#include <queue>
#include <set>
#ifdef __vxworks
#include <unistd.h>
#endif
#include "types.h"
#include "common_qiu.h"
#include "sys/ioctl.h"
using namespace std;

#ifdef IPCORE
#define STATUS int
#endif

#ifdef DUMP_RAM  //��RAM����д���ڴ棬ֹͣ����ʱ������ļ�
void dumpRAM();
#endif

STATUS ReadRam (UINT16 cardNum, UINT16 index, UINT16 length, UINT16 *data);
STATUS WriteRam (UINT16 cardNum, UINT16 index, UINT16 length, UINT16 *data);

void AtWriteReg( WORD Bus, WORD Reg, WORD Value );
WORD AtReadReg( WORD Bus, WORD Reg );
void AtWriteRam( WORD Bus, WORD Offset, WORD Value );
WORD AtReadRam( WORD Bus, WORD Offset );
int AtWriteBlock( WORD Bus, WORD Offset, WORD* DataIn, WORD Count );
int AtReadBlock( WORD Bus, WORD Offset, WORD* DataOut, WORD Count );
void showReg();
void showRam(WORD index, WORD count);
void showCnt();
void ClearCnt();



/*IP���ڲ��Ĵ���*/
#define REG_CTRL		00/*Control*/
#define REG_OPSTAT		01/*Operation and Status*/
#define REG_CMD			02/*Current Command*/
#define REG_INT_MASK	03/*Interrupt Mask*/
#define REG_INT_PEND	04/*Pending Interrupt*/
#define REG_INT_PT		05/*Interrupt Pointer*/
#define REG_BIT			06/*Build-In Test register*/
#define REG_TT			07/*Minor Frame Timer*/
#define REG_BLK_PT		08/*Command Block Pointer*/
#define REG_EN_FEAT		32/*Enhanced Features*/

//cs_n_1553_3 ����ΪBC(CORE 1)
#define AT_BC_BASE_RAM_ADDR_BUS0	    (0x00000000)  /*ͨ��0RAM����ַ*/
#define AT_BC_BASE_RAM_ADDR_BUS1	    (0x00010000)  /*ͨ��1RAM����ַ*/
#define AT_BC_BASE_REG_ADDR_BUS0		(0)  /*ͨ��0�Ĵ�������ַ*/
#define AT_BC_BASE_REG_ADDR_BUS1		(0x00030000)  /*ͨ��1�Ĵ�������ַ*/

#define AT_BC_MIN_FRAME_TIME	800	
#define AT_RAM_SIZE				(2048-32)   //zhanghao: ���һ����Ŀǰ�߼������⣬��ʱ����     0x00010000/*64KW  luxq*/

#define MT_BLOCK_COUNT 	40
#define MT_BUFF_SIZE 		(MT_BLOCK_COUNT * 8) //MT�ɻ���MT_BLOCK_COUNT����Ϣ

/*��д�ڴ溯����װ����Ӧ�ڵײ�����*/
#define   MODEMDEVICE 	 "/dev/memory_device"
extern void open_device(void);
extern void close_device(void);

/*1553B������*/
typedef union
{
	WORD CommanWORD;
	struct
	{
		unsigned short DataNum	:5;
		unsigned short SubAddr	:5;
		unsigned short TRFlag	:1;
		unsigned short RTAddr	:5;
	} Command;
} AT_MSG_COMMAND;

/*1553B״̬��*/
typedef union
{
	WORD StatusWord;
	struct
	{
		unsigned short Terminal	:1;
		unsigned short DynBusAcept:1;
		unsigned short SsysFlag	:1;
		unsigned short Busy		:1;
		unsigned short BcastRecv	:1;
		unsigned short 			:3;
		unsigned short SvcReq		:1;
		unsigned short Instrument	:1;
		unsigned short MsgError	:1;
		unsigned short RTAddr		:5;
	} Status;
} AT_MSG_STATUS;


#endif /* __AT_IO_H__ */
