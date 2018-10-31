/*

  __  __ _       _                                 
 |  \/  (_)     (_)                                
 | \  / |_ _ __  _ _ __ ___   ___  _   _ ___  ___  
 | |\/| | | '_ \| | '_ ` _ \ / _ \| | | / __|/ _ \
 | |  | | | | | | | | | | | | (_) | |_| \__ \  __/ 
 |_|  |_|_|_| |_|_|_| |_| |_|\___/ \__,_|___/\___| 
                                                   
                                                   
Description       : STM32L MCU .   
                    Example of MCU class implementation based on stm32L4 + mbed library
License           : Revised BSD License, see LICENSE.TXT file include in the project

Maintainer        : Fabien Holin (SEMTECH)
*/
#ifndef McuSTM32L0_H
#define McuSTM32L0_H

#include "stdint.h"
#include "stm32l0xx_hal.h"
#include "stm32l0xx_ll_spi.h"
#include "time.h"
#include "Uart.h"
#include "stdint.h"



/* Definition for USARTx Pins */
#define USARTx_TX_PIN                    GPIO_PIN_2
#define USARTx_TX_GPIO_PORT              GPIOA  
#define USARTx_TX_AF                     GPIO_AF4_USART1
#define USARTx_RX_PIN                    GPIO_PIN_3
#define USARTx_RX_GPIO_PORT              GPIOA 
#define USARTx_RX_AF                     GPIO_AF4_USART1



static IWDG_HandleTypeDef Iwdg;
 extern "C" {
     void EXTI4_15_IRQHandler(void);
       void EXTI2_3_IRQHandler(void);
     void LPTIM1_IRQHandler ( void );
}



typedef enum {
    PA_0  = 0x00,
    PA_1  = 0x01,
    PA_2  = 0x02,
    PA_3  = 0x03,
    PA_4  = 0x04,
    PA_5  = 0x05,
    PA_6  = 0x06,
    PA_7  = 0x07,
    PA_8  = 0x08,
    PA_9  = 0x09,
    PA_10 = 0x0A,
    PA_11 = 0x0B,
    PA_12 = 0x0C,
    PA_13 = 0x0D,
    PA_14 = 0x0E,
    PA_15 = 0x0F,

    PB_0  = 0x10,
    PB_1  = 0x11,
    PB_2  = 0x12,
    PB_3  = 0x13,
    PB_4  = 0x14,
    PB_5  = 0x15,
    PB_6  = 0x16,
    PB_7  = 0x17,
    PB_8  = 0x18,
    PB_9  = 0x19,
    PB_10 = 0x1A,
    PB_11 = 0x1B,
    PB_12 = 0x1C,
    PB_13 = 0x1D,
    PB_14 = 0x1E,
    PB_15 = 0x1F,

    PC_0  = 0x20,
    PC_1  = 0x21,
    PC_2  = 0x22,
    PC_3  = 0x23,
    PC_4  = 0x24,
    PC_5  = 0x25,
    PC_6  = 0x26,
    PC_7  = 0x27,
    PC_8  = 0x28,
    PC_9  = 0x29,
    PC_10 = 0x2A,
    PC_11 = 0x2B,
    PC_12 = 0x2C,
    PC_13 = 0x2D,
    PC_14 = 0x2E,
    PC_15 = 0x2F,

    PD_2  = 0x32,

    PH_0  = 0x70,
    PH_1  = 0x71,

    // ADC internal channels
    ADC_TEMP = 0xF0,
    ADC_VREF = 0xF1,
    ADC_VBAT = 0xF2,

    // Arduino connector namings
    A0          = PA_0,
    A1          = PA_1,
    A2          = PA_4,
    A3          = PB_0,
    A4          = PC_1,
    A5          = PC_0,
    D0          = PA_3,
    D1          = PA_2,
    D2          = PA_10,
    D3          = PB_3,
    D4          = PB_5,
    D5          = PB_4,
    D6          = PB_10,
    D7          = PA_8,
    D8          = PA_9,
    D9          = PC_7,
    D10         = PB_6,
    D11         = PA_7,
    D12         = PA_6,
    D13         = PA_5,
    D14         = PB_9,
    D15         = PB_8,

    // Generic signals namings
    LED1        = PA_5,
    LED2        = PA_5,
    LED3        = PA_5,
    LED4        = PA_5,
    USER_BUTTON = PC_13,
    // Standardized button names
    BUTTON1 = USER_BUTTON,
    SERIAL_TX   = PA_2,
    SERIAL_RX   = PA_3,

    I2C_SCL     = PB_8,
    I2C_SDA     = PB_9,
    SPI_MOSI    = PA_7,
    SPI_MISO    = PA_6,
    SPI_SCK     = PA_5,
    SPI_CS      = PB_6,
    PWM_OUT     = PB_3,

    //USB pins

    // Not connected
    NC = (int)0xFFFFFFFF
} PinName;
/********************************************************************/
/*                        Gpio Handler  functions                   */
/********************************************************************/

