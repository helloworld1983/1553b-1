#pragma once

#include "head.h"
class ISR_Thread : public Thread{
public:
	ISR_Thread(){
	}
	ISR_Thread(S16BIT DevNum){
		this->DevNum = DevNum;
	}
	SyncEvent syncEvent;
	SyncEvent* MT_sync; //MTʹ��Adapter�����sync��������ʱ�����ж϶�����signal��
	void do_job();
private:
	S16BIT DevNum;
};

///��¼ÿ��ͨ���õ��ı���
class CDeviceInfo
{
public:
	CDeviceInfo(void);
	CDeviceInfo(S16BIT DevNum);
	~CDeviceInfo(void);

	S16BIT DevNum;///< ͨ���ţ���0��ʼ�������������������ͨ����

	U32BIT IPCORE_wIntPtr;	///< �ж���־ָ��
	int is_IPCORE_start;	///< �Ƿ�ʼ����
	WORD IPCORE_mode;		///< BC RT MT


	void (*g_1553B_usrISR)( S16BIT DevNum, U32BIT dwIrqStatus );
	ISR_Thread isr_thread;

	///BC
	///������֡ǰ��Ϣ������ݿ��һЩȫ�ֱ���
	vector<AT_COMMAND_BLOCK> BC_vec_CmdBlk; ///< ������Ϣ��������ʱ����飬������첽��Ϣ��Ҫ����ret call commad
	map<WORD, WORD> BC_map_msgNo_cmdIndex; ///< <ͬ����Ϣ�ţ���Ϣ�����index>
	map<WORD, WORD> BC_map_AsyncMsgNo_cmdIndex; ///< <�첽��Ϣ�ţ���Ϣ�����index>
	map<WORD, AT_DATA_BLK> BC_map_dataNo_data; ///< <���ݿ�ţ����ݿ�> ���ݿ�ŵ�ֵ����MsgBlk->DataPtr��

	///������֡��һЩȫ�ֱ���,��֡���֮��Ͳ�������
	map<WORD, WORD> BC_map_msgCode_msgNo;///< <��Ϣ�����룬��Ϣ��>
	map<WORD, vector<WORD> > BC_map_minFraID_msgCodes;///< <С֡�ţ���Ϣ������>
	map<WORD, WORD> BC_map_minFraID_minFraTime; ///< <С֡�ţ�С֡����>
	map<WORD, WORD> BC_map_minFraCode_minFraID;///< <С֡�����룬С֡��>

	///������֡��RAM����Ϣ������ݿ�ĵ�ַһЩȫ�ֱ���
	set<pair<WORD,WORD> > BC_set_msgNo_blkAddr; ///< <��Ϣ�ţ���Ϣ���RAM��ַ>ȫ����
	map<WORD, WORD> BC_map_msgNo_ifCallBlkAddr; ///< <�첽��Ϣ�ţ��첽��Ϣ���ÿ��RAM��ַ>
	map<WORD, WORD> BC_map_dataNo_dataAddr; ///< <���ݿ�ţ����ݿ��RAM��ַ>

	///����ִ�й�����Ϣ��һЩȫ�ֱ���
	map<WORD, queue<pair<AT_COMMAND_BLOCK,AT_DATA_BLK> > > BC_map_msgNo_blkQueue; ///< <��Ϣ�ţ���Ϣ��Ķ���>BC��Ϣ��д��Ͷ�������һ���߳�����ɵģ����Բ��ü���
	map<WORD, vector<pair<WORD,WORD> > > BC_map_IntBlkAddr_msgAddr;///< <С֡ĩ���첽��Ϣĩ�жϿ��ַ��vec<��Ϣ��, ��Ϣ��RAM��ַ>>

	///�����첽��Ϣ�ķ�������
	map<WORD, queue<AT_DATA_BLK> > BC_map_AsyncMsgNo_dataQueue; ///< <�첽��Ϣ�ţ����첽��Ϣ�����ݶ���>

	///RT
	map<WORD, WORD> RT_map_dataID_dataAddr; ///< <���ݿ�ID, ����RAM��ַ> 
	map<WORD, WORD> RT_map_dataID_dspBlkAddr; ///< <���ݿ�ID, ��Ӧ���������ַ>
	queue<MSGSTRUCT> RT_queue_MSG; ///< ����1553��Ϣ��д��Ͷ�������һ���߳���ģ��������������

	///MT
	queue<MSGSTRUCT> MT_queue_MSG; ///< ����1553��Ϣ
	MXLock MT_queue_MSG_lock; ///< MT_queue_MSG����Ϣд��Ͷ������������߳��У�������Ҫ����
	short MT_msg_now; ///< MT�����жϼ���ѯ�ķ�ʽ�����ݣ��ñ�����¼������Ϣ�ĵ�ǰλ��
};
