/*

  __  __ _       _                                 
 |  \/  (_)     (_)                                
 | \  / |_ _ __  _ _ __ ___   ___  _   _ ___  ___  
 | |\/| | | '_ \| | '_ ` _ \ / _ \| | | / __|/ _ \
 | |  | | | | | | | | | | | | (_) | |_| \__ \  __/ 
 |_|  |_|_|_| |_|_|_| |_| |_|\___/ \__,_|___/\___| 
                                                   
                                                   
Description       : Flash Api.  


License           : Revised BSD License, see LICENSE.TXT file include in the project

Maintainer        : Fabien Holin (SEMTECH)
*/
#ifndef APIRTC_H
#define APIRTC_H
#include "mbed.h"
void     my_rtc_init     ( void ) ;
uint32_t RtcGetTimeMs    ( void ) ;//uint32_t  *Seconds, uint16_t * SubSeconds);
uint32_t RtcGetTimeSecond( void ) ;//uint32_t  *Seconds, uint16_t * SubSeconds);
void myrtc_write(  time_t t );
void mysleep (int time);
extern RTC_HandleTypeDef RtcHandle;
extern Timer timerglobal ; 
void TimerLoraInit(void);
int GetTime(void);
void RtcSetAlarm( void );
void RtcGetAlarm( void );
void wait_s ( int t );

  

/* Specific implementation for STM32 . The following implementation is just to put in place
a setalarm function as described on the architecture specification of minimouse */

#define RTC_INIT_DATE 1
#define RTC_INIT_MONTH  RTC_MONTH_JANUARY
#define RTC_INIT_YEAR  0
#define RTC_INIT_WEEKDAY RTC_WEEKDAY_SATURDAY
#define RTC_INIT_SUBSEC_SECONDS_MINUTES_HOURS 0 /*time at 0:0:0*/

#define END_OF_FEBRUARY_LEAP 60 //31+29
#define END_OF_JULY_LEAP    213 //31+29+...

#define END_OF_FEBRUARY_NORM 59 //31+28
#define END_OF_JULY_NORM    212 //31+28+...

#define DIV_1461(X) (( (X)*91867+22750)>>25)
   
#define DIV_APPROX_86400(X)    ( ((X)>>18) +( (X)>>17))
   
#define DIV_1461(X) (( (X)*91867+22750)>>25)
   
#define DIV_APPROX_1000(X)    ( ((X)>>10) +( (X)>>16) + ( (X)>>17))
   
#define DIV_APPROX_60(X)   ( ( (X) *17476 )>>20 )
   
#define DIV_APPROX_61(X) (((X) *68759)>>22)
   
#define MODULO_7(X)   ( (X) -(( ( ((X)+1)  *299593)>>21)*7))

#define  DAYS_IN_MONTH_CORRECTION_NORM     ((uint32_t) 0x99AAA0 )
#define  DAYS_IN_MONTH_CORRECTION_LEAP     ((uint32_t) 0x445550 )

#define  DAYS_IN_LEAP_YEAR (uint32_t) 366

#define  DAYS_IN_YEAR      (uint32_t) 365

#define  SECONDS_IN_1DAY   (uint32_t) 86400

#define  SECONDS_IN_1HOUR   (uint32_t) 3600

#define  SECONDS_IN_1MINUTE   (uint32_t) 60

#define  MINUTES_IN_1HOUR    (uint32_t) 60

#define  HOURS_IN_1DAY      (uint32_t) 24

/* Calculates ceiling(X/N) */
#define DIVC(X,N)   ( ( (X) + (N) -1 ) / (N) )

#define DIVC_BY_4(X)   ( ( (X) + 3 ) >>2 )

#define DIVC_BY_2(X)   ( ( (X) + 1 ) >>1 )



/* subsecond number of bits */
#define N_PREDIV_S                 10

/* Synchonuous prediv  */
#define PREDIV_S                  ((1<<N_PREDIV_S)-1)

/* Asynchonuous prediv   */
#define PREDIV_A                  (1<<(15-N_PREDIV_S))-1



