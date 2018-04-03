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
#include "mbed.h"
#include "Define.h"
#include "rtc_api.h"
#include "LoraWanProcess.h"
#include "mbed.h"
#include "ApiTimers.h"
#include "Define.h"
static RTC_HandleTypeDef RtcHandle;
//static int globalt ;
//void (*globalfunc)(void);
LowPowerTimerLoRa LowPowerTimerLora;
/*!
 * Irq Handler dedicated for wake up It
 * 
 * \param [IN]  void
 * \param [OUT] void         
 */
static void RTC_IRQHandlerWakeUp (void)
{
    RtcHandle.Instance = RTC;
    HAL_RTCEx_DeactivateWakeUpTimer(&RtcHandle);
    HAL_RTCEx_WakeUpTimerIRQHandler(&RtcHandle);
    __HAL_RTC_WAKEUPTIMER_EXTI_ENABLE_EVENT();
}

/*!
* WakeUpInit : Init the application wake up rtc timer.  
 * \remark this timer is not used by the LoRaWAN object , only used to wake up from applciation's low power sleep mode.
 * \param [IN]  void
 * \param [OUT] void         
 */

void WakeUpInit ( void ) {
      RtcHandle.Instance = RTC;
      NVIC_ClearPendingIRQ(RTC_WKUP_IRQn);
      NVIC_DisableIRQ(RTC_WKUP_IRQn);
      NVIC_SetVector(RTC_WKUP_IRQn, (uint32_t)RTC_IRQHandlerWakeUp);
      NVIC_EnableIRQ(RTC_WKUP_IRQn);
}

/*!
* WakeUpAlarmMSecond : Configures the application wake up timer with a delay duration in ms
 * When the timer expires , the rtc block generates an It to wake up the Mcu 
 * \remark this function is not used by the LoRaWAN object, only provided for application purposes.
 * \param [IN]  int delay in ms
 * \param [OUT] void         
 */

void WakeUpAlarmMSecond ( int delay) {
    int DelayMs2tick = delay * 2 + ( ( 6 * delay ) >> 7);
    HAL_RTCEx_SetWakeUpTimer_IT(&RtcHandle, DelayMs2tick, RTC_WAKEUPCLOCK_RTCCLK_DIV16);
}
/*!
* WakeUpAlarmMecond : Configure the wake up timer with a delay duration in second
 * \remark this function is not used by the LoRaWAN object, only provided for application purposes.
 * When the timer expires , the rtc block generates an It to wake up the Mcu 
 * 
 * \param [IN]  int delay in s
 * \param [OUT] void         
 */
void WakeUpAlarmSecond ( int delay) {
    HAL_RTCEx_SetWakeUpTimer_IT(&RtcHandle, delay, 4);
}

/*!
* RtcGetTimeMs : return the Current Rtc time in Ms 
 * \remark is used to timestamp radio events (end of TX), will also be used for future classB
* \remark this function may be used by the application.
 * \param [IN]   void
 * \param [OUT]  uint32_t Current RTC time in ms wraps every 49 days       
 */
uint32_t RtcGetTimeMs( void )
{
    RTC_DateTypeDef dateStruct;
    RTC_TimeTypeDef timeStruct;
    struct tm timeinfo;
    RtcHandle.Instance = RTC;
    HAL_RTC_GetTime(&RtcHandle, &timeStruct, FORMAT_BIN);
    HAL_RTC_GetDate(&RtcHandle, &dateStruct, FORMAT_BIN);
    timeinfo.tm_wday = dateStruct.WeekDay;
    timeinfo.tm_mon  = dateStruct.Month ;
    timeinfo.tm_mday = dateStruct.Date;
    timeinfo.tm_year = dateStruct.Year ;
    timeinfo.tm_hour = timeStruct.Hours;
    timeinfo.tm_min  = timeStruct.Minutes;
    timeinfo.tm_sec  = timeStruct.Seconds;
    // Convert to timestamp
    time_t t = mktime(&timeinfo);
    return ( ( t * 1000 ) + ( 999 - ( ( timeStruct.SubSeconds *999) / 255 ) ) );  // get time en ms
}

/*!
* RtcGetTimeSecond : return the Current Rtc time in Second 
* \remark is used for :
* \remark scheduling autonomous retransmissions (for exemple NbTrans) , transmitting MAC answers , basically any delay without accurate time constraints
* \remark also used to measure the time spent inside the LoRaWAN process for the integrated failsafe
 * \param [IN]   void
 * \param [OUT]  uint32_t RTC time in Second       
 */
