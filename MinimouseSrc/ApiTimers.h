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
#ifndef API_TIMERS_H
#define API_TIMERS_H
#include "mbed.h"
#include "ApiFlash.h"
#include "MacLayer.h"
void     RtcInit            ( void ) ;
uint32_t RtcGetTimeMs       ( void ) ;
uint32_t RtcGetTimeSecond   ( void ) ;
void     WakeUpInit         ( void ) ;
void     WakeUpAlarmMSecond ( int delay) ;
void     WakeUpAlarmSecond  ( int delay) ;
void     GotoSleep          (void );

class LowPowerTimerLoRa {
public : 
    LowPowerTimerLoRa          ( );
    ~LowPowerTimerLoRa         ( ){};
    void LowPowerTimerLoRaInit ( void );
    void StartTimerMsecond     ( void (* _Func) (void *) , void * _obj, int delay) ;
    void timerISR              ( void ) { Func(obj); };
private :
    static void DoNothing (void *) { };
    void (* Func) (void *);
    void * obj;
};
extern    LowPowerTimerLoRa LowPowerTimerLora;

#endif