/* Sub-second mask definition  */
#define HW_RTC_ALARMSUBSECONDMASK N_PREDIV_S<<RTC_ALRMASSR_MASKSS_Pos

/* RTC Time base in us */
#define USEC_NUMBER               1000000
#define MSEC_NUMBER               (USEC_NUMBER/1000)
#define RTC_ALARM_TIME_BASE       (USEC_NUMBER>>N_PREDIV_S)

#define COMMON_FACTOR        3
#define CONV_NUMER                (MSEC_NUMBER>>COMMON_FACTOR)
#define CONV_DENOM                (1<<(N_PREDIV_S-COMMON_FACTOR))


__STATIC_INLINE uint32_t cal_div_61( uint32_t in )
{
#if 0
  return  (in / 61 );
#else

  uint32_t out_temp=0;
  uint32_t div_result=DIV_APPROX_61(in);
  while (div_result >=1 )
  {
    out_temp+=div_result;
    in -= div_result*61;
    div_result=DIV_APPROX_61(in);
  }
  if( in>= 61 )
  {
    out_temp+=1;
    in -= 61;
  }
  return out_temp;
#endif
}

__STATIC_INLINE uint32_t cal_get_month(uint32_t Days, uint32_t year)
{
  uint32_t month;
  if ( (year%4) ==0  )
  {  /*leap year*/
    if ( Days<END_OF_FEBRUARY_LEAP )
    { /*January or february*/
     // month =  Days*2/(30+31);
      month =  cal_div_61(Days*2);
    }
    else if ( Days<END_OF_JULY_LEAP )
    {
      month =  cal_div_61((Days-END_OF_FEBRUARY_LEAP)*2)+2;
    }
    else
    {
      month =  cal_div_61((Days-END_OF_JULY_LEAP)*2)+7;
    }
  }
  else
  {
    if ( Days<END_OF_FEBRUARY_NORM )
    { /*January of february*/
      month =  cal_div_61(Days*2);
    }
    else if ( Days<END_OF_JULY_NORM )
    {
      month =  cal_div_61((Days-END_OF_FEBRUARY_NORM)*2)+2;
    }
    else
    {
      month =  cal_div_61((Days-END_OF_JULY_NORM)*2)+7;
    }
  }
  return month;
}

__STATIC_INLINE void cal_div_86400( uint32_t in, uint32_t* out, uint32_t* remainder )
{
#if 0
  *remainder= total_nb_seconds % SECONDS_IN_1DAY;
  * nbDays =     total_nb_seconds / SECONDS_IN_1DAY;
#else

  uint32_t out_temp=0;
  uint32_t div_result=DIV_APPROX_86400(in);
  while (div_result >=1 )
  {
    out_temp+=div_result;
    in -= div_result*86400;
    div_result=DIV_APPROX_86400(in);
  }
  if( in>=86400 )
  {
    out_temp+=1;
    in -= 86400;
  }

  *remainder = in;
  *out = out_temp;
#endif
}



__STATIC_INLINE void cal_div_1000( uint32_t in, uint32_t* out, uint32_t* remainder )
{
#if 0
  *remainder= total_nb_seconds % 1000;
  *out      = total_nb_seconds / 1000;
#else

  uint32_t out_temp=0;
  uint32_t div_result=DIV_APPROX_1000(in);
  while (div_result >=1 )
  {
    out_temp+=div_result;
    in -= div_result*1000;
    div_result=DIV_APPROX_1000(in);
  }
  if( in>= 1000 )
  {
    out_temp+=1;
    in -= 1000;
  }
  *remainder = in;
  *out = out_temp;
#endif
}

__STATIC_INLINE void cal_div_60( uint32_t in, uint32_t* out, uint32_t* remainder )
{
#if 0
  *remainder= total_nb_seconds % 60;
  *out      = total_nb_seconds / 60;
#else

  uint32_t out_temp=0;
  uint32_t div_result=DIV_APPROX_60(in);
  while (div_result >=1 )
  {
    out_temp+=div_result;
    in -= div_result*60;
    div_result=DIV_APPROX_60(in);
  }
  if( in>= 60 )
  {
    out_temp+=1;
    in -= 60;
  }
  *remainder = in;
  *out = out_temp;
#endif
}

