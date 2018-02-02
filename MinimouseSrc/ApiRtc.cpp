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
#include "ApiRtc.h"
#include "Define.h"
RTC_HandleTypeDef RtcHandle;
static int globalt ;
void (*globalfunc)(void);
AlarmRtc myalarm;

static void RTC_IRQHandlerAlarm(void )
{
    HAL_RTC_AlarmIRQHandler(&RtcHandle);
    myalarm.run ();
}
static void RTC_IRQHandlerWakeUp (void)
{
    RtcHandle.Instance = RTC;
    HAL_RTCEx_DeactivateWakeUpTimer(&RtcHandle);
    myalarm.run ();
    HAL_RTCEx_WakeUpTimerIRQHandler(&RtcHandle);
    __HAL_RTC_WAKEUPTIMER_EXTI_ENABLE_EVENT();
}

void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc){
    HAL_RTC_DeactivateAlarm( hrtc, RTC_ALARM_A );
    DEBUG_MSG("alarm ok \n");
}

 AlarmRtc::AlarmRtc() { 
    mutex = 0;
    Func = DoNothing;
    obj = NULL;
} ; 
void AlarmRtc::AlarmInit ( ) {
      RtcHandle.Instance = RTC;
      NVIC_ClearPendingIRQ(RTC_WKUP_IRQn);
      NVIC_DisableIRQ(RTC_WKUP_IRQn);
      NVIC_SetVector(RTC_WKUP_IRQn, (uint32_t)RTC_IRQHandlerWakeUp);
      NVIC_EnableIRQ(RTC_WKUP_IRQn);
}
/* max 60 second */
void AlarmRtc::AttachMsecond ( void (* _Func) (void *) , void * _obj, int delay) {
        Func =  _Func ;
        obj = _obj;
        int DelayMs2tick = delay * 2 + ( ( 6 * delay ) >> 7);
    if ( mutex == 0 ) {
        mutex = 1;
        HAL_RTCEx_SetWakeUpTimer_IT(&RtcHandle, DelayMs2tick, RTC_WAKEUPCLOCK_RTCCLK_DIV16);
    }
}
void AlarmRtc::AttachMsecond ( int delay) {
    int DelayMs2tick = delay * 2 + ( ( 6 * delay ) >> 7);
    HAL_RTCEx_SetWakeUpTimer_IT(&RtcHandle, DelayMs2tick, RTC_WAKEUPCLOCK_RTCCLK_DIV16);
}

void AlarmRtc::SleepMs (int delay ) {
    if ( mutex == 0 ) {
        Func =  DoNothing ;
        obj = NULL;
        int DelayMs2tick = delay * 2 + ( ( 6 * delay ) >> 7);
        HAL_RTCEx_SetWakeUpTimer_IT(&RtcHandle, DelayMs2tick, RTC_WAKEUPCLOCK_RTCCLK_DIV16);
    }
    sleep();
}
void AlarmRtc::SleepSecond (int delay ) {
    if ( mutex == 0 ) {
        Func =  DoNothing ;
        obj = NULL;
        HAL_RTCEx_SetWakeUpTimer_IT(&RtcHandle, delay, 4);
        }
    deepsleep();
//    HAL_PWREx_EnterSTOP2Mode(PWR_STOPENTRY_WFI);
//        SetSysClock();
}
     
uint32_t RtcGetTimeMs( uint64_t  *longTime64bits )//uint32_t  *Seconds, uint16_t * SubSeconds)
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
    *longTime64bits =  cal_convertBCD_2_Cnt64( &dateStruct, &timeStruct );
    // Convert to timestamp
    time_t t = mktime(&timeinfo);
    return ( ( t * 1000 ) + ( 999 - ( ( timeStruct.SubSeconds *999) / 255 ) ) );  // get time en ms
}

uint32_t RtcGetTimeSecond( void )//uint32_t  *Seconds, uint16_t * SubSeconds)
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

void RtcSetAlarm( void ){ 
    RTC_AlarmTypeDef sAlarm;
    sAlarm.AlarmTime.Hours = 0;
    sAlarm.AlarmTime.Minutes = 1;
    sAlarm.AlarmTime.Seconds = 0x02;//RTC_ByteToBcd2(1)+1;//RTC_ByteToBcd2(next_second);
    sAlarm.AlarmTime.TimeFormat = RTC_HOURFORMAT12_AM;
    sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
    sAlarm.AlarmMask = RTC_ALARMMASK_ALL;//(RTC_ALARMMASK_NONE|RTC_ALARMMASK_DATEWEEKDAY | RTC_ALARMMASK_HOURS       | RTC_ALARMMASK_MINUTES     );// RTC_ALARMMASK_ALL;//
    sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
    sAlarm.AlarmDateWeekDay = 1;
    sAlarm.Alarm = RTC_ALARM_A;
    HAL_RTC_SetAlarm_IT(&RtcHandle, &sAlarm, FORMAT_BCD);

}
void RtcGetAlarm( void ){ 
    RTC_AlarmTypeDef sAlarm;
    HAL_RTC_GetAlarm(&RtcHandle, &sAlarm, RTC_ALARM_A,FORMAT_BCD);
    DEBUG_PRINTF("sAlarm.AlarmTime.Hours = %d\n",sAlarm.AlarmTime.Hours);
    DEBUG_PRINTF("sAlarm.AlarmTime.Minutes = %d\n",sAlarm.AlarmTime.Minutes);
    DEBUG_PRINTF("sAlarm.AlarmTime.Seconds = %d\n",sAlarm.AlarmTime.Seconds);
    DEBUG_PRINTF("sAlarm.AlarmTime.AlarmDateWeekDaySel = %d\n",sAlarm.AlarmDateWeekDaySel);
    DEBUG_PRINTF("sAlarm.AlarmTime.AlarmMask = %x\n",sAlarm.AlarmMask);
}



void my_rtc_init (void)
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
    struct tm *timeinfo = localtime(0);
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

void wait_s ( int t )
{
    wait(t);
}