static RTC_HandleTypeDef RtcHandle;

class McuSTM32L0 {
public :    
    McuSTM32L0 ( PinName mosi, PinName miso, PinName sclk ) {
        McuMosi = mosi;   // don't modify
        McuMiso = miso;   // don't modify
        McuSclk = sclk;   // don't modify
    }     ;
    ~McuSTM32L0 ( ){};
    void InitMcu ( void );

/******************************************************************************/
/*                                Mcu Spi Api                                 */
/******************************************************************************/
    /** Create a SPI master connected to the specified pins
    *
    *  @param mosi SPI Master Out, Slave In pin
    *  @param miso SPI Master In, Slave Out pin
    *  @param sclk SPI Clock pin
    */
    void InitSpi ( ) {
        SPI_HandleTypeDef hspi1;
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
        GPIO_InitStruct.Alternate = GPIO_PIN_5;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        hspi1.Instance = SPI1;
        hspi1.Init.Mode = SPI_MODE_MASTER;
        hspi1.Init.Direction = SPI_DIRECTION_2LINES;
        hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
        hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
        hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
        hspi1.Init.NSS = SPI_NSS_SOFT;
        hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
        hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
        hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
        hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
        hspi1.Init.CRCPolynomial = 7;
        //hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
        //hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
        __HAL_SPI_DISABLE(&hspi1);
        HAL_SPI_Init(&hspi1);
        __HAL_SPI_ENABLE(&hspi1);
   };
    /** Write to the SPI Slave and return the response
    *
    *  @param value Data to be sent to the SPI slave
    *
    *  @returns
    *    Response from the SPI slave
    */
    uint8_t SpiWrite(int value) {
        uint8_t rxData = 0;
        while( LL_SPI_IsActiveFlag_TXE ( SPI1 ) == 0  ){};
        LL_SPI_TransmitData8 (SPI1, uint8_t (value&0xFF));
        while( LL_SPI_IsActiveFlag_RXNE ( SPI1 ) == 0 ){};
        rxData =  LL_SPI_ReceiveData8( SPI1 );
        return (rxData);
    };
    
    /** Configure the data transmission format
    *
    *  @param bits Number of bits per SPI frame (4 - 16)
    *  @param mode Clock polarity and phase mode (0 - 3)
    *
    * @code
    * mode | POL PHA
    * -----+--------
    *   0  |  0   0
    *   1  |  0   1
    *   2  |  1   0
    *   3  |  1   1
    * @endcode
    */
    void Spiformat(int bits, int mode = 0) { } ;
        