uint32_t RtcGetTimeSecond( void )
{
    RTC_DateTypeDef dateStruct;
    RTC_TimeTypeDef timeStruct;
    struct tm timeinfo;
    RtcHandle.Instance = RTC;

    HAL_RTC_GetTime(&RtcHandle, &timeStruct, FORMAT_BIN);
    HAL_RTC_GetDate(&RtcHandle, &dateStruct, FORMAT_BIN);
    timeinfo.tm_wday = dateStruct.WeekDay;
    timeinfo.tm_mon  = dateStruct.Month ;
    timeinfo.tm_mday = dateStruct.Date;
    timeinfo.tm_year = dateStruct.Year ;
    timeinfo.tm_hour = timeStruct.Hours;
    timeinfo.tm_min  = timeStruct.Minutes;
    timeinfo.tm_sec  = timeStruct.Seconds;
    time_t t = mktime(&timeinfo);
    return ( t );
}

/*!
* RtcInit Function
 * \remark must be called before any call to initiliaze the timers
 * \param [IN]   void
 * \param [OUT]  void       
 */
void RtcInit (void)
{
    uint32_t rtc_freq = 0;
    // RTC_HandleTypeDef RtcHandle;
    RCC_OscInitTypeDef RCC_OscInitStruct;
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;
    HAL_PWR_EnableBkUpAccess();
    RtcHandle.Instance = RTC;
    __PWR_CLK_ENABLE();
    // Reset Backup domain
    __HAL_RCC_BACKUPRESET_FORCE();
    __HAL_RCC_BACKUPRESET_RELEASE();
    // Enable LSE Oscillator
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_NONE; // Mandatory, otherwise the PLL is reconfigured!
    RCC_OscInitStruct.LSEState       = RCC_LSE_ON; // External 32.768 kHz clock on OSC_IN/OSC_OUT
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) == HAL_OK) {
        // Connect LSE to RTC
        __HAL_RCC_RTC_CLKPRESCALER(RCC_RTCCLKSOURCE_LSE);
        __HAL_RCC_RTC_CONFIG(RCC_RTCCLKSOURCE_LSE);
        rtc_freq = LSE_VALUE;
    }
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
        DEBUG_MSG("PeriphClkInitStruct RTC failed with LSE\n");
    }
    // Enable RTC
    __HAL_RCC_RTC_ENABLE();
    RtcHandle.Init.HourFormat     = RTC_HOURFORMAT_24;
    RtcHandle.Init.AsynchPrediv   = 127;
    RtcHandle.Init.SynchPrediv    = (rtc_freq / 128 )-1 ;
    RtcHandle.Init.OutPut         = RTC_OUTPUT_DISABLE;
    RtcHandle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    RtcHandle.Init.OutPutType     = RTC_OUTPUT_TYPE_OPENDRAIN;
    if (HAL_RTC_Init(&RtcHandle) != HAL_OK) {
        DEBUG_MSG("RTC error: RTC initialization failed.");
    }
    /*init time to 0*/
    time_t t = 0;
    struct tm * timeinfo ;
    timeinfo = localtime (&t) ;
    RTC_DateTypeDef dateStruct;
    RTC_TimeTypeDef timeStruct;
    // Fill RTC structures
    dateStruct.WeekDay        = timeinfo->tm_wday;
    dateStruct.Month          = timeinfo->tm_mon + 1;
    dateStruct.Date           = timeinfo->tm_mday;
    dateStruct.Year           = timeinfo->tm_year;
    timeStruct.Hours          = timeinfo->tm_hour;
    timeStruct.Minutes        = timeinfo->tm_min;
    timeStruct.Seconds        = timeinfo->tm_sec;
    timeStruct.TimeFormat     = RTC_HOURFORMAT_24;
    timeStruct.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    timeStruct.StoreOperation = RTC_STOREOPERATION_RESET;
    // Change the RTC current date/time
    HAL_RTC_SetDate(&RtcHandle, &dateStruct, FORMAT_BIN);
    HAL_RTC_SetTime(&RtcHandle, &timeStruct, FORMAT_BIN);

}

/*!
* A function to set the mcu in low power mode  for duration seconds
 * \remark inside this function watchdog has to be manage to not reset the mcu
 * \param [IN]   void
 * \param [OUT]  void       
 */
void GotoSleepSecond (int duration ) {
#if LOW_POWER_MODE == 1
    int cpt = duration ;
    WatchDogRelease ( );
    while ( cpt > ( WATCH_DOG_PERIOD_RELEASE ) ) {
        cpt -= WATCH_DOG_PERIOD_RELEASE ;
        WakeUpAlarmSecond( WATCH_DOG_PERIOD_RELEASE );
        sleep();
        WatchDogRelease ( );
    }
    WakeUpAlarmSecond( cpt );
    sleep();
    WatchDogRelease ( );
# else
    int cpt = duration ;
    WatchDogRelease ( );
    while ( cpt > ( WATCH_DOG_PERIOD_RELEASE ) ) {
        cpt -= WATCH_DOG_PERIOD_RELEASE ;
        wait( WATCH_DOG_PERIOD_RELEASE );
        WatchDogRelease ( );
    }
    wait( cpt );
    WatchDogRelease ( );
#endif
}


/*!
* A function to set the mcu in low power mode  for duration in milliseconds
 * \remark 
 * \param [IN]   void
 * \param [OUT]  void       
 */
