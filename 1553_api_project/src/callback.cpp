#include <stdio.h>
#include <stdlib.h>
#include "zynq_1553b_api.h"
#include "callback.h"


/*�ص�����*/
ZYNQ_MSG_CALLBACK  g_pfucCallback = NULL;

/******************************************************************
*������ ��zynq_reg_func 
*������������ ���ص�����������
*�������� ��pfunc���ص�����ԭ����
*��������ֵ ����
********************************************************************/
void zynq_reg_func(ZYNQ_MSG_CALLBACK pfunc)
{
		g_pfucCallback = pfunc;
}