    /** Set the spi bus clock frequency
    *
    *  @param hz SCLK frequency in hz (default = 1MHz)
    */
    void SetSpiFrequency(int hz = 1000000) { } ;
    PinName McuMosi;
    PinName McuMiso;
    PinName McuSclk;
    
    
/******************************************************************************/
/*                                Mcu RTC Api                                 */
/******************************************************************************/
    /*!
    * RtcInit Function
    * \remark must be called before any call to initiliaze the timers
    * \param [IN]   void
    * \param [OUT]  void       
    */
    void     RtcInit            ( void ){ 

        __HAL_RCC_RTC_ENABLE();
        RtcHandle.Init.HourFormat     = RTC_HOURFORMAT_24;
        RtcHandle.Init.AsynchPrediv   = 127;
        RtcHandle.Init.SynchPrediv    = 255;//(rtc_freq / 128 )-1 ;
        RtcHandle.Init.OutPut         = RTC_OUTPUT_DISABLE;
        RtcHandle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
        RtcHandle.Init.OutPutType     = RTC_OUTPUT_TYPE_OPENDRAIN;
        if (HAL_RTC_Init(&RtcHandle) != HAL_OK) {
            MMprint("RTC error: RTC initialization failed.");
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
    };
    
    /*!
    * RtcGetTimeSecond : return the Current Rtc time in Second 
    * \remark is used for :
    * \remark scheduling autonomous retransmissions (for exemple NbTrans) , transmitting MAC answers , basically any delay without accurate time constraints
    * \remark also used to measure the time spent inside the LoRaWAN process for the integrated failsafe
    * \param [IN]   void
    * \param [OUT]  uint32_t RTC time in Second       
    */
    uint32_t RtcGetTimeSecond       ( void ) {
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
    };
   /*!
    * RtcGetTimeMs : return the Current Rtc time in Ms 
    * \remark is used to timestamp radio events (end of TX), will also be used for future classB
    * \remark this function may be used by the application.
    * \param [IN]   void
    * \param [OUT]  uint32_t Current RTC time in ms wraps every 49 days       
    */
    uint32_t RtcGetTimeMs  ( void ) {
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
    };    
    
    
/******************************************************************************/
/*                                Mcu Flash Api                               */
/******************************************************************************/
     /** RestoreContext data from a flash device.  
     * 
     *  This method invokes memcpy - reads number of bytes from the address 
     * 
     *  @param buffer Buffer to write to 
     *  @param addr   Flash address to begin reading from 
     *  @param size   Size to read in bytes 
     *  @return       0 on success, negative error code on failure 
     */ 
    int RestoreContext(uint8_t *buffer, uint32_t addr, uint32_t size){      
        /* have to be implemented by mcu providers
    the following code propose a lite implementation without any error cases*/
    int i;
    for (i=0;i<size;i++){
        buffer[i]= *((uint8_t*)(addr)+i);
    }
     return 0;
    };

 
    /** StoreContext data to flash
     *  To be safer this function have to implement a read/check data sequence after programation 
     *  
     * 
     *  @param buffer Buffer of data to be written 
     *  @param addr   Flash Address to begin writing to,
     *  @param size   Size to write in word 64 bits,
     *  @return       0 on success, negative error code on failure 
     */ 
    int StoreContext(const void *buffer, uint32_t addr, uint32_t size){  
        HAL_StatusTypeDef res = HAL_OK; 
        HAL_FLASH_Unlock(); 

//        for (; size >= 4; size -= 4, addr += 4) { 
//            res = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, *(uint32_t const*)buffer); 
//            buffer = (uint32_t const*)buffer + 1; 
//            if (res != HAL_OK) 
//            goto done; 
//        } 
        for (int i = 0 ; i < (8 * size ) ; i++){
         *((uint8_t*)(addr)+i) = *((uint8_t *)(buffer) + i);
         __WFI();
    }
        
            HAL_FLASH_Lock(); 
            return res == HAL_OK ? 0 : -1; 
    }; 
        
/******************************************************************************/
/*                             Mcu WatchDog Api                               */
/******************************************************************************/
    /* A function to init and start the Watchdog 
    * \remark The expired period = WATCH_DOG_PERIOD_RELEASE seconds
    * \param [IN]   void  
    * \param [OUT]  void       
    */
    void WatchDogStart ( void ){
    Iwdg.Instance = IWDG;
    Iwdg.Init.Prescaler =  IWDG_PRESCALER_256;
    Iwdg.Init.Window = IWDG_WINDOW_DISABLE;
    Iwdg.Init.Reload = 0xFFF;
    HAL_IWDG_Init(&Iwdg);
} ;

    /* A function to release the Watchdog 
    * \remark Application have to call this function periodically (with a period <WATCH_DOG_PERIOD_RELEASE)
    *         If not , the mcu will reset.
    * \param [IN]   void  
    * \param [OUT]  void       
    */
    void WatchDogRelease ( void ) {
    HAL_IWDG_Refresh(&Iwdg);
};
    
/******************************************************************************/
/*                      Mcu Low Power Timer Api                               */
/******************************************************************************/
/*!
    * LowPowerTimerLoRa Init
    *\remark initializes the dedicated LoRaWAN low power timer object. MCU specific.
    * \param [IN]  void
    * \param [OUT] void         
    */
    void LowPowerTimerLoRaInit ( void );
    /*!
    * LowPowerTimerLoRa AttachMsecond
    *
    * \param void (* _Func) (void *) a static method member of the current Obj
    * \param *_obj a pointer to the current objet
    * \param int delay in ms delay should be between 1ms and 16s.
    * \param [OUT] void         
    * \remark the code  Func =  _Func ; and obj  = _obj; isn't mcu dependent , and could be keep as already implemented
    * \remark starts the LoRaWAN dedicated timer and attaches the IRQ to the handling Interupt Service Routine in the LoRaWAN object.
    */
    void StartTimerMsecond     ( void (* _Func) (void *) , void * _obj, int delay) ;
        
    /*!
    *  timerISR
    * \remark    Do Not Modify 
    */
    void timerISR              ( void ) { Func(obj); };
/******************************************************************************/
/*                           Mcu Gpio Api                                     */
/******************************************************************************/
    void SetValueDigitalOutPin ( PinName pin, int Value ) {
        uint16_t pintmp ;
        if ( pin < 15 ) {
            pintmp = (uint16_t) pin;
            HAL_GPIO_WritePin(GPIOA, ( 1 << pintmp ), (GPIO_PinState) Value);
        } else  {
            pintmp = (uint16_t)pin-16;
            HAL_GPIO_WritePin(GPIOB, ( 1 << pintmp ), (GPIO_PinState) Value);
        }
    };
    int  GetValueDigitalInPin  ( PinName pin ){
        uint16_t pintmp;
        if ( pin < 15 ) {
            pintmp = (uint16_t) pin;
            return ( (int) HAL_GPIO_ReadPin(GPIOA, ( 1 << pintmp ) ) );
        } else  {
            pintmp = (uint16_t) pin-16;
            return ( (int) HAL_GPIO_ReadPin(GPIOB, ( 1 << pintmp ) ) );
        }
    };
    void Init_Irq ( PinName pin) ;
    void AttachInterruptIn     (  void (* _Funcext) (void *) , void * _objext)  {
        Funcext =  _Funcext ;
        objext  = _objext;
        userIt  = 0 ; 
    };
    void AttachInterruptIn     (  void (* _Funcext) ( void ) ) { 
        _UserFuncext = _Funcext; userIt = 1 ; 
    };
    void DetachInterruptIn     (  void (* _Funcext) ( void ) ) { userIt = 0 ; };
    /*!
    *  ExtISR
    * \remark    Do Not Modify 
    */
    void ExtISR                ( void ) { 
        if (userIt == 0 ) { 
        Funcext(objext); 
        } else {
        _UserFuncext ();
        }; 
    };
    
/******************************************************************************/
/*                           Mcu Wait Api                                     */
/******************************************************************************/
    void mwait_ms (int delayms) {
              HAL_Delay(delayms);
    };
    void mwait (int delays) {
              HAL_Delay(1000*delays);
    };
    
/******************************************************************************/
/*                           Mcu Uart Api                                     */
/******************************************************************************/
    void UartInit ( void );
    void MMprint( const char *fmt, ...);
private :
    /*!
    *  Low power timer
    * \remark    Do Not Modify 
    */
    static void DoNothing (void *) { };
    void (* Func) (void *);
    void * obj;
    void (* Funcext) (void *);
    void * objext;
    void (* _UserFuncext) ( void );
    int userIt;
    
};



#endif
