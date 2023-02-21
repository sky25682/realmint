#include "InterruptHandler.h"
#include "PIC.h"
#include "Keyboard.h"
#include "Console.h"
#include "Utility.h"
#include "Task.h"
#include "Descriptor.h"
#include "AssemblyUtility.h"
#include "HardDisk.h"

void kCommonExceptionHandler(int iVectorNumber, QWORD qwErrorCode)
{
	char vcBuffer[3] = { 0, };

	//인터럽트 벡터를 화면 오른쪽 위에 2자리 정수로 출력
	vcBuffer[0] = '0' + iVectorNumber / 10;
	vcBuffer[1] = '0' + iVectorNumber % 10;

	kPrintStringXY(0, 0, "===================================================");
	kPrintStringXY(0, 1, "                Exception Occur~!!!!               ");
	kPrintStringXY(0, 2, "                    Vector:                         ");
	kPrintStringXY(27, 2, vcBuffer);
	kPrintStringXY(0, 3, "===================================================");

	while (1);
}

// 공통으로 사용하는 인터럽트 핸들러
void kCommonInterruptHandler(int iVectorNumber)
{
	char vcBuffer[] = "[INT: , ]";
	static int g_iCommonInterruptCount = 0;
	
	//=====================================================================
	// 인터럽트가 발생했음을 알리려고 메시지를 출력하는 부분
	// 인터럽트 벡터를 화면 오른쪽 위에 2자리 정수로 출력
	vcBuffer[5] = '0' + iVectorNumber / 10;
	vcBuffer[6] = '0' + iVectorNumber % 10;
	// 발생한 횟수 출력
	vcBuffer[8] = '0' + g_iCommonInterruptCount;
	g_iCommonInterruptCount = (g_iCommonInterruptCount + 1) % 10;
	kPrintStringXY(70, 0, vcBuffer);
	//============================================== =======================
	
	// EOI 전송
	kSendEOIToPIC(iVectorNumber - PIC_IRQSTARTVECTOR);
}

// 키보드 인터럽트의 핸들러
void kKeyboardHandler(int iVectorNumber)
{
	char vcBuffer[] = "[INT: , ]";
	static int g_iKeyboardInterruptCount = 0;
	BYTE bTemp;
	
	
	//=====================================================================
	// 인터럽트가 발생했음을 알리려고 메시지를 출력하는 부분
	// 인터럽트 벡터를 화면 왼쪽 위에 2자리 정수로 출력
	vcBuffer[5] = '0' + iVectorNumber / 10;
	vcBuffer[6] = '0' + iVectorNumber % 10;
	// 발생한 횟수 출력
	vcBuffer[8] = '0' + g_iKeyboardInterruptCount;
	g_iKeyboardInterruptCount = (g_iKeyboardInterruptCount + 1) % 10;
	kPrintStringXY(0, 0, vcBuffer);
	//================================================= ====================

	 // 키보드 컨트롤러에서 데이터를 읽어서 ASCII로 변환하여 큐에 삽입

	if(kIsOutputBufferFull() == TRUE )
	{
		bTemp = kGetKeyboardScanCode();
		kConvertScanCodeAndPutQueue( bTemp );
	}

	// EOI 전송
	kSendEOIToPIC(iVectorNumber - PIC_IRQSTARTVECTOR);
}


//타이머 인터럽트의 핸들러
void kTimerHandler(int iVecterNumber)
{
	char vcBuffer[] = "[INT:  , ]";
	static int g_iTimerInterruptCount = 0;

	//=========================================================== ==========
	// 인터럽트가 발생했음을 알리려고 메시지를 출력하는 부분
	// 인터럽트 벡터를 화면 오른쪽 위에 2자리 정수로 출력
	vcBuffer[5] = '0' + iVecterNumber / 10;
	vcBuffer[6] = '0' + iVecterNumber % 10;
	//발생한 횟수 출력
	vcBuffer[8] = '0' + g_iTimerInterruptCount;
	g_iTimerInterruptCount = (g_iTimerInterruptCount + 1) % 10;
	kPrintStringXY(70, 0, vcBuffer);
	//=======================================================================

	//eoi 전송
	kSendEOIToPIC(iVecterNumber - PIC_IRQSTARTVECTOR);

	//타이머 발생 횟수를 증가
	g_qwTickCount++;

	//태스크가 사용한 프로세서의 시가늘 줄임
	kDecreaseProcessorTime();
	//프로세서가 사용할 수 있는 시간을 다 썻다면 태스크 전환 수행
	if (kIsProcessorTimeExpired() == TRUE)
	{
		kScheduleInInterrupt();
	}
}

