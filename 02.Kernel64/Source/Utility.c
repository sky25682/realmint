#include "Utility.h"
#include "AssemblyUtility.h"
#include <stdarg.h>
#include "Utility.h"
#include "AssemblyUtility.h"
#include <stdarg.h>

// PIT 컨트롤러가 발생한 횟수를 저장할 카운터
volatile QWORD g_qwTickCount = 0;


void kMemSet(void* pvDestination, BYTE bData, int iSize)
{
	int i;
	for(i=0; i < iSize ; i++)
	{
		((char* ) pvDestination)[i] = bData;
	}
}

int kMemCpy(void* pvDestination, const void* pvSource, int iSize)
{
	int i;
	for(i = 0; i< iSize ; i++)
	{
		( ( char*) pvDestination)[i] = ((char*) pvSource)[i];
	}

	return iSize;
}

// 메모리 비교
int kMemCmp( const void* pvDestination, const void* pvSource, int iSize )
{
 	int i;
 	char cTemp;

 	for( i = 0 ; i < iSize ; i++ )
 	{
 		cTemp = ( ( char* ) pvDestination )[ i ] - ( ( char* ) pvSource )[ i ];
 		if( cTemp != 0 )
 		{
 			return ( int ) cTemp;
 		}
 	}
 	return 0;
}

// RFLAGS 레지스터의 인터럽트 플래그를 변경하고 이전 인터럽트 플래그의 상태를 반환
BOOL kSetInterruptFlag( BOOL bEnableInterrupt)
{
	QWORD qwRFLAGS;

	 // 이전의 RFLAGS 레지스터 값을 읽은 뒤에 인터럽트 가능/불가 처리

	qwRFLAGS = kReadRFLAGS();
	if(bEnableInterrupt == TRUE )
	{
		kEnableInterrupt();
	}
	else
	{
		kDisableInterrupt();
	}

	 // 이전 RFLAGS 레지스터의 IF 비트(비트 9)를 확인하여 이전의 인터럽트 상태를 반환

	if(qwRFLAGS & 0x0200)
	{
		return TRUE;
	}
	return FALSE;
}

// 문자열의 길이를 반환

//문자열의 길이를 반환
int kStrLen(const char* pcBuffer) {
	int i;

	for (i = 0; ; i++)
	{
		if (pcBuffer[i] == '\0')
		{
			break;
		}
	
	}
	return i;
}

// 램의 총 크기(MB 단위)
static gs_qwTotalRAMMBSize = 0;

// 64MB 이상의 위치부터 램 크기를 체크
// 최소 부팅 과정에서 한 번만 호출해야 함
void kCheckTotalRAMSize(void)
{
	DWORD* pdwCurrentAddress;
	DWORD dwPreviousValue;

	//64MB(0x400000)부터 4MB 단위로 검사
	pdwCurrentAddress = (DWORD*)0x4000000;
	while(1)
	{
		//이전 메모리에 있던 값 저장
		dwPreviousValue = *pdwCurrentAddress;
		// 0x123456을 써서 읽었을 때 문제가 ㅇ벗는 곳까지를 유효한 메모리 영역으로 인정
		*pdwCurrentAddress = 0x12345678;
		if (*pdwCurrentAddress != 0x12345678)
		{
			break;
		}
		//이전 메모리 값으로 복원
		*pdwCurrentAddress = dwPreviousValue;

		pdwCurrentAddress += (0x400000 / 4);
	}
	
	//체크가 성공한 어드레스를 1MB로 나누어 MB단위로 계산
	gs_qwTotalRAMMBSize = (QWORD)pdwCurrentAddress / 0x100000;
}

QWORD kGetTotalRAMSize(void)
{
	return gs_qwTotalRAMMBSize;
}

long kAToI(const char* pcBuffer, int iRadix)
{
	long lReturn;
	switch (iRadix)
	{
	case 16:
		lReturn = kHexStringToQword(pcBuffer);
		break;
	case 10:
	default:
		lReturn = kDecimalStringToLong(pcBuffer);
		break;
	}
	return lReturn;
}

