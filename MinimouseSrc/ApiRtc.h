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
void my_rtc_init( void );
uint32_t RtcGetTimeMs( void ) ;//uint32_t  *Seconds, uint16_t * SubSeconds);
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
#endif