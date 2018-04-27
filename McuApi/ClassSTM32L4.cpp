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
#include "ClassSTM32L4.h"
#include "stdint.h"
#include "ApiMcu.h"
#include "stm32l4xx_hal.h"
#define WATCH_DOG_PERIOD_RELEASE 30 // this period have to be lower than the Watch Dog period of 32 seconds








/********************************************************************/
/*                           Flash local functions                  */
/********************************************************************/
#define DATA_EEPROM_BASE       ( ( uint32_t )0x8000000 )              /*!< DATA_EEPROM base address in the alias region */
#define DATA_EEPROM_END        ( ( uint32_t )DATA_EEPROM_BASE + 4096 ) /*!< DATA EEPROM end address in the alias region */

void FlashPageErase( uint32_t page, uint32_t banks )
{
    // Check the parameters
    assert_param( IS_FLASH_PAGE( page ) );
    assert_param( IS_FLASH_BANK_EXCLUSIVE( banks ) );

    if( ( banks & FLASH_BANK_1 ) != RESET )
    {
        CLEAR_BIT( FLASH->CR, FLASH_CR_BKER );
    }
    else
    {
        SET_BIT( FLASH->CR, FLASH_CR_BKER );
    }

    // Proceed to erase the page
    MODIFY_REG( FLASH->CR, FLASH_CR_PNB, ( page << 3 ) );
    SET_BIT( FLASH->CR, FLASH_CR_PER );
    SET_BIT( FLASH->CR, FLASH_CR_STRT );
}

uint8_t EepromMcuWriteBuffer( uint32_t addr, uint8_t *buffer, uint16_t size )
{
    uint64_t *flash = ( uint64_t* )buffer;
    
   uint32_t Findpage = (addr - 0x8000000 )>>11;
   uint32_t NumberOfPage = (size >> 11)+1; 
   HAL_FLASH_Unlock( );
    for (uint32_t i = 0 ; i < NumberOfPage; i ++){
    FlashPageErase( Findpage + i, 1 );
    }
    
    WRITE_REG( FLASH->CR, 0x40000000 );

    for( uint32_t i = 0; i < size; i++ )
    {
        HAL_FLASH_Program( FLASH_TYPEPROGRAM_DOUBLEWORD, addr + ( 8 * i ), flash[i] );
    }

    HAL_FLASH_Lock( );

    return SUCCESS;
}

uint8_t EepromMcuReadBuffer( uint32_t addr, uint8_t *buffer, uint16_t size )
{
    assert_param( buffer != NULL );

    //assert_param( addr >= DATA_EEPROM_BASE );
    assert_param( buffer != NULL );
    assert_param( size < ( DATA_EEPROM_END - DATA_EEPROM_BASE ) );
for( uint32_t i = 0; i < size; i++ )
    {
     buffer[i]= *((( uint8_t* )addr)+i);
    }     
    return SUCCESS;
}

void EepromMcuSetDeviceAddr( uint8_t addr )
{
    assert_param( FAIL );
}

uint8_t EepromMcuGetDeviceAddr( void )
{
    assert_param( FAIL );
    return 0;
}


/********************************************************************/
/*                         Wake Up local functions                  */
/********************************************************************/
static RTC_HandleTypeDef RtcHandle;


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



/********************************************************************/
/*                 Low Power Timer local functions                  */
/********************************************************************/
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
    mcu.timerISR();
}



/********************************************************************/
/*                        Gpio Handler  functions                   */
/********************************************************************/

void  IrqHandlerRadio ( void ){
    int pintmp = TX_RX_IT;
    if ( pintmp < 15 ) {
        HAL_GPIO_EXTI_IRQHandler( (1 << pintmp) );
    } else if ( pintmp < 31 ) {
        HAL_GPIO_EXTI_IRQHandler( (1 << ( pintmp - 16 )) );
    } else if ( pintmp < 47 ) {
        HAL_GPIO_EXTI_IRQHandler( (1 << ( pintmp - 32 )) );
    } else if ( pintmp < 63 ) {
        HAL_GPIO_EXTI_IRQHandler( (1 << ( pintmp - 48 )) );
    } else if ( pintmp < 79 ) {
        HAL_GPIO_EXTI_IRQHandler( (1 << ( pintmp - 64 )) );
    } else if ( pintmp < 95 ) {
        HAL_GPIO_EXTI_IRQHandler( (1 << ( pintmp - 80 )) );
    } else if ( pintmp < 111 ) {
        HAL_GPIO_EXTI_IRQHandler( (1 << ( pintmp - 96 )) );
    } 
    pintmp = RX_TIMEOUT_IT;
    if ( pintmp < 15 ) {
        HAL_GPIO_EXTI_IRQHandler( (1 << pintmp) );
    } else if ( pintmp < 31 ) {
        HAL_GPIO_EXTI_IRQHandler( (1 << ( pintmp - 16 )) );
    } else if ( pintmp < 47 ) {
        HAL_GPIO_EXTI_IRQHandler( (1 << ( pintmp - 32 )) );
    } else if ( pintmp < 63 ) {
        HAL_GPIO_EXTI_IRQHandler( (1 << ( pintmp - 48 )) );
    } else if ( pintmp < 79 ) {
        HAL_GPIO_EXTI_IRQHandler( (1 << ( pintmp - 64 )) );
    } else if ( pintmp < 95 ) {
        HAL_GPIO_EXTI_IRQHandler( (1 << ( pintmp - 80 )) );
    } else if ( pintmp < 111 ) {
        HAL_GPIO_EXTI_IRQHandler( (1 << ( pintmp - 96 )) );
    } 
    mcu.ExtISR();
}

    

