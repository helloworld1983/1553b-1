/*	
 * rt.cpp
 * ʵ������RT���ܵ�һЩ�ӿ�ʵ��
 * \date 2013-6-5
 * \author xiaoqing.lu
 */

#include "zynq_1553b_api.h"
#include "IPCORE.h"
#include "head.h"
#include "common_qiu.h"
#include <bitset>

extern map<S16BIT, CDeviceInfo*> map_devInfo;
/* 
 * RAMʹ�ý��ܣ�
 * 
 * ÿһ���ӵ�ַ��Ӧ�շ��������ݿ�ID��
 * ÿһ�����ݿ�ID��Ӧһ�������飻
 * ÿһ������������A��B��������ָ�룬��Ӧ��������RAM��ַ���ǰ��ŷŵģ���¼���ǵ�һ���ĵ�ַ��
 * ���������������ָ��B��A��ָ���һ������RAM��ַ���ڶ�������RAM��ַ���ţ�
 * �����������A��B����ָ��ֱ�ָ����������RAM��ַ����ʼRT���޸�����ʱ��pingpongģʽ�޸ġ�
 * 
 * ��ʽ���������ֻ��Ӧһ������RAM��ַ����A��B����ָ�붼ָ����һ��RAM��ַ��
 * 
 * RAM�Ų���(��λ����)
 * 128���ӵ�ַ_������_R
 * 128���ӵ�ַ_������_T
 * 128����ʽ��_������_R
 * 128����ʽ��_������_T
 * 34*6����ʽ��_����RAM
 * 68*n���ӵ�ַ_����RAM
 * ...
 * 32*2���ж�log
 */

WORD RT_dataAddr = 512;
int rtisrcnt = 0;
void g_1553B_RTISR(S16BIT DevNum)
{
	if (map_devInfo.find(DevNum) == map_devInfo.end()){
		return;
	}
	CDeviceInfo* dev_info = map_devInfo[DevNum];

	WORD baseRamAddr = (DevNum%2 == 0) ? AT_BC_BASE_RAM_ADDR_BUS0 : AT_BC_BASE_RAM_ADDR_BUS1;
	WORD wRegVal = AtReadReg( DevNum, 5 ) - baseRamAddr;
	
	if (wRegVal < AT_RAM_SIZE - 32)
	{
		return;
	}
	
	while(wRegVal != dev_info->IPCORE_wIntPtr)
	{
		WORD wIntData[2];
		AtReadBlock(DevNum, dev_info->IPCORE_wIntPtr, wIntData, 2 );
		if( wIntData[0] & 0x0700 )
		{
			WORD IntBlkAddr = wIntData[1] - baseRamAddr;
			WORD ctrlWord = AtReadRam( DevNum, IntBlkAddr );
			
			WORD dataAddr = AtReadRam( DevNum, IntBlkAddr + 1 ) - baseRamAddr;
			WORD msgInfo = AtReadRam( DevNum, dataAddr );
			WORD timeTag = AtReadRam( DevNum, dataAddr + 1 );
		
			MSGSTRUCT msg;
			memset(&msg, 0, sizeof(MSGSTRUCT));
			msg.wTimeTag = timeTag;
			//������������
			msg.wWordCount = msgInfo >> 11;
			if(msg.wWordCount == 0){
				msg.wWordCount = 32;
			}
			//����������
			msg.wCmdWrd1Flg = 1;
			U16BIT wRT = AtReadReg( DevNum, 1 ) >> 11;
			U16BIT wTR;
			U16BIT wSA;
			//�����ӵ�ַ
			if(IntBlkAddr < 256){
				wSA = (IntBlkAddr % 128) / 4;
			}
			else{
				wSA = 0; //mode code
			}
			//������շ���λ
			if((IntBlkAddr < 128) || (IntBlkAddr >= 256 && IntBlkAddr < 384)){
				wTR = 0;
			}
			else{
				wTR = 1;
			}
			aceCmdWordCreate(&msg.wCmdWrd1,wRT,wTR,wSA,msg.wWordCount);
			//��ȡ����
			AtReadBlock( DevNum, dataAddr + 2, msg.aDataWrds, 32 );
			
			dev_info->RT_queue_MSG.push(msg);
		}
		dev_info->IPCORE_wIntPtr += 2;
		if( dev_info->IPCORE_wIntPtr >= AT_RAM_SIZE - 1 )
		{
			dev_info->IPCORE_wIntPtr = AT_RAM_SIZE - 32;
		}
	}

}

