#ifndef __SAFE_QUE_H
#define __SAFE_QUE_H
#include <wtypes.h>
const size_t BUFFER_SIZE_LIMIT = 1024 * 1024 * 1024;
class CSafeQueException {
	// BufSize���ܳ���BUFFER_SIZE_LIMIT��
};
class BrSafeQue
{

public:
	BrSafeQue();
	~BrSafeQue();

private:
	BYTE* m_pbyBuffer;
	DWORD m_dwBufSize;
	DWORD m_dwWritePos;
	DWORD m_dwReadPos;

	CRITICAL_SECTION m_objSync;

	inline BOOL IsPosLE( DWORD a, DWORD b )
	{
		return (b - a) <= m_dwBufSize * 2;
	}

	inline BOOL IsPosGE( DWORD a, DWORD b )
	{
		return (a - b) <= m_dwBufSize * 2;
	}

	inline void EnterCritical()
	{
		EnterCriticalSection( &m_objSync );
	}

	inline void ExitCritical()
	{
		LeaveCriticalSection( &m_objSync );
	}

	inline void RoundWriteMemory( const BYTE* pbySrc, DWORD dwItemPos, DWORD dwSize )
	{
		if ( dwItemPos + dwSize <= m_dwBufSize )
		{
			memcpy( m_pbyBuffer + dwItemPos, pbySrc, dwSize );
		}
		else
		{
			memcpy( m_pbyBuffer + dwItemPos, pbySrc, m_dwBufSize - dwItemPos );
			memcpy( m_pbyBuffer, pbySrc + m_dwBufSize - dwItemPos,
				dwSize - (m_dwBufSize - dwItemPos) );
		}
	}

	inline void RoundReadMemory( BYTE* pbyDest, DWORD dwItemPos, DWORD dwSize )
	{
		if ( dwItemPos + dwSize <= m_dwBufSize )
		{
			memcpy( pbyDest, m_pbyBuffer + dwItemPos, dwSize );
		}
		else
		{
			memcpy( pbyDest, m_pbyBuffer + dwItemPos, m_dwBufSize - dwItemPos  );
			memcpy( pbyDest + m_dwBufSize - dwItemPos,
				m_pbyBuffer, dwSize - (m_dwBufSize - dwItemPos)  );
		}
	}

public:
	void reset();		///���û���
	BOOL init( DWORD dwbufsize ); ///��ʼ�����壬dwbufsize �����С
	void destroy();  /// ���ٻ���

	BOOL read_block( BYTE* pbyData, DWORD dwSize ); ///�������ݣ�������ݲ���dwSize����Ϊʧ�ܡ�
	BOOL write_block( const BYTE* pbyData, DWORD dwSize );///д�����ݣ�������ݲ���dwSize����Ϊʧ�ܡ�

	DWORD read( BYTE* pbyData, DWORD dwSize ); ///�����ݣ����ػ�ȡ�����ݴ�С
	DWORD write( const BYTE* pbyData, DWORD dwSize ); ///�����ݣ�����д������ݴ�С

	DWORD  get_read_pos();  ///��ǰ��λ��
	DWORD  get_write_pos(); ///��ǰдλ��

	DWORD  get_size();  ///���ػ����С
	float get_usage();  ///���ػ���ʹ����
};

#endif // __SAFE_QUE_H