//16진수 문자열을 QWORD로 변환
QWORD kHexStringToQword(const char* pcBuffer)
{
	QWORD qwValue = 0;
	int i;

	//문자열 돌면서 변환
	for ( i = 0; pcBuffer[i] != '\0'; i++)
	{
		qwValue *= 16;
		if (('A' <= pcBuffer[i]) && (pcBuffer[i] <='Z'))
		{
			qwValue += (pcBuffer[i] - 'A') + 10;
		}
		else if (('a' <= pcBuffer[i]) && (pcBuffer[i] <= 'z'))
		{
			qwValue += (pcBuffer[i] - 'a') + 10;
		}
		else
		{
			qwValue += pcBuffer[i] - '0';
		}
	}
	return qwValue;
}

//10진수 문자열을 long으로 변환

long kDecimalStringToLong(const char* pcBuffer)
{
	long lValue = 0;
	int i;

	if (pcBuffer[0] == '-')
	{
		i = 1;
	}
	else
	{
		i = 0;
	}

	for (; pcBuffer[i] != '\0'; i++)
	{
		lValue *= 10;
		lValue += pcBuffer[i] - '0';
	}
	if (pcBuffer[0] == '-')
	{
		lValue = -lValue;
	}
	return lValue;

}

int kIToA(long lValue, char* pcBuffer, int iRadix)
{
	int iReturn;

	switch (iRadix)
	{
	case 16:
		iReturn = kHexToString(lValue, pcBuffer);
		break;

	case 10:
	default:
		iReturn = kDecimalToString(lValue, pcBuffer);
		break;
	}
	return iReturn;
}

int kHexToString(QWORD qwValue, char* pcBuffer)
{
	QWORD i;
	QWORD qwCurrentValue;

	if (qwValue == 0)
	{
		pcBuffer[0] = '0';
		pcBuffer[1] = '\0';
		return 1;
	}
	//버퍼의 1의 자리부터 16, 256 ...의 자리 순서로 숫자 삽입
	for (i = 0; qwValue > 0; i++)
	{
		qwCurrentValue = qwValue % 16;
		if (qwCurrentValue >= 10)
		{
			pcBuffer[i] = 'A' + (qwCurrentValue - 10);
		}
		else
		{
			pcBuffer[i] = '0' + qwCurrentValue;
		}
		qwValue = qwValue / 16;
	}
	pcBuffer[i] = '\0';

	kReverseString(pcBuffer);
	return i;
	
}

//10진수 값을 문자열로 변환
int kDecimalToString(long lValue, char* pcBuffer)
{
	long i;

	if (lValue == 0)
	{
		pcBuffer[0] = '0';
		pcBuffer[1] = '\0';
		return 1;
	}

	if (lValue < 0)
	{
		i = 1;
		pcBuffer[0] = '-';
		lValue = -lValue;
	}
	else
	{
		i = 0;
	}
	//버퍼의 1의 자리부터 10 100 10000의 자리 순서로 숫자 삽입
	for (; lValue > 0; i++)
	{
		pcBuffer[i] = '0' + lValue % 10;
		lValue = lValue / 10;
	}
	pcBuffer[i] = '\0';
	if (pcBuffer[0] == '-')
	{
		kReverseString(&(pcBuffer[1]));
	}
	else
	{
		kReverseString(pcBuffer);
	}
	return i;
}

//문자열의 순서 바꿈
void kReverseString(char* pcBuffer)
{
	int iLength;
	int i;
	char cTemp;

	iLength = kStrLen(pcBuffer);
	for ( i = 0; i < iLength / 2; i++)
	{
		cTemp = pcBuffer[i];
		pcBuffer[i] = pcBuffer[iLength - 1 - i];
		pcBuffer[iLength - 1 - i] = cTemp;
	}
}