S16BIT aceRTSetAddress
(
    S16BIT DevNum,
    U16BIT wRTAddress
)
{
	if (map_devInfo.find(DevNum) == map_devInfo.end()){
		return -1;
	}
	
	WORD baseRamAddr = (DevNum%2 == 0) ? AT_BC_BASE_RAM_ADDR_BUS0 : AT_BC_BASE_RAM_ADDR_BUS1;

	WORD wRegVal = AtReadReg(DevNum, 1);
	wRegVal = (wRegVal & 0x3ff) + (wRTAddress << 11);
	//��У��
	std::bitset<5> bs( (long) wRTAddress );
	if (bs.count() % 2 )
	{
		wRegVal &= (~0x0400);
	} 
	else
	{
		wRegVal |= 0x0400;
	}

	AtWriteReg( DevNum, 1, wRegVal );

	//��ʼ���ӵ�ַ������
	//�ӵ�ַ���յ�
	for(WORD dspBlkAddr=0; dspBlkAddr<128; dspBlkAddr+=4){
		AtWriteRam( DevNum, dspBlkAddr, 0x01E3 );	
		AtWriteRam( DevNum, dspBlkAddr+1, AT_RAM_SIZE - 32 - 34 );	
		AtWriteRam( DevNum, dspBlkAddr+2, AT_RAM_SIZE - 32 - 34 );	
		AtWriteRam( DevNum, dspBlkAddr+3, AT_RAM_SIZE - 32 - 34 );	
	}
	//�ӵ�ַ���͵�
	for(WORD dspBlkAddr=128; dspBlkAddr<256; dspBlkAddr+=4){
		AtWriteRam( DevNum, dspBlkAddr, 0x01E3 );
		AtWriteRam( DevNum, dspBlkAddr+1, AT_RAM_SIZE - 32 - 68 );	
		AtWriteRam( DevNum, dspBlkAddr+2, AT_RAM_SIZE - 32 - 68 );	
		AtWriteRam( DevNum, dspBlkAddr+3, AT_RAM_SIZE - 32 - 68 );		
	}
	//��ʽ���
	for(WORD dspBlkAddr=256; dspBlkAddr<512; dspBlkAddr+=4){
		AtWriteRam( DevNum, dspBlkAddr, 0x01E3 );
		AtWriteRam( DevNum, dspBlkAddr+1, AT_RAM_SIZE - 32 - 68 );	
		AtWriteRam( DevNum, dspBlkAddr+2, AT_RAM_SIZE - 32 - 68 );	
		AtWriteRam( DevNum, dspBlkAddr+3, AT_RAM_SIZE - 32 - 68 );		
	}

	//���ô����ݵķ�ʽ���������ָ��
	RT_dataAddr = 512;
	for(WORD modeCode=16; modeCode<22; modeCode++){
		WORD dspBlkAddr;
		//T
		if(modeCode == 0x10 || modeCode == 0x12 || modeCode == 0x13){
			dspBlkAddr = 384 + 4 * modeCode;
		}
		//R
		else{
			dspBlkAddr = 256 + 4 * modeCode;
		}
		AtWriteRam( DevNum, dspBlkAddr + 1, baseRamAddr + RT_dataAddr ); //Data pointer A
		AtWriteRam( DevNum, dspBlkAddr + 2, baseRamAddr + RT_dataAddr ); //Data pointer B
		AtWriteRam( DevNum, dspBlkAddr + 3, baseRamAddr + RT_dataAddr ); //broadcast
		RT_dataAddr += 34;  //luxq 102
	}
	return 0;
}

S16BIT aceRTDataBlkCreate
(
    S16BIT DevNum,
    S16BIT nDataBlkID,
    U16BIT wDataBlkType,
    U16BIT *pBuffer,
    U16BIT wBufferSize
)
{
	if (map_devInfo.find(DevNum) == map_devInfo.end()){
		return -1;
	}
	CDeviceInfo* dev_info = map_devInfo[DevNum];
       //512 + 68 = 580            2048 - 32 -34 -34 = 1948 
	if((RT_dataAddr + 102) > (AT_RAM_SIZE - 32 - 34 -34)){
		printf("warning: no more RAM.\n");
		return -1;
	}
	dev_info->RT_map_dataID_dataAddr.insert(make_pair(nDataBlkID, RT_dataAddr));
	AtWriteBlock(DevNum, RT_dataAddr + (34*0 +2),   pBuffer, wBufferSize);  //data A
	AtWriteBlock(DevNum, RT_dataAddr + (34*1 + 2), pBuffer,  wBufferSize); //data B   36+34=70
//	AtWriteBlock(DevNum, RT_dataAddr + 80, pBuffer, wBufferSize); //broadcast
	AtWriteBlock(DevNum, RT_dataAddr + (34*2 + 2), pBuffer, wBufferSize); //broadcast
	//RT_dataAddr += 68;  //luxq
	RT_dataAddr += 102; //modify by yinhongen
	
	return 0;
}

S16BIT aceRTDataBlkWrite
(
    S16BIT DevNum,
    S16BIT nDataBlkID,
    U16BIT *pBuffer,
    U16BIT wBufferSize,
    U16BIT wOffset
)
{
	if (map_devInfo.find(DevNum) == map_devInfo.end()){
		return -1;
	}
	CDeviceInfo* dev_info = map_devInfo[DevNum];

	if(dev_info->RT_map_dataID_dataAddr.find(nDataBlkID) 
		== dev_info->RT_map_dataID_dataAddr.end())
	{
		return -1;
	}
	WORD dataAddr = dev_info->RT_map_dataID_dataAddr[nDataBlkID];
	//���RTδ��ʼ��ֱ���޸�buffA��buffB������
	if(dev_info->is_IPCORE_start == FALSE){
		AtWriteBlock(DevNum, dataAddr + 2, pBuffer, wBufferSize);  //data A
		AtWriteBlock(DevNum, dataAddr + 36, pBuffer, wBufferSize); //data B
		AtWriteBlock(DevNum, dataAddr + 70, pBuffer, wBufferSize); //broadcast   open by yinhonggen
	}
	//���RT��ʼ�ˣ�����pingpong�����޸�����buff������
	else{
		//Disable the ping pong mode by setting PPEN (register 00, bit X) LOW
		WORD wRegVal = AtReadReg(DevNum, 0);
		wRegVal &= ~(1 << 2);
		AtWriteReg( DevNum, 0, wRegVal );
		//Verify that ping pong mode has been disabled by querying bit 9, MSGTO (Message Timeout)
		wRegVal = AtReadReg(DevNum, 0);
		while((wRegVal & (1 << 9)) > 0){
//			sleep(1);
			wRegVal = AtReadReg(DevNum, 0);
		}
		//Determine the active buffer by querying bit 2, A/B (A or B buffer), 
		//of the current descriptor Control Word
		WORD dspBlkAddr = dev_info->RT_map_dataID_dspBlkAddr[nDataBlkID];
		WORD ctrlWord = AtReadRam(DevNum, dspBlkAddr);
		
		//A logic 1 designates buffer A as the primary; logic 0 designates buffer B.
		WORD dataPoint = 0;//A
		if((ctrlWord & (1 << 2)) >0){
			dataPoint = 1; //�����õ���A���������Ƿ���B
		}
		//Service the secondary buffer  luxq
		AtWriteBlock(DevNum, dataAddr + 34*dataPoint + 2, pBuffer, wBufferSize); 

		//д����һ��buffer
		dataPoint ^= 1;
		AtWriteBlock(DevNum, dataAddr + 34*dataPoint + 2, pBuffer, wBufferSize);
		
		//��A/Bλȡ��
		//ctrlWord ^= 1 << 2;
		if(1 == dataPoint)
		{
		      ctrlWord |= (1 << 2);      
		}
		else
		{
            ctrlWord &= ~(1 << 2);
		}
		AtWriteRam(DevNum, dspBlkAddr, ctrlWord);
			
		//Re-enable ping pong mode (setting PPEN HIGH)
		wRegVal = AtReadReg(DevNum, 0);
		wRegVal |= 1 << 2;
		AtWriteReg( DevNum, 0, wRegVal );
		//Verify that ping pong mode has been enabled by querying MSGTO
		wRegVal = AtReadReg(DevNum, 0);
		while((wRegVal & (1 << 9)) == 0){
          sleep(1);
			wRegVal = AtReadReg(DevNum, 0);
		    }	
		}

	return 0;
}