//Device Not Available 예외의 핸들러
void kDeviceNotAvailableHandler(int iVecterNumber)
{
	TCB* pstFPUTask, * pstCurrentTask;
	QWORD qwLastFPUTaskID;

	//====================================================================
	// FPU 예외가 발생했음을 알리려고 메시지를 출력하는 부분
	char vcBuffer[] = "[EXC:  , ]";
	static int g_iFPUInterruptCount = 0;

	//예외 벡터를 화며 오른쪽 위에 2자리 정수로 출력
	vcBuffer[5] = '0' + iVecterNumber/10;
	vcBuffer[6]='0'+iVecterNumber%10;
	//발생한 횟수 출력
	vcBuffer[8] = '0'+ g_iFPUInterruptCount;
	g_iFPUInterruptCount = (g_iFPUInterruptCount + 1 )%10;
	kPrintStringXY(0,0,vcBuffer);
	//==================================================================

	//cr0 컨트롤 레지스터의 ts 비트를 0으로 설정
	kClearTS();

	//이전에 FPU를 사용한 태스크가 있는지 확인해 있다면 FPU의 상태를 태스크에 저장
	qwLastFPUTaskID = kGetLastFPUUsedTaskID();
	pstCurrentTask = kGetRunningTask();

	//이전에 FPU를 사용한 것이 자신이면 아무것도 안 함
	if(qwLastFPUTaskID == pstCurrentTask->stLink.qwID)
	{
		return ;
	}
	//FPU를 사용한 태스크가 있으면 FPU 상태를 저장
	else if( qwLastFPUTaskID != TASK_INVALIDID)
	{
		pstFPUTask = kGetTCBInTCBPool(GETTCBOFFSET(qwLastFPUTaskID));
		if((pstFPUTask != NULL) && ( pstFPUTask->stLink.qwID == qwLastFPUTaskID))
		{
			kSaveFPUContext(pstFPUTask->vqwFPUContext);
		}
		//FPU를 사용한 태스크 ID를 현재 태스크로 변경
		kSetLastFPUUsedTaskID(pstCurrentTask->stLink.qwID);
	}
}

// 하드디스크에서 발생하는 인터럽트의 핸들러
void kHDDHandler(int iVectorNumber)
{
	char vcBuffer[] = "[INT:  , ]";
	static int g_iHDDInterruptCount = 0;
	BYTE bRemp;

	//==========================================================================
	//인터럽트가 발생했음을 알리려고 메시지를 출력하는 부분
	//인터럽트 벡터를 화면 왼쪽 위에 두 자리 정수로 출력
	vcBuffer[5] = '0'+iVectorNumber /10;
	vcBuffer[6] = '0' + iVectorNumber % 10;
	//발생한 횟수 출력
	vcBuffer[8] = '0' + g_iHDDInterruptCount;
	g_iHDDInterruptCount = ( g_iHDDInterruptCount + 1) % 10;
	//왼쪽 위에 있는 메시지와 겹치지 않도록 10,0 엣 출력
	kPrintStringXY(10,0,vcBuffer);
	//========================================================================

	//첫 번째 PATA 포트의 인터럽트 벡터(IRQ 14) 처리
	if(iVectorNumber - PIC_IRQSTARTVECTOR == 14)
	{
		//첫 번째 PATA 포트의 인터럽트 발생 여부를 TRUE로 설정
		kSetHDDInterruptFlag(TRUE, TRUE);
	}
	else{
	//두 번째 PATA 포트의 인터럽트 발생 여부를 TRUE로 설정
	kSetHDDInterruptFlag(FALSE, TRUE);
	}
	// EOI 전송
	kSendEOIToPIC(iVectorNumber - PIC_IRQSTARTVECTOR);
}