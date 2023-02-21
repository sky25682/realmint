#include "PIC.h"

void kInitializePIC(void)
{
	// 마스터 PIC 컨트롤러 초기화
	kOutPortByte(PIC_MASTER_PORT1, 0x11);

	kOutPortByte(PIC_MASTER_PORT2, PIC_IRQSTARTVECTOR);

	kOutPortByte(PIC_MASTER_PORT2, 0x04);

	kOutPortByte(PIC_MASTER_PORT2, 0x01);

	// 슬레이브 PIC 컨트롤러를 초기화
	kOutPortByte(PIC_SLAVE_PORT1, 0x11);
	kOutPortByte(PIC_MASTER_PORT2, PIC_IRQSTARTVECTOR + 8);

	kOutPortByte(PIC_SLAVE_PORT2, 0x02);

	kOutPortByte(PIC_SLAVE_PORT2, 0x01);

}

void kMaskPICInterrupt(WORD wIRQBitmask)
{
	//마스터 PIIC 컨트롤러에 IMR 설정
	kOutPortByte(PIC_MASTER_PORT2, (BYTE)wIRQBitmask );

	//슬레이브 PIC 컨트롤러에 imr 설정
	kOutPortByte(PIC_SLAVE_PORT2, (BYTE)(wIRQBitmask >> 8));
}

void kSendEOIToPIC(int iIRQNumber)
{
	kOutPortByte(PIC_MASTER_PORT1, 0x20);

	if (iIRQNumber >= 8)
	{
		kOutPortByte(PIC_SLAVE_PORT1, 0x20); //a0 ? 20?
	}

}