/**
 * �����ݿ��ַд����������
 * 
 */
S16BIT aceRTDataBlkMapToSA
(
    S16BIT DevNum,
    S16BIT nDataBlkID,
    U16BIT wSA,
    U16BIT wMsgType,
    U16BIT wIrqOptions,
    U16BIT wLegalizeSA
)
{
	if (map_devInfo.find(DevNum) == map_devInfo.end()){
		return -1;
	}
	CDeviceInfo* dev_info = map_devInfo[DevNum];

	WORD baseRamAddr = (DevNum%2 == 0) ? AT_BC_BASE_RAM_ADDR_BUS0 : AT_BC_BASE_RAM_ADDR_BUS1;
	WORD dspBlkAddr = 4 * wSA;
	if(wMsgType == ACE_RT_MSGTYPE_TX){
		dspBlkAddr += 128 ;
	}

	if(dev_info->RT_map_dataID_dataAddr.find(nDataBlkID) 
		== dev_info->RT_map_dataID_dataAddr.end())
	{
		return -1;
	}
	WORD dataAddr = dev_info->RT_map_dataID_dataAddr[nDataBlkID];
	//Data pointer A
	AtWriteRam( DevNum, dspBlkAddr + 1, baseRamAddr + dataAddr ); 
	//Data pointer B
	if(wMsgType == ACE_RT_MSGTYPE_TX){
		AtWriteRam( DevNum, dspBlkAddr + 2, baseRamAddr + dataAddr + 34 ); 
	}
	else{
		AtWriteRam( DevNum, dspBlkAddr + 2, baseRamAddr + dataAddr );
	}
	//broadcast
	AtWriteRam( DevNum, dspBlkAddr + 3, baseRamAddr + dataAddr ); 

	
	dev_info->RT_map_dataID_dspBlkAddr.insert(make_pair(nDataBlkID, dspBlkAddr));
	
/*	//����RT�Ϸ����Ĵ���
	if(wSA < 16){
		WORD regVal = AtReadReg(DevNum, 16);
		regVal &= ~(1 << wSA);
		AtWriteReg( DevNum, 16, regVal );
		AtWriteReg( DevNum, 18, regVal );
	}
	else{
		WORD regVal = AtReadReg(DevNum, 17);
		regVal &= ~( 1 << (wSA % 16) );
		AtWriteReg( DevNum, 17, regVal );
		AtWriteReg( DevNum, 19, regVal );		
	}*/
	return 0;
}

/**
 * ���յ���ʽ������ʱ�������ж�
 */
S16BIT aceRTModeCodeIrqEnable
(
    S16BIT DevNum,
    U16BIT wModeCodeType,
    U16BIT wModeCodeIrq
)
{

	return 0;
}

