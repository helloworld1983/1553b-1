#ifndef __COMMON_QIU__
#define __COMMON_QIU__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif
typedef unsigned long       DWORD;
typedef int                 BOOL;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef float               FLOAT;


#ifndef BASETYPES
#define BASETYPES
typedef unsigned long ULONG;
//typedef ULONG *PULONG;
typedef unsigned short USHORT;
//typedef USHORT *PUSHORT;
typedef unsigned char UCHAR;
//typedef UCHAR *PUCHAR;
//typedef char *PSZ;
#endif  /* !BASETYPES */

namespace Agent
{
	class ISysServ
	{
	private:
		int x;
	};

	class IAdapter
	{
	private:
		int x;
	};

	class IAdapter_V2
	{
	private:
		int x;
	};

	class DevDataHeader
	{
	public:
		int extra_size;
	};
}

/*
typedef FLOAT               *PFLOAT;
typedef BOOL near           *PBOOL;
typedef BOOL far            *LPBOOL;
typedef BYTE near           *PBYTE;
typedef BYTE far            *LPBYTE;
typedef int near            *PINT;
typedef int far             *LPINT;
typedef WORD near           *PWORD;
typedef WORD far            *LPWORD;
typedef long far            *LPLONG;
typedef DWORD near          *PDWORD;
typedef DWORD far           *LPDWORD;
typedef void far            *LPVOID;
typedef CONST void far      *LPCVOID;

typedef unsigned int        *PUINT;
*/

typedef int                 INT;
typedef unsigned int        UINT;

typedef short SHORT;
typedef long LONG;
typedef char CHAR;

#endif
