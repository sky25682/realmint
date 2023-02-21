#ifndef __RTC_H__
#define __RTC_H__

#include "Types.h"
//매크로

//ㅑi/o 포트
#define RTC_CMOSADDRESS	0X70
#define RTC_CMOSDATA	0X71

// CMOS 메모리 어드레스
#define RTC_ADDRESS_SECOND	0X00
#define RTC_ADDRESS_MINUTE	0X02
#define RTC_ADDRESS_HOUR	0X04
#define RTC_ADDRESS_DAYOFWEEK	0X06
#define RTC_ADDRESS_DAYOFMONTH	0X07
#define RTC_ADDRESS_MONTH	0X08
#define RTC_ADDRESS_YEAR	0X09

//BCD 포맷을 bINARY로 변환하는 매크로
#define RTC_BCDTOBINARY(x)	( ( ( ( x ) >> 4 ) * 10 ) + ( ( x ) & 0x0F ) )

//함수
void kReadRTCTime(BYTE* pbHour, BYTE* pbMinute, BYTE* pbSecond);
void kReadRTCDate(WORD* pwYear, BYTE* pbMonth, BYTE* pbDayOfMonth, BYTE* pbDayOfWeek);
char* kConvertDayOfWeekToString(BYTE bDayOdWeek);

#endif // !__RTC_H__

