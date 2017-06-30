/**
* comm_bc.h
* BC�����ù��ߺ��������Ľӿ��ļ�
*
*    \date 2003-8-24
*  \author xiaoqing.lu
*/

#ifndef _COMM_BC_H_
#define _COMM_BC_H_

/**
* ��Ϣ����
*/
typedef enum _enumMsgType
{
	MSG_BC_TO_RT = 0,			///< BC->RT��Ϣ
	MSG_RT_TO_BC = 1,			///< RT->BC��Ϣ
	MSG_RT_TO_RT = 2,			///< RT->RT��Ϣ
	MSG_MODE_CODE = 3,			///< ��ʽ������Ϣ
	MSG_BROADCAST = 4,			///< BC->RT�㲥��Ϣ
	MSG_RT_BROADCAST = 5,		///< RT->RT�㲥��Ϣ
	MSG_MODE_CODE_BROADCAST = 6,///< ��ʽ����㲥��Ϣ
	MSG_OPCODE_IRQ = 7,			///< �жϲ���
}
enumMsgType;

/**
* ��Ϣ����ͨ��
*/
typedef enum _enumChannel
{
	CHANNEL_A = 0,	///< Aͨ��
	CHANNEL_B = 1,	///< Bͨ��
	CHANNEL_AB = 2,	///< ABͨ������
}
enumChannel;

/**
* ��Ϣ����ʱ��
*/
typedef enum _enumSendTiming
{
	SENDTIMING_ALWAYS = 0,			///< �̶�����
	SENDTIMING_FOLLOW_SYNC = 1,		///< ���ݱ����Ϣ�����ж��Ƿ���_ͬ��
	SENDTIMING_FOLLOW_ASYNC = 2,	///< ���ݱ����Ϣ�����ж��Ƿ���_�첽
	SENDTIMING_DUP = 3,				///< ����ע��
}
enumSendTiming;

/**
* ��Ϣ������
*/
class BC_Msg_info
{
public:
	char name[66];
	short id;					///< ��Ϣid
	short blkid;				///< ���ݿ�id
	enumMsgType type;			///< ��Ϣ����
	BOOL isAsync;				///< �Ƿ��첽����
	unsigned int gap_time;		///< ��ʱ
	short length;				///< ��Ϣ���ȣ���ʽ�����и���Ϊ��ʽ��
	unsigned short data[32];	///< ����
	enumChannel channel;		///< ͨ��
	BOOL isRetry;				///< �Ƿ�����
	short Addr[4];				///< Դ��ַ��Դ�ӵ�ַ��Ŀ�ĵ�ַ��Ŀ���ӵ�ַ
	enumSendTiming sendTiming;	///< ����ʱ��
	unsigned int time;			///< �����Է�����Ϣ������
	short followMsgID;			///< �����ĸ���Ϣ�ж��Ƿ�Ҫ��
	unsigned short followValue;	///< �ж�ֵ
	unsigned short followMask;	///< �ж�MASK
	short frameIndex;			///< ����Ϣ�����ĸ�С֡
 	short followDataindex;		///< �жϸ���Ϣ�ĵڼ���������
	BOOL isCheckSrvbit;			///< �Ƿ��ж���Ϣ�ķ�������λ
	BOOL isCanMiss;				///< �Ƿ���©��ͣ��
	BOOL isTimeCode;			///< �Ƿ���ʱ����
	BOOL isCheckCRC;			///< �Ƿ����У��
	short CRCMode;				///< У�鷽ʽ���ۼӺͻ�CRC
	short CRCTiming;			///< У��ʱ������Ϣĩβ���ĩβ
	BOOL isFromFile;			///< �Ƿ��з����ļ�
	short packLen;				///< ������,��λ����
//	unsigned char defaultValue;		///< �����β����Ĭ�����ֵ
//	unsigned char reverse;			///< ����λ
	unsigned short defaultValue;	///< �����β����Ĭ�����ֵ
	char filename[128];			///< �����ļ���
};

/// �����ļ������Ը��ַ�����ͷ��������Ϊ��Ч�ļ�
#define CHECK_STRING "BCConfcheck"
#endif