S16BIT aceRTMTStart(S16BIT DevNum)
{
	if (map_devInfo.find(DevNum) == map_devInfo.end()){
		return -1;
	}
	CDeviceInfo* dev_info = map_devInfo[DevNum];

	//�����������controlword
	//�ӵ�ַ��

	//showRam(0, 10);
	map<WORD,WORD>::iterator itr;
	for(itr=dev_info->RT_map_dataID_dspBlkAddr.begin(); 
		itr!=dev_info->RT_map_dataID_dspBlkAddr.end();
		itr++){
		WORD dspBlkAddr = itr->second;
		AtWriteRam( DevNum, dspBlkAddr,  0x01E3);	//luxq 0x01E3  0x0062
	}

	//��ʽ�����
	for(WORD dspBlkAddr=256; dspBlkAddr<512; dspBlkAddr+=4){
		AtWriteRam( DevNum, dspBlkAddr, 0x01E3 );	
	}

	//��ʼRT
	WORD wRegVal;
	wRegVal = AtReadReg(DevNum, 0);
	AtWriteReg( DevNum, 0, wRegVal | 0x8000 );
	dev_info->is_IPCORE_start = TRUE;
	return 0;
}

S16BIT aceRTMTStop(S16BIT DevNum)
{
	if (map_devInfo.find(DevNum) == map_devInfo.end()){
		return -1;
	}
	CDeviceInfo* dev_info = map_devInfo[DevNum];

	WORD wRegVal;
	wRegVal = AtReadReg( DevNum, 0 );
	AtWriteReg( DevNum, 0, wRegVal & 0x7FFF );	
	dev_info->is_IPCORE_start = FALSE;
	return 0;
}

S16BIT aceRTGetStkMsgDecoded
(
    S16BIT DevNum,
    MSGSTRUCT *pMsg,
    U16BIT wMsgLoc
)
{
	if (map_devInfo.find(DevNum) == map_devInfo.end()){
		return -1;
	}
	CDeviceInfo* dev_info = map_devInfo[DevNum];

	if(dev_info->RT_queue_MSG.empty()){
		return 0;
	}
	
	MSGSTRUCT msg = dev_info->RT_queue_MSG.front();
	memcpy(pMsg, &msg, sizeof(MSGSTRUCT) );
	
	if(wMsgLoc == ACE_RT_MSGLOC_LATEST_PURGE){ //����next_purge��luxq
		dev_info->RT_queue_MSG.pop();
	}
	return 1;
}

S16BIT aceRTStatusBitsSet
(
    S16BIT DevNum,
    U16BIT wStatusBits
)
{
	if (map_devInfo.find(DevNum) == map_devInfo.end()){
		return -1;
	}

	WORD wRegVal;
	wRegVal = AtReadReg( DevNum, 9 );
	
	// æλ Busy
	if (ACE_RT_STSBIT_BUSY == wStatusBits){
		wRegVal |= 1 << 3;
	}
	// ��������λ Service Request
	else if (ACE_RT_STSBIT_SREQ == wStatusBits){
		wRegVal |= 1 << 8;
	}
	// ��ϵͳ����λ Subsystem Flag
	else if (ACE_RT_STSBIT_SSFLAG == wStatusBits){
		wRegVal |= 1 << 2;
	}
	// �ն�����λ RT Flag
	else if (ACE_RT_STSBIT_RTFLAG == wStatusBits){
		wRegVal |= 1 ;
	}

	AtWriteReg( DevNum, 9, wRegVal );
	return 0;
}

S16BIT aceRTStatusBitsClear
(
    S16BIT DevNum,
    U16BIT wStatusBits
)
{
	if (map_devInfo.find(DevNum) == map_devInfo.end()){
		return -1;
	}

	WORD wRegVal;
	wRegVal = AtReadReg( DevNum, 9 );
	
	// æλ Busy
	if (ACE_RT_STSBIT_BUSY == wStatusBits){
		wRegVal &= ~(1 << 3);
	}
	// ��������λ Service Request
	else if (ACE_RT_STSBIT_SREQ == wStatusBits){
		wRegVal &= ~(1 << 8);
	}
	// ��ϵͳ����λ Subsystem Flag
	else if (ACE_RT_STSBIT_SSFLAG == wStatusBits){
		wRegVal &= ~(1 << 2);
	}
	// �ն�����λ RT Flag
	else if (ACE_RT_STSBIT_RTFLAG == wStatusBits){
		wRegVal &= ~(1);
	}

	AtWriteReg( DevNum, 9, wRegVal );
	return 0;
}