__STATIC_INLINE uint64_t cal_convertBCD_2_Cnt64( RTC_DateTypeDef* RTC_DateStruct, RTC_TimeTypeDef* RTC_TimeStruct )
{
  uint64_t cnt64 = 0;
  
  uint32_t nbdays ;
  
  uint32_t nbsecs;
   
  uint32_t correction_month[4]={DAYS_IN_MONTH_CORRECTION_LEAP,
                                DAYS_IN_MONTH_CORRECTION_NORM,
                                DAYS_IN_MONTH_CORRECTION_NORM,
                                DAYS_IN_MONTH_CORRECTION_NORM};
  
  nbdays= DIVC( (DAYS_IN_YEAR*3 + DAYS_IN_LEAP_YEAR)* RTC_DateStruct->Year , 4);
 
  nbdays +=( DIVC_BY_2( (RTC_DateStruct->Month-1)*(30+31) ) - (((correction_month[RTC_DateStruct->Year % 4]>> ((RTC_DateStruct->Month-1)*2) )&0x3)));

  nbdays += (RTC_DateStruct->Date -1);
  
  /* convert from days to seconds */
  cnt64 = nbdays* SECONDS_IN_1DAY; 

  nbsecs = ( ( uint32_t )RTC_TimeStruct->Seconds + 
                     ( ( uint32_t )RTC_TimeStruct->Minutes * SECONDS_IN_1MINUTE ) +
                     ( ( uint32_t )RTC_TimeStruct->Hours * SECONDS_IN_1HOUR ) ) ;

  cnt64 = nbdays * SECONDS_IN_1DAY + nbsecs ; 
  
  cnt64 = (cnt64<<N_PREDIV_S) + ( RTC_TimeStruct->SubSeconds);
  
  return cnt64;
}



__STATIC_INLINE void cal_convert_Cnt64_2_Bcd( RTC_DateTypeDef* Date, RTC_TimeTypeDef* Time, uint64_t cnt64 )
{
  uint32_t correction_month[4]={DAYS_IN_MONTH_CORRECTION_LEAP,
                                     DAYS_IN_MONTH_CORRECTION_NORM,
                                     DAYS_IN_MONTH_CORRECTION_NORM,
                                     DAYS_IN_MONTH_CORRECTION_NORM};
  uint32_t weekDays = (RTC_INIT_WEEKDAY-1);
  uint32_t cnt32 = cnt64>> N_PREDIV_S; /*total in seconds (136 year)*/
  uint32_t seconds;
  uint32_t minutes;
  uint32_t Days;
  uint32_t div_out;
  uint32_t div_rem;

  cal_div_86400(cnt32, &Days, &seconds);

  Time->SubSeconds = ( cnt64 & PREDIV_S);

  /* calculates seconds */
  cal_div_60(seconds, &minutes, &div_rem);
  Time->Seconds=(uint8_t) div_rem;

  /* calculates minutes and hours*/
  cal_div_60(minutes, &div_out, &div_rem);
  Time->Minutes = (uint8_t) div_rem;
  Time->Hours = (uint8_t)   div_out;
 /* calculates Year */
  Date->Year = DIV_1461(Days);
  Days-= DIVC_BY_4( (DAYS_IN_YEAR*3 + DAYS_IN_LEAP_YEAR)* Date->Year );

  /*calculates month*/
  Date->Month = cal_get_month( Days, Date->Year) ;

  /*calculates weekdays*/
  weekDays +=  DIVC_BY_4((Date->Year*5));
  weekDays +=  Days;
  Date->WeekDay  = MODULO_7( weekDays );

  Days -=( DIVC_BY_2( (Date->Month)*(30+31) ) - (((correction_month[Date->Year % 4]>> ((Date->Month)*2) )&0x3)));

  /* convert 0 to 1 indexed. */
  Date->WeekDay ++ ;/* 0 to 1 indexed.  month={0..6} to {1..7}*/
  Date->Month++; /* 0 to 1 indexed.  month={0..11} to {1..12}*/
  Date->Date = (Days+1);/* 0 to 1 indexed */
}






#endif