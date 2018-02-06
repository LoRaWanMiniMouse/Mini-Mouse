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

static void RTC_IRQHandlerWakeUp (void)
{
    RtcHandle.Instance = RTC;
    HAL_RTCEx_DeactivateWakeUpTimer(&RtcHandle);
    HAL_RTCEx_WakeUpTimerIRQHandler(&RtcHandle);
    __HAL_RTC_WAKEUPTIMER_EXTI_ENABLE_EVENT();
}



void WakeUpInit ( void ) {
      RtcHandle.Instance = RTC;
      NVIC_ClearPendingIRQ(RTC_WKUP_IRQn);
      NVIC_DisableIRQ(RTC_WKUP_IRQn);
      NVIC_SetVector(RTC_WKUP_IRQn, (uint32_t)RTC_IRQHandlerWakeUp);
      NVIC_EnableIRQ(RTC_WKUP_IRQn);
}

void WakeUpAlarmMSecond ( int delay) {
    int DelayMs2tick = delay * 2 + ( ( 6 * delay ) >> 7);
    HAL_RTCEx_SetWakeUpTimer_IT(&RtcHandle, DelayMs2tick, RTC_WAKEUPCLOCK_RTCCLK_DIV16);
}

void WakeUpAlarmSecond ( int delay) {
    HAL_RTCEx_SetWakeUpTimer_IT(&RtcHandle, delay, 4);
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

LPTIM_HandleTypeDef hlptim1;

void LPTIM1_IRQHandler ( void ) {
    hlptim1.Instance = LPTIM1;
    HAL_LPTIM_IRQHandler(&hlptim1);
    HAL_LPTIM_TimeOut_Stop(&hlptim1);
    LowPowerTimerLora.run();
}

 LowPowerTimerLoRa::LowPowerTimerLoRa ( ) {
    Func = DoNothing;
    obj = NULL;
};

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
void LowPowerTimerLoRa::AttachMsecond ( void (* _Func) (void *) , void * _obj, int delay){
    Func =  _Func ;
    obj  = _obj;
    int DelayMs2tick = delay * 2 + ( ( 6 * delay ) >> 7);
    HAL_LPTIM_TimeOut_Start_IT(&hlptim1, 65535, DelayMs2tick);
};