/*
 * �����������������ֻ�з�ʽ��16����Ϊ18�Ǵ���һ�������֣� 19�Ǵ�BIT��
 * 
 */
S16BIT aceRTModeCodeWriteData
(
    S16BIT DevNum,
    U16BIT wModeCode,
    U16BIT wMCData
)
{
	if (map_devInfo.find(DevNum) == map_devInfo.end()){
		return -1;
	}

	WORD baseRamAddr = (DevNum%2 == 0) ? AT_BC_BASE_RAM_ADDR_BUS0 : AT_BC_BASE_RAM_ADDR_BUS1;
	WORD dspBlkAddr;
	//T
	if(wModeCode == 0x10 || wModeCode == 0x12 || wModeCode == 0x13){
		dspBlkAddr = 384 + 4 * wModeCode;
	}
	else{
		return 0;
	}

	WORD dataAddr = AtReadRam(DevNum, dspBlkAddr + 1);
//	if(is_IPCORE_start == FALSE){
		AtWriteBlock(DevNum, dataAddr + 2 - baseRamAddr, &wMCData, 1);  //data A
//		AtWriteBlock(DevNum, dataAddr + 36 - baseRamAddr, &wMCData, 1); //data B
//		AtWriteBlock(DevNum, dataAddr + 70 - baseRamAddr, &wMCData, 1); //broadcast
/*	}

	else{
		//Disable the ping pong mode by setting PPEN (register 00, bit X) LOW
		WORD wRegVal = AtReadReg(DevNum, 0);
		wRegVal &= ~(1 << 2);
		AtWriteReg( DevNum, 0, wRegVal );
		//Verify that ping pong mode has been disabled by querying bit 9, MSGTO (Message Timeout)
		wRegVal = AtReadReg(DevNum, 0);
		while((wRegVal & (1 << 9)) > 0){
			sleep(1);
			wRegVal = AtReadReg(DevNum, 0);
		}
		//Determine the active buffer by querying bit 2, A/B (A or B buffer), 
		//of the current descriptor Control Word
		WORD ctrlWord = AtReadRam(DevNum, dspBlkAddr);
		//A logic 1 designates buffer A as the primary; logic 0 designates buffer B.
		WORD dataPoint = 0;//A
		if(ctrlWord & (1 << 2) >0){
			dataPoint = 1; //�����õ���A���������Ƿ���B
		}
		//Service the secondary buffer
		AtWriteBlock(DevNum, dataAddr + 34*dataPoint + 2 - baseRamAddr, &wMCData, 1); 
		//Re-enable ping pong mode (setting PPEN HIGH)
		wRegVal = AtReadReg(DevNum, 0);
		wRegVal |= 1 << 2;
		AtWriteReg( DevNum, 0, wRegVal );
		//Verify that ping pong mode has been enabled by querying MSGTO
		wRegVal = AtReadReg(DevNum, 0);
		while((wRegVal & (1 << 9)) == 0){
			sleep(1);
			wRegVal = AtReadReg(DevNum, 0);
		}
	}
*/
	return 0;
}

/**
 * The default resolution of this timer is 64 ms/bit,��ʵ�ʹ۲���64us
 * DDC����2us��������ɸ�DDC��һ��
 * 
 * ������һ�£����ǲ�֧�֣���ʱ����ȥȡ����ʱ��
 */
S16BIT aceGetTimeTagValueEx(S16BIT DevNum, 
		U64BIT* ullTTValue)
{
	return ACE_ERR_NOT_SUPPORTED;
/*	
	WORD wRegVal;
	wRegVal = AtReadReg( DevNum, 7 );
	*ullTTValue = wRegVal * 32; 
	return 0;
*/
}