/*************************************************************/
/*           Mcu Object Definition Constructor               */
/*************************************************************/
McuSTM32L4::McuSTM32L4(PinName mosi, PinName miso, PinName sclk )  {
    Func = DoNothing; // don't modify
    obj = NULL;       // don't modify
    McuMosi = mosi;   // don't modify
    McuMiso = miso;   // don't modify
    McuSclk = sclk;   // don't modify
}     
McuSTM32L4::~McuSTM32L4(){
      // to be completed by mcu providers
} 

/*******************************************/
/*                  Mcu Init               */
/*******************************************/
void McuSTM32L4::InitMcu( void ) {
    // system clk Done with mbed to be completed by mcu providers if mbed is removed

    WakeUpInit ( );
    InitSpi ( );
    NVIC_ClearPendingIRQ(RTC_WKUP_IRQn);
    NVIC_DisableIRQ(RTC_WKUP_IRQn);
    NVIC_SetVector(RTC_WKUP_IRQn, (uint32_t)RTC_IRQHandlerWakeUp);
    NVIC_EnableIRQ(RTC_WKUP_IRQn);

}

void  McuSTM32L4::Init_Irq ( PinName pin) {
    IRQn_Type IrqNum; 
    int pintmp;
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    if ( pin < 15 ) {
        pintmp = pin;
        GPIO_InitStruct.Pin = (1 << pintmp);
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    } else if ( pin < 31 ) {
        pintmp = pin-16;
        GPIO_InitStruct.Pin = (1 << pintmp);
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    } else if ( pin < 47 ) {
        pintmp = pin-32;
        GPIO_InitStruct.Pin = (1 << pintmp);
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    } else if ( pin < 63 ) {
        pintmp = pin-48;
        GPIO_InitStruct.Pin = (1 << pintmp);
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    } else if ( pin < 79 ) {
        pintmp = pin-64;
        GPIO_InitStruct.Pin = (1 << pintmp);
        HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
    } else if ( pin < 95 ) {
        pintmp = pin-80;
        GPIO_InitStruct.Pin = (1 << pintmp);
        HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
    } else if ( pin < 111 ) {
        pintmp = pin-96;
        GPIO_InitStruct.Pin = (1 << pintmp);
        HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
    }
    switch ( pintmp ) {
        case 0 : 
            IrqNum = EXTI0_IRQn;
            break;
        case 1 : 
            IrqNum = EXTI1_IRQn;
            break;
        case 2 : 
            IrqNum = EXTI2_IRQn;
            break;
        case 3 : 
            IrqNum = EXTI3_IRQn;
            break;
        case 4 : 
            IrqNum = EXTI4_IRQn;
            break;
        case 5 :
        case 6 :
        case 7 :
        case 8 :
        case 9 :            
            IrqNum = EXTI9_5_IRQn;
            break;
        case 10 :
        case 11 :
        case 12 :
        case 13 :
        case 14 :
        case 15 :            
            IrqNum = EXTI15_10_IRQn;
            break;
        default : 
            IrqNum = EXTI15_10_IRQn;
            break;
    }
    HAL_NVIC_SetPriority(IrqNum, 0, 0);
    NVIC_SetVector(IrqNum, (uint32_t)IrqHandlerRadio);
    HAL_NVIC_EnableIRQ(IrqNum);
}    
/******************************************************************************/
/*                                Mcu Spi Api                                 */
/******************************************************************************/
    /** Create a SPI master connected to the specified pins
    *
    *  @param mosi SPI Master Out, Slave In pin
    *  @param miso SPI Master In, Slave Out pin
    *  @param sclk SPI Clock pin
    */
