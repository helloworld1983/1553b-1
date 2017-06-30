/*	
 * actel1553io.cpp
 * ʵ������������ܵ�һЩ����ʵ��
 * \date 2013-6-5
 * \author xiaoqing.lu
 */

#include "zynq_1553b_api.h"
#include "head.h"
#ifdef __vxworks
#include <unistd.h>
#endif
#include <sstream>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#ifdef WIN32
#include <io.h>
#endif

/*
actel 1553b IP core �ļĴ������ڴ���ʷ�ʽ��
(1)�ȸ���ʼ��ַΪ0x43c00000��ƫ�Ƶ�ַΪ0�ĵ�ַ�ռ�д2(����reg)����4(����ram)��
(2)Ȼ��Ե�ַ�ռ���ʼ��ַΪ0x43d00000���ж�д����
*/
#define REG_ACCESS    0
#define RAM_ACCESS    1
#define NONE_ACCESS -1

#define REG_TYPE    2
#define RAM_TYPE    4

//STATUS ReadRam (UINT16 cardNum, UINT32 address, UINT32 length, UINT16 *data){return 1;};
//STATUS WriteRam (UINT16 cardNum, UINT32 address, UINT16*data, UINT32 length){return 1;};

/*
 * ��ȡ��address��ʼ��RAM��ַ�ϵ����ݣ�д��dumpRAM.bin�ļ���
 * ������8k�ֽ�
 */
 
#if 0 
void dumpRAM(UINT16 cardNum, UINT32 address){
#ifdef WIN32
	int fd = _open("dumpRAM.bin", O_WRONLY | O_BINARY, 0);
#else
	int fd = open("dumpRAM.bin", O_WRONLY | O_CREAT, 0);
#endif

	if ( fd == -1 ) {
		printf("open file dumpRAM.bin error.\n" );
		return;
	}

#ifdef WIN32
	_lseek(fd, 0, 0);
#else
	lseek(fd, 0, 0);
#endif	

	WORD g_ram[AT_RAM_SIZE];
	ReadRam(cardNum, address, AT_RAM_SIZE, g_ram );

#ifdef WIN32
	_write(fd, (char*)g_ram, AT_RAM_SIZE*2);
#else
	write(fd, (char*)g_ram, AT_RAM_SIZE*2);
#endif	
	
#ifdef WIN32
	_close(fd);
#else
	close(fd);
#endif	

	memset(g_ram, 0, AT_RAM_SIZE*2);
}
#endif

pthread_mutex_t mutex;

int fd_addrdevice;  
void open_device(void)
{
	fd_addrdevice = open(MODEMDEVICE,O_RDWR);   
	if (fd_addrdevice < 0) 
	{  
		printf("open %s faile\n",MODEMDEVICE);  
		exit(1);  
	}  

		pthread_mutexattr_t attr;
	    pthread_mutexattr_init(&attr); 
    // ���� recursive ����
       pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE_NP); 
       pthread_mutex_init(&mutex,&attr);
}

void close_device(void)
{
 	 close(fd_addrdevice);
	 pthread_mutex_destroy(&mutex);
}



#ifdef DUMP_RAM //����ʱ�ã�g_ram�������RAM�ռ�
WORD g_ram[2][AT_RAM_SIZE * 4];

void dumpRAM(){
	int fd = open("dumpRAM.bin", O_WRONLY | O_CREAT | 0x8000, 0);
	if ( fd == -1 ) {
		printf("open file dumpRAM.bin error.\n" );
		return;
	}
	lseek(fd, 0, 0);
	write(fd, (char*)g_ram, AT_RAM_SIZE * 2);
	close(fd);
	
	memset(g_ram, 0, AT_RAM_SIZE * 4 * 2);
}
#endif


int flag = NONE_ACCESS;
void WriteCtrWord(int accessType)
{
    unsigned int value;
    unsigned int w_val;
   
    if((accessType == REG_ACCESS) && (flag != REG_ACCESS))
    {
        flag = REG_ACCESS;
        w_val = REG_TYPE;
        ioctl(fd_addrdevice, 0, 0);
        write(fd_addrdevice, &w_val, sizeof(unsigned int));
    }
    else if((accessType == RAM_ACCESS) && (flag != RAM_ACCESS))
    {
        flag = RAM_ACCESS;
        w_val = RAM_TYPE;
        ioctl(fd_addrdevice, 0, 0);
        write(fd_addrdevice, &w_val, sizeof(unsigned int));
    }

}

STATUS ReadRam (UINT16 cardNum, UINT16 index, UINT16 length, UINT16 *data)
{ 
    UINT16 value = 0;
    UINT16 address = 0;

    if (cardNum > 1)
    {
        printf("[ReadRam]error cardNum %d.\n", cardNum);
        return -1;
    }

    for(UINT16 i=0; i<length; i++)
    {
        //���Ĵ�����Ż��ڴ��ֱ��(index)������߼�ʶ��ĵ�ַ��������4�ֽڶ���
        //��ƫ����
        address = (index<<2) + (i<<2);
        ioctl(fd_addrdevice, 1, address);
        read(fd_addrdevice, &value, sizeof(UINT16));
        data[i] = value ;
    }

    return 0;
}

