#include "RTC.h"

//CMOS 메모리에서 RTC 컨트롤러가 저장한 현재 시간을 읽음
void kReadRTCTime(BYTE* pbHour, BYTE* pbMinute, BYTE* pbSecond)
{
	BYTE bData;

	//Cmos 메모리 어드레스 레지스터 포트0x07에 시간을 저장하는 레지스터 지정
	kOutPortByte(RTC_CMOSADDRESS, RTC_ADDRESS_HOUR);
	//cmos 데이터 레지스터 포트0x71애서 시간을 읽음
	bData = kInPortByte(RTC_CMOSDATA);
	*pbHour = RTC_BCDTOBINARY(bData);

	//분저장 레지스터 지정 0ㅌ70
	kOutPortByte(RTC_CMOSADDRESS, RTC_ADDRESS_MINUTE);
	bData = kInPortByte(RTC_CMOSDATA);
	*pbMinute = RTC_BCDTOBINARY(bData);

	kOutPortByte(RTC_CMOSADDRESS, RTC_ADDRESS_SECOND);
	bData = kInPortByte(RTC_CMOSDATA);
	*pbMinute = RTC_BCDTOBINARY(bData);
	
}

//cmod 메모리에서 rtc 컨트롤러가 저장한 현재 일자를 읽음
void kReadRTCDate(WORD* pwYear, BYTE* pbMonth, BYTE* pbDayOfMonth, BYTE* pbDayOfWeek)
{
	BYTE bData;

	//년도 저장
	kOutPortByte(RTC_CMOSADDRESS, RTC_ADDRESS_YEAR);
	bData = kInPortByte(RTC_CMOSDATA);
	*pwYear = RTC_BCDTOBINARY(bData) + 2000;
	//월 저장
	kOutPortByte(RTC_CMOSADDRESS, RTC_ADDRESS_MONTH);
	bData = kInPortByte(RTC_CMOSDATA);
	*pbMonth = RTC_BCDTOBINARY(bData);

	//일 저장
	kOutPortByte(RTC_CMOSADDRESS, RTC_ADDRESS_DAYOFMONTH);
	bData = kInPortByte(RTC_CMOSDATA);
	*pbDayOfMonth = RTC_BCDTOBINARY(bData);

	kOutPortByte(RTC_CMOSADDRESS, RTC_ADDRESS_DAYOFWEEK);
	bData = kInPortByte(RTC_CMOSDATA);
	*pbDayOfWeek = RTC_BCDTOBINARY(bData);
}

char* kConvertDayOfWeekToString(BYTE bDayOfWeek)
{
	static char* vpcDayOfWeekString[8] = { "Error","Sunday","MondaY","Tuesday","Wendnesday","Thursday","Friday","Saturday" };

	//요일 범위가 넘어가면 에러를 반환
	if (bDayOfWeek >= 8)
	{
		return vpcDayOfWeekString[0];
	}
	//요일을 반환
	return vpcDayOfWeekString[bDayOfWeek];
}