//sprintf(함수 구현
int kSPrintf(char* pcBuffer,const char* pcFormatString, ...)
{
	va_list ap;
	int iReturn;

	va_start(ap, pcFormatString);
	iReturn = kVSPrintf(pcBuffer, pcFormatString, ap);
	va_end(ap);

	return iReturn;
}
//vsprintf() 함수 내부 구현
int kVSPrintf(char* pcBuffer, const char* pcFormatString, va_list ap)
{
	QWORD i, j, k;
	int iBufferIndex = 0;
	int iFormatLength, iCopyLength;
	char* pcCopyString;
	QWORD qwValue;
	int iValue;
	double dValue;

	iFormatLength = kStrLen(pcFormatString);
	for ( i = 0; i < iFormatLength; i++)
	{
		if (pcFormatString[i] == '%')
		{
			i++;
			switch (pcFormatString[i])
			{
			case 's':
				pcCopyString = (char*)(va_arg(ap, char*));
				iCopyLength = kStrLen(pcCopyString);
				//문자열의 길이만큼을 출력 버퍼로 복사하고 출력한 길이만큼
				//버퍼의 인덱스를 이동
				kMemCpy(pcBuffer + iBufferIndex, pcCopyString, iCopyLength);
				iBufferIndex += iCopyLength;
				break;
			case 'c'://문자
				pcBuffer[iBufferIndex] = (char*)(va_arg(ap, int));
				iBufferIndex++;
				break;
			case 'd':
			case 'i':
				iValue = (int)(va_arg(ap,int));
				iBufferIndex += kIToA(iValue, pcBuffer + iBufferIndex, 10);
				break;
			case'x':
			case 'X':
				qwValue = (DWORD)(va_arg(ap, DWORD)) & 0xFFFFFFFF;
				iBufferIndex += kIToA(qwValue, pcBuffer + iBufferIndex, 16);
				break;

			case 'q':
			case 'Q':
			case 'p':
				qwValue = (QWORD)(va_arg(ap, QWORD));
				iBufferIndex += kIToA(qwValue, pcBuffer + iBufferIndex, 16);
				break;
			
			//소수점 둘째 자리까지 실수를 출력
			case 'f':
				dValue = (double) (va_arg(ap,double));
				//셋째 자리에서 바농림 처리
				dValue += 0.005;
				//소수점 둘째 자리부터 차례로 저장하여 버퍼를 뒤집음
				pcBuffer[iBufferIndex] = '0'+(QWORD)(dValue * 100) % 10;
				pcBuffer[iBufferIndex + 1] = '0' +(QWORD)(dValue * 10)%10;
				pcBuffer[iBufferIndex + 2] = '.';
				for ( k = 0;; k++)
				{
					//정수 부분이 0이면 종료
					if(((QWORD) dValue == 0) && ( k!= 0))
					{
						break;
					}
					pcBuffer[iBufferIndex+3+k] = '0'+((QWORD) dValue % 10);
					dValue = dValue / 10;
				}
				pcBuffer[iBufferIndex + 3 + k] = '\0';
				// 값이 저장된 길이만큼 뒤집고 길이를 증가시킴
				kReverseString(pcBuffer + iBufferIndex);
				iBufferIndex += 3 + k;
				break;

			default:
				pcBuffer[iBufferIndex] = pcFormatString[i];
				iBufferIndex++;
				break;
			}
		}
		else//일반 문자열 처리
		{
			pcBuffer[iBufferIndex] = pcFormatString[i];
			iBufferIndex++;
		}
	}
	pcBuffer[iBufferIndex] = '\0';
	return iBufferIndex;
}

// Tick Count를 반환
QWORD kGetTickCount( void )
{
 return g_qwTickCount;
}

//밀리세컨드 동안 대기
void kSleep(QWORD qwMillisecond)
{
	QWORD qwLastTickCount;

	qwLastTickCount = g_qwTickCount;

	while( ( g_qwTickCount - qwLastTickCount ) <= qwMillisecond )
	{
		kSchedule();
	}
}
