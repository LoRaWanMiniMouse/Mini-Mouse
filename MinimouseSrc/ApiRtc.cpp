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
RTC_HandleTypeDef RtcHandle;
static int rtc_inited = 0;
static void RTC_IRQHandler (void)
{
    HAL_RTC_AlarmIRQHandler(&RtcHandle);
}
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc){
    HAL_RTC_DeactivateAlarm( hrtc, RTC_ALARM_A );
    pcf.printf("alarm ok \n");
}


uint32_t RtcGetTimeMs( uint64_t  *longTime64bits )//uint32_t  *Seconds, uint16_t * SubSeconds)
{
    RTC_DateTypeDef dateStruct;
    RTC_TimeTypeDef timeStruct;
    struct tm timeinfo;
    RtcHandle.Instance = RTC;
    uint32_t  Seconds;
    uint16_t SubSeconds;
    // Read actual date and time
    // Warning: the time must be read first!
    HAL_RTC_GetTime(&RtcHandle, &timeStruct, FORMAT_BIN);
    HAL_RTC_GetDate(&RtcHandle, &dateStruct, FORMAT_BIN);

    // Setup a tm structure based on the RTC
    timeinfo.tm_wday = dateStruct.WeekDay;
    timeinfo.tm_mon  = dateStruct.Month ;
    timeinfo.tm_mday = dateStruct.Date;
    timeinfo.tm_year = dateStruct.Year ;
    timeinfo.tm_hour = timeStruct.Hours;
    timeinfo.tm_min  = timeStruct.Minutes;
    timeinfo.tm_sec  = timeStruct.Seconds;
    SubSeconds      = ( 255 - ( timeStruct.SubSeconds ) ) << 7; 
    *longTime64bits =  cal_convertBCD_2_Cnt64( &dateStruct, &timeStruct );
    // Convert to timestamp
    time_t t = mktime(&timeinfo);
    Seconds = t;
    return ( ( t * 1000 ) + ( 999 - ( ( timeStruct.SubSeconds *999) / 255 ) ) );  // get time en ms
}

uint32_t RtcGetTimeSecond( void )//uint32_t  *Seconds, uint16_t * SubSeconds)
{
    RTC_DateTypeDef dateStruct;
    RTC_TimeTypeDef timeStruct;
    struct tm timeinfo;
    RtcHandle.Instance = RTC;

    // Read actual date and time
    // Warning: the time must be read first!
    HAL_RTC_GetTime(&RtcHandle, &timeStruct, FORMAT_BIN);
    HAL_RTC_GetDate(&RtcHandle, &dateStruct, FORMAT_BIN);

    // Setup a tm structure based on the RTC
    timeinfo.tm_wday = dateStruct.WeekDay;
    timeinfo.tm_mon  = dateStruct.Month ;
    timeinfo.tm_mday = dateStruct.Date;
    timeinfo.tm_year = dateStruct.Year ;
    timeinfo.tm_hour = timeStruct.Hours;
    timeinfo.tm_min  = timeStruct.Minutes;
    timeinfo.tm_sec  = timeStruct.Seconds;
    
    // Convert to timestamp
    time_t t = mktime(&timeinfo);
    return ( t );
}

void RtcSetAlarm( void ){ 
    RTC_AlarmTypeDef sAlarm;
    sAlarm.AlarmTime.Hours = 0;
    sAlarm.AlarmTime.Minutes = 0;
    sAlarm.AlarmTime.Seconds = RTC_ByteToBcd2(1);//RTC_ByteToBcd2(next_second);
    sAlarm.AlarmTime.TimeFormat = RTC_HOURFORMAT12_AM;
    sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
    sAlarm.AlarmMask = RTC_ALARMMASK_ALL;//(RTC_ALARMMASK_NONE|RTC_ALARMMASK_DATEWEEKDAY | RTC_ALARMMASK_HOURS       | RTC_ALARMMASK_MINUTES     );
    sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
    sAlarm.AlarmDateWeekDay = 1;
    sAlarm.Alarm = RTC_ALARM_A;
    HAL_RTC_SetAlarm_IT(&RtcHandle, &sAlarm, FORMAT_BCD);
}
void RtcGetAlarm( void ){ 
    RTC_AlarmTypeDef sAlarm;
    HAL_RTC_GetAlarm(&RtcHandle, &sAlarm, RTC_ALARM_A,FORMAT_BCD);
    pcf.printf("sAlarm.AlarmTime.Hours = %d\n",sAlarm.AlarmTime.Hours);
    pcf.printf("sAlarm.AlarmTime.Minutes = %d\n",sAlarm.AlarmTime.Minutes);
    pcf.printf("sAlarm.AlarmTime.Seconds = %d\n",sAlarm.AlarmTime.Seconds);
    pcf.printf("sAlarm.AlarmTime.AlarmDateWeekDaySel = %d\n",sAlarm.AlarmDateWeekDaySel);
    pcf.printf("sAlarm.AlarmTime.AlarmMask = %x\n",sAlarm.AlarmMask);
}



void my_rtc_init (void)
{
    int rtc_inited = 0;
    uint32_t rtc_freq = 0;
    RTC_HandleTypeDef RtcHandle;
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
        pcf.printf("PeriphClkInitStruct RTC failed with LSE\n");
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
        pcf.printf("RTC error: RTC initialization failed.");
    }
    NVIC_ClearPendingIRQ(RTC_WKUP_IRQn);
    NVIC_DisableIRQ(RTC_WKUP_IRQn);
    NVIC_SetVector(RTC_WKUP_IRQn, (uint32_t)RTC_IRQHandler);
    NVIC_EnableIRQ(RTC_WKUP_IRQn);

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
