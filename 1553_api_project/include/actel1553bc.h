/**
 * actel1553bc.h
 * BC�ṹ�͹��ܺ�������
 * \date 2013-6-5
 * \author xiaoqing.lu
 */
#ifndef _1553COREBC_H_
#define _1553COREBC_H_
#include "actel1553io.h"

#define SIZEOFCMDBLK 8  //����鳤��



//������
typedef enum
{
	OP_CODE_EOL,//0000-End of List//
	OP_CODE_SKIP,//0001-Skip//
	OP_CODE_GOTO,//0010-Go To//
	OP_CODE_BIT,//0011-Build-in Test//
	OP_CODE_EXEC,//0100-Execute Block Continue//
	OP_CODE_EXEB,//0101-Execute Block Branch//
	OP_CODE_EXEBOC,//0110-Execute Block-Branch on Condition//
	OP_CODE_ROC,//0111-Retry on Condition//
	OP_CODE_ROCB,//1000-Retry On Condition-Branch//
	OP_CODE_ROCBARF,//1001-Retry on Condition Branch if All Retries Fail//
	OP_CODE_INTC,//1010-Interrupt �C Continue//
	OP_CODE_CALL,//1011-Call//
	OP_CODE_RTCALL,//1100-Return to Call//
	OP_CODE_RESERVED,//1101-Reserved//
	OP_CODE_LMFT,//1110-Load Minor Frame Time//
	OP_CODE_RTB//1111-Return to Branch//
}OP_CODE;

//1553B������
typedef union
{
	WORD ControlWord;
	
	struct
	{
		unsigned short	BAME		         : 1;/**/
		unsigned short	Condition_Code		 : 7;/**/
		unsigned short	RT_RT		         : 1;/*�Ƿ�RT��RT����Ϣ�������ڶ����������Ƿ��ͣ�*/
		unsigned short	Channel_AorB	     : 1;/*��������A��B��ΪPrimary����*/
		unsigned short	Retry			     : 2;/*���Դ�����Reg0-bit2������A��B���ԣ�*/
		unsigned short	OP_Code		         : 4;/*������*/
	} Control;
} AT_MSG_CONTROL;

//�����
typedef struct
{
	AT_MSG_CONTROL CtrlWord;
	AT_MSG_COMMAND CmWORD1;
	AT_MSG_COMMAND CmWORD2;
	WORD		   DataPtr;
	AT_MSG_STATUS  StatWord1;
	AT_MSG_STATUS  StatWord2;
	WORD		   BranchAddr;
	WORD		   TimerVal;
}AT_COMMAND_BLOCK;

//���ݿ�
class AT_DATA_BLK{
public:
	AT_DATA_BLK(){
		dataLen = 32;
	}
	AT_DATA_BLK(WORD* p, WORD len=32){
		if(len > 32){
			return;
		}
		for(int i=0; i<len; i++){
			data[i] = p[i];
		}
		dataLen = len;
	}
	WORD data[32];
	WORD dataLen;
	void operator=(AT_DATA_BLK datablk){
		for(int i=0; i<32; i++){
			data[i] = datablk.data[i];
		}
		dataLen = datablk.dataLen;
	}
};

int AtBCMakeControl( AT_COMMAND_BLOCK &CmdBlk, U32BIT dwMsgOptions,WORD isRT_RT );
int AtBCDefMsg( WORD Bus, unsigned char MsgNo, unsigned char DataNo, WORD wMsgGapTime, AT_COMMAND_BLOCK* MsgBlk );
int AtBCDefAsyncMsg( WORD Bus, unsigned char MsgNo, unsigned char DataNo, WORD wMsgGapTime, AT_COMMAND_BLOCK* MsgBlk );

#endif

