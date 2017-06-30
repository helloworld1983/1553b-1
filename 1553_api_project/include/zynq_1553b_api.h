#ifndef __ZYNQ_1553B_API_H__
#define __ZYNQ_1553B_API_H__

#include <string>
using namespace std;

/*�Ƿ񱣴����ݵ�ö��*/
typedef enum _enum_msg_is_saves
{
	ENUM_MSG_SAVE          = 0,  /*������Ϣ*/
	ENUM_MSG_NOT_SAVE  = 1, /*��������Ϣ*/
}msg_is_save;

/*������Ϣ��ö��*/
typedef enum _enum_operate_type
{
	ENUM_DEL_MSG         = 0,     /*ɾ����Ϣ�Ĳ���*/
	ENUM_ADD_MSG        = 1,	   /*�����µ���Ϣ*/
	ENUM_MODIFY_MSG   = 2,   /*�޸���Ϣ�Ĳ���*/
} enum_operate_type;

/*��Ϣ�ṹ�壬���������֣������֣�״̬�ֵ���Ϣ*/
typedef struct msg_struct
{
	unsigned short  cmdwrd;              /*������*/
	unsigned short  iaddrs;               /*RT��ַ*/
	unsigned short  trflag;               /*���������*/
	unsigned short	  subaddr;          /*�ӵ�ַ*/
	unsigned short  datalen;          /*���ݳ���*/
	unsigned short  stswrd;           /*״̬��*/
	unsigned short  datawrd[32];   /*������*/
}stmsg_struct;

/*��Ҫ�޸ĵ���Ϣ�ṹ��*/
typedef struct ctl_data_wrd_info
{
       enum_operate_type type;
	size_t msg_lenth;
	unsigned short msg_id;
	unsigned short pos;
	char *data;
} ctl_data_wrd_info;

#define RET_OK     0  /*�ɹ�*/
#define RET_ERR   1  /*ʧ��*/

/*������ʵʱ����غ���*/
typedef  void (*ZYNQ_MSG_CALLBACK)(stmsg_struct *pmsg);
extern void zynq_reg_func(ZYNQ_MSG_CALLBACK pfunc);
/*BC��صĲ�������*/
extern int  zynq_bc_set_config(const std::string cfg);
extern int  zynq_bc_init(void);
extern int  zynq_bc_start(void);
extern int zynq_bc_stop(void);
extern int zynq_bc_msg_write(ctl_data_wrd_info *datahead);
extern void zynq_bc_ioctl(unsigned short msg_id, msg_is_save flag, const char *save_path);
/*RT��صĲ�������*/
extern int zynq_rt_set_config(const std::string cfg);
extern int zynq_rt_init(void);
extern int zynq_rt_start(void);
extern int zynq_rt_stop(void);
extern int zynq_rt_msg_write(unsigned short subaddr, const char *data, size_t len);
extern void zynq_rt_ioctl(unsigned short subaddr, msg_is_save flag, const char *save_path);
/*MT��صĲ�������*/
extern int  zynq_mt_init(void);
extern int  zynq_mt_start(void);
extern int zynq_mt_stop(void);
extern int zynq_mt_msg_save(const char *save_path);

#endif
