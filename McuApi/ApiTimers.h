/*

  __  __ _       _                                 
 |  \/  (_)     (_)                                
 | \  / |_ _ __  _ _ __ ___   ___  _   _ ___  ___  
 | |\/| | | '_ \| | '_ ` _ \ / _ \| | | / __|/ _ \
 | |  | | | | | | | | | | | | (_) | |_| \__ \  __/ 
 |_|  |_|_|_| |_|_|_| |_| |_|\___/ \__,_|___/\___| 
                                                   
                                                   
Description       : Timer Api.  


License           : Revised BSD License, see LICENSE.TXT file include in the project

Maintainer        : Fabien Holin (SEMTECH)
*/
#ifndef API_TIMERS_H
#define API_TIMERS_H
#include "mbed.h"
#include "MacLayer.h"
    /*!
    * RtcInit Function
    * \remark must be called before any call to initiliaze the timers
    * \param [IN]   void
    * \param [OUT]  void       
    */
    void     RtcInit            ( void ) ;
    
    /*!
    * RtcGetTimeSecond : return the Current Rtc time in Second 
    * \remark is used for :
    * \remark scheduling autonomous retransmissions (for exemple NbTrans) , transmitting MAC answers , basically any delay without accurate time constraints
    * \remark also used to measure the time spent inside the LoRaWAN process for the integrated failsafe
    * \param [IN]   void
    * \param [OUT]  uint32_t RTC time in Second       
    */
    uint32_t RtcGetTimeSecond       ( void ) ;
   /*!
    * RtcGetTimeMs : return the Current Rtc time in Ms 
    * \remark is used to timestamp radio events (end of TX), will also be used for future classB
    * \remark this function may be used by the application.
    * \param [IN]   void
    * \param [OUT]  uint32_t Current RTC time in ms wraps every 49 days       
    */
    uint32_t RtcGetTimeMs  ( void ) ;
    
    /*!
    * WakeUpInit : Init the application wake up rtc timer.  
    * \remark this timer is not used by the LoRaWAN object , only used to wake up from applciation's low power sleep mode.
    * \param [IN]  void
    * \param [OUT] void         
    */
    void     WakeUpInit         ( void ) ;
    
    /*!
    * WakeUpAlarmMSecond : Configures the application wake up timer with a delay duration in ms
    * When the timer expires , the rtc block generates an It to wake up the Mcu 
    * \remark this function is not used by the LoRaWAN object, only provided for application purposes.
    * \param [IN]  int delay in ms
    * \param [OUT] void         
    */
    void     WakeUpAlarmMSecond ( int delay ) ;
    
    /*!
    * WakeUpAlarmMecond : Configure the wake up timer with a delay duration in second
    * \remark this function is not used by the LoRaWAN object, only provided for application purposes.
    * When the timer expires , the rtc block generates an It to wake up the Mcu 
    * 
    * \param [IN]  int delay in s
    * \param [OUT] void         
    */
    void     WakeUpAlarmSecond  ( int delay ) ;
    
    /*!
    * A function to set the mcu in low power mode  for duration seconds
     * \remark inside this function watchdog has to be manage to not reset the mcu
     * \param [IN]   int delay 
     * \param [OUT]  void       
     */
    void     GotoSleepSecond    ( int delay );
    
    /*!
    * A function to set the mcu in low power mode  for duration in milliseconds
    * \remark 
    * \param [IN]   int delay 
    * \param [OUT]  void       
    */
    void     GotoSleepMSecond   ( int delay );
    
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

#define WATCH_DOG_PERIOD_RELEASE 30 // this period have to be lower than the Watch Dog period of 32 seconds
    /* A function to init and start the Watchdog 
    * \remark The expired period = WATCH_DOG_PERIOD_RELEASE seconds
    * \param [IN]   void  
    * \param [OUT]  void       
    */
    void WatchDogStart ( void ) ;

    /* A function to release the Watchdog 
    * \remark Application have to call this function periodically (with a period <WATCH_DOG_PERIOD_RELEASE)
    *         If not , the mcu will reset.
    * \param [IN]   void  
    * \param [OUT]  void       
    */
    void WatchDogRelease ( void ) ;
#endif