SPI_HandleTypeDef hspi1;
void McuSTM32L4::InitSpi ( ){
    SPI pspi( McuMosi, McuMiso, McuSclk );
    
/***************************************************************************/    
/* following code give an example to how remove spi done with mbed library */
/* In this example Pin configuration is fixed                              */
/***************************************************************************/ 
#if 0
    GPIO_InitTypeDef GPIO_InitStruct;
    __HAL_RCC_SPI1_CLK_ENABLE();
    HAL_NVIC_SetPriority(SPI1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(SPI1_IRQn);
    /**SPI1 GPIO Configuration    
    PA5     ------> SPI1_SCK
    PA6     ------> SPI1_MISO
    PA7     ------> SPI1_MOSI 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial = 7;
    //hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
    //hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
    __HAL_SPI_DISABLE(&hspi1);
    HAL_SPI_Init(&hspi1);
    __HAL_SPI_ENABLE(&hspi1);
#endif

}
    /** Write to the SPI Slave and return the response
    *
    *  @param value Data to be sent to the SPI slave
    *
    *  @returns
    *    Response from the SPI slave
    */
uint8_t McuSTM32L4::SpiWrite(int value){
    SPI pspi( McuMosi, McuMiso, McuSclk );
    return ( pspi.write( value ) );
/***************************************************************************/    
/* following code give an example to how remove spi done with mbed library */
/* In this example Pin configuration is static                             */
/***************************************************************************/ 
# if 0
    uint8_t rxData = 0;
    while( LL_SPI_IsActiveFlag_TXE ( SPI1 ) == 0  ){};
    LL_SPI_TransmitData8 (SPI1, uint8_t (value&0xFF));
    while( LL_SPI_IsActiveFlag_RXNE ( SPI1 ) == 0 ){};
    rxData =  LL_SPI_ReceiveData8( SPI1 );
    return (rxData);
#endif
}
/******************************************************************************/
/*                                Mcu Flash Api                               */
/******************************************************************************/
int McuSTM32L4::RestoreContext(uint8_t *buffer, uint32_t addr, uint32_t size){
     /* have to be implemented by mcu providers
    the following code propose a lite implementation without any error cases*/
    uint16_t sizet = size & 0xFFFF;
    EepromMcuReadBuffer( addr, buffer, sizet );
    return ( 0 ); 
}


int McuSTM32L4::StoreContext(const void *buffer, uint32_t addr, uint32_t size){
    /* have to be implemented by mcu providers
    the following code propose a lite implementation without any error cases
    this section have to be very robust, have to support failure mode such as  power off during flash programmation    
    This basic implementation suppose that the addr is 4 Bytes aligned and suppose also that the size can be divide by 4.
    */
    uint16_t sizet = size & 0xFFFF;    
    EepromMcuWriteBuffer( addr,  (uint8_t*) buffer, sizet );    
    return ( 0 ); 
} 
   


/******************************************************************************/
/*                                Mcu RTC Api                                 */
/******************************************************************************/

void McuSTM32L4::RtcInit (void)
{
    uint32_t rtc_freq = 0;
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

uint32_t McuSTM32L4::RtcGetTimeMs( void )
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

uint32_t McuSTM32L4::RtcGetTimeSecond( void )
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

/******************************************************************************/
/*                                Mcu Sleep Api                               */
/******************************************************************************/
void McuSTM32L4::GotoSleepSecond (int duration ) {
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

void McuSTM32L4::GotoSleepMSecond (int duration ) {
#if LOW_POWER_MODE == 1
    WakeUpAlarmMSecond ( duration );
    sleep();
# else
    wait_ms ( duration ) ;
#endif
}


/******************************************************************************/
/*                             Mcu WatchDog Api                               */
/******************************************************************************/
static IWDG_HandleTypeDef Iwdg;

/*!
 * Watch Dog Init And start with a period befor ereset set to 32 seconds
*/
void McuSTM32L4::WatchDogStart ( void ) {
    Iwdg.Instance = IWDG;
    Iwdg.Init.Prescaler =  IWDG_PRESCALER_256;
    Iwdg.Init.Window = IWDG_WINDOW_DISABLE;
    Iwdg.Init.Reload = 0xFFF;
    HAL_IWDG_Init(&Iwdg);
}
/*!
 * Watch Dog Release
*/
void McuSTM32L4::WatchDogRelease ( void ) {
    HAL_IWDG_Refresh(&Iwdg);
};
/******************************************************************************/
/*                             Mcu LOwPower timer Api                         */
/******************************************************************************/
void McuSTM32L4::LowPowerTimerLoRaInit ( ) {
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
    //Initialize delay Systick timer for wait function
    //TM_DELAY_Init();
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
void McuSTM32L4::StartTimerMsecond ( void (* _Func) (void *) , void * _obj, int delay){
    Func =  _Func ;
    obj  = _obj;
    int DelayMs2tick = delay * 2 + ( ( 6 * delay ) >> 7);
    HAL_LPTIM_TimeOut_Start_IT(&hlptim1, 65535, DelayMs2tick); // MCU specific
};

/******************************************************************************/
/*                           Mcu Gpio Api                                     */
/******************************************************************************/
void McuSTM32L4::SetValueDigitalOutPin ( PinName Pin, int Value ){
    DigitalOut DigOutMbed ( Pin ) ;
    DigOutMbed = Value;

};
int McuSTM32L4::GetValueDigitalInPin ( PinName Pin ){
    DigitalIn DigInMbed ( Pin );
    return ( DigInMbed );
};

void  McuSTM32L4::AttachInterruptIn       (  void (* _Funcext) (void *) , void * _objext) {
    Funcext =  _Funcext ;
    objext  = _objext;
    userIt  = 0 ; 
};