void GotoSleepMSecond (int duration ) {
#if LOW_POWER_MODE == 1
    WakeUpAlarmMSecond ( duration );
    sleep();
# else
    wait_ms ( duration ) ;
#endif
}
static LPTIM_HandleTypeDef hlptim1;
/*!
 * Irq Handler dedicated for Low power Timer reserved for lorawan layer
 *\remark LowPowerTimerLora.timerISR() is used to callback the timer Interupt Service Routine of the current obj LoraWanObject 
 * \param [IN]  void
 * \param [OUT] void         
 */
void LPTIM1_IRQHandler ( void ) {
    hlptim1.Instance = LPTIM1;
    HAL_LPTIM_IRQHandler(&hlptim1);
    HAL_LPTIM_TimeOut_Stop(&hlptim1);
    LowPowerTimerLora.timerISR();
}


/**************************************************/
/*        Class LowPowerTimerLoRa                 */
/**************************************************/
/*!
 * Constructor of LowPowerTimerLoRa
 *\remark This timer is dedicated to the LoraWanObject. It CANNOT be used by the application. This timer must be able to wake up the mcu when it expires.
 * \param [IN]  void
 * \param [OUT] void         
 */
 LowPowerTimerLoRa::LowPowerTimerLoRa ( ) {
    Func = DoNothing;
    obj = NULL;
};
/*!
 * LowPowerTimerLoRa Init
 *\remark initializes the dedicated LoRaWAN low power timer object. MCU specific.
 * \param [IN]  void
 * \param [OUT] void         
 */
void LowPowerTimerLoRa::LowPowerTimerLoRaInit ( ) {
    /* Peripheral clock enable */
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LPTIM1;
    PeriphClkInitStruct.Lptim1ClockSelection = RCC_LPTIM1CLKSOURCE_LSE;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
        DEBUG_MSG("PeriphClkInitStruct LPTIM failed with LSE\n");
    }
    __HAL_RCC_LPTIM1_CLK_ENABLE();
    hlptim1.Instance                 = LPTIM1;
    hlptim1.Init.Clock.Source        = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
    hlptim1.Init.Clock.Prescaler     = LPTIM_PRESCALER_DIV16;
    hlptim1.Init.Trigger.Source      = LPTIM_TRIGSOURCE_SOFTWARE;
    hlptim1.Init.Trigger.ActiveEdge  = LPTIM_ACTIVEEDGE_RISING;
    hlptim1.Init.Trigger.SampleTime  = LPTIM_TRIGSAMPLETIME_DIRECTTRANSITION;
    hlptim1.Init.OutputPolarity      = LPTIM_OUTPUTPOLARITY_HIGH;
    hlptim1.Init.UpdateMode          = LPTIM_UPDATE_IMMEDIATE;
    hlptim1.Init.CounterSource = LPTIM_COUNTERSOURCE_INTERNAL;
    HAL_LPTIM_Init(&hlptim1) ;
    NVIC_ClearPendingIRQ(LPTIM1_IRQn);
    NVIC_DisableIRQ(LPTIM1_IRQn);
    NVIC_SetVector(LPTIM1_IRQn, (uint32_t)LPTIM1_IRQHandler);
    NVIC_EnableIRQ(LPTIM1_IRQn);
    Func = DoNothing;
    obj = NULL;
};
/*!
 * LowPowerTimerLoRa AttachMsecond
 *
 * \param void (* _Func) (void *) a static method member of the current Obj
 * \param *_obj a pointer to the current objet
 * \param int delay in ms delay should be between 1ms and 16s.
 * \param [OUT] void         
 * \remark the code  Func =  _Func ; and obj  = _obj; isn't mcu dependent
 * \remark starts the LoRaWAN dedicated timer and attaches the IRQ to the handling Interupt SErvice Routine in the LoRaWAN object.
 */
void LowPowerTimerLoRa::StartTimerMsecond ( void (* _Func) (void *) , void * _obj, int delay){
    Func =  _Func ;
    obj  = _obj;
    int DelayMs2tick = delay * 2 + ( ( 6 * delay ) >> 7);
    HAL_LPTIM_TimeOut_Start_IT(&hlptim1, 65535, DelayMs2tick); // MCU specific
};

/*******************************/
/*          Watch Dog API      */
/*******************************/
static IWDG_HandleTypeDef Iwdg;
/*!
 * Watch Dog Init And start with a period befor ereset set to 32 seconds
*/
void WatchDogStart ( void ) {
    Iwdg.Instance = IWDG;
    Iwdg.Init.Prescaler =  IWDG_PRESCALER_256;
    Iwdg.Init.Window = IWDG_WINDOW_DISABLE;
    Iwdg.Init.Reload = 0xFFF;
    HAL_IWDG_Init(&Iwdg);
}
/*!
 * Watch Dog Release
*/
void WatchDogRelease ( void ) {
    HAL_IWDG_Refresh(&Iwdg);
}