STATUS WriteRam (UINT16 cardNum, UINT16 index, UINT16 length, UINT16 *data)
{
    UINT16 address = 0;
    
    if (cardNum > 1)
    {
        printf("[WriteRam]error cardNum %d.\n", cardNum);
        return -1;
    }

    for(UINT16 i=0; i<length; i++)
    {
        //���Ĵ�����Ż��ڴ��ֱ��(index)������߼�ʶ��ĵ�ַ��������4�ֽڶ���
        //��ƫ����
        address = (index<<2) + (i<<2);
        ioctl(fd_addrdevice, 1, address);
        write(fd_addrdevice, data+i, sizeof(UINT16));

        ioctl(fd_addrdevice, 1, address);
        write(fd_addrdevice, data+i, sizeof(UINT16));

        //�ڴ����д���飬�ٶ�һ��
        if(flag == RAM_ACCESS)
        {
            UINT16 value = 0;
            ioctl(fd_addrdevice, 1, address);
            read(fd_addrdevice, &value, sizeof(UINT16));
        }
    }

    return 0;
}


void AtWriteReg( WORD Bus, WORD Reg, WORD Value )
{
    pthread_mutex_lock(&mutex);
    WriteCtrWord(REG_ACCESS);
    WriteRam(Bus/2, Reg, 1, &Value);
    pthread_mutex_unlock(&mutex);
    //printf(" Write the reg number is %d , address is [0x%x] , value is 0x%x\n",Reg,AT_BC_BASE_REG_ADDR_BUS0+address,Value);
}

WORD AtReadReg( WORD Bus, WORD Reg )
{
    pthread_mutex_lock(&mutex);
    WORD wVal = 0;
    WriteCtrWord(REG_ACCESS);
    ReadRam(Bus/2, Reg, 1, &wVal );
    pthread_mutex_unlock(&mutex);
    //printf(" Read the reg number is %d , address is [0x%x] , value is 0x%x\n",Reg,AT_BC_BASE_REG_ADDR_BUS0+address,wVal);	
    return  wVal;
}

/* дRAM���� */
void AtWriteRam( WORD Bus, WORD Offset, WORD Value )
{
    pthread_mutex_lock(&mutex);
    WriteCtrWord(RAM_ACCESS);
    WriteRam(Bus/2, Offset, 1, &Value );
    pthread_mutex_unlock(&mutex);
    //printf(" write the ram number is %d , address is [0x%x] , value is 0x%x\n",Offset,AT_BC_BASE_REG_ADDR_BUS0+address,Value);	
}

/* ��RAM���� */
WORD AtReadRam( WORD Bus, WORD Offset )
{
    pthread_mutex_lock(&mutex);
    WORD wVal = 0;
    WriteCtrWord(RAM_ACCESS);
    ReadRam(Bus/2, Offset, 1, &wVal );
    pthread_mutex_unlock(&mutex);
    //printf(" read the ram number is %d , address is [0x%x] , value is 0x%x\n",Offset,AT_BC_BASE_REG_ADDR_BUS0+address,wVal);
    return  wVal;
}

/* ��RAM���ֶ� */
int AtWriteBlock( WORD Bus, WORD Offset, WORD* DataIn, WORD Count )
{
    pthread_mutex_lock(&mutex);
    WriteCtrWord(RAM_ACCESS);	
    WriteRam(Bus/2, Offset, Count, DataIn);
    pthread_mutex_unlock(&mutex);
    return 1;
}

int AtReadBlock( WORD Bus, WORD Offset, WORD* DataOut, WORD Count )
{
    pthread_mutex_lock(&mutex);
    WriteCtrWord(RAM_ACCESS);	
    ReadRam(Bus/2, Offset, Count, DataOut);
    pthread_mutex_unlock(&mutex);
    return  1;
}


void showReg()
{
    int i;
    WORD value = 0;
        
    for(i=0; i<33; i++)
    {
        value = AtReadReg(0, i);
        printf("Reg[%02d] = 0x%04x    ", i, value);
        if(i%4 == 3)
            printf("\n");
    }
    
    printf("\n");
}

void showRam(WORD index, WORD count)
{
    int i, j;
    WORD value = 0;
    
    if((index >= AT_RAM_SIZE) || ((index + count)> AT_RAM_SIZE))
    {
        printf("Input value overflow! \n");
        return;
    }

    for(i=0; i<count; i++)
    {
        WORD offset = index+i;
        WORD res = offset%8;
        
        value = AtReadRam(0, offset);

        if((res == 0) ||(i == 0))
        {
            printf("0x%04x(%04d): ", offset-res, offset-res);
        }

        if(i == 0)
        {
            for(j=8-res; j<8; j++)
                printf("      ");
        }
        
        printf("%04x  ", value);
        if(res == 7)
            printf("\n");

    }

    printf("\n");

}


