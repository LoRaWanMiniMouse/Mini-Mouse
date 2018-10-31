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
#if 0
#include "ClassSTM32L0.h"
#include "stdint.h"
#include "ApiMcu.h"
#include "stm32l0xx_hal.h"
#define WATCH_DOG_PERIOD_RELEASE 30 // this period have to be lower than the Watch Dog period of 32 seconds
#if DEBUG_TRACE == 1
    #include <stdarg.h>
    #include <string.h>
#endif
#include "main.h"
UART_HandleTypeDef UartHandle;


/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = MSI
  *            SYSCLK(Hz)                     = 2000000
  *            HCLK(Hz)                       = 2000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            APB2 Prescaler                 = 1
  *            Flash Latency(WS)              = 0
  *            Main regulator output voltage  = Scale3 mode
  * @param  None
  * @retval None
  */
void SystemClock_Config(void)
{

    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_OscInitTypeDef RCC_OscInitStruct;


    /* The voltage scaling allows optimizing the power consumption when the device is
       clocked below the maximum system frequency, to update the voltage scaling value
       regarding system frequency refer to product datasheet. */
    __PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /* Enable HSI and HSI48 oscillators and activate PLL with HSI as source */
    RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.HSEState            = RCC_HSE_OFF;
    RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;

    RCC_OscInitStruct.LSEState          = RCC_LSE_ON; /* For USB and RNG clock */
    // PLLCLK = (16 MHz * 4)/2 = 32 MHz
    RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLMUL          = RCC_PLLMUL_4;
    RCC_OscInitStruct.PLL.PLLDIV          = RCC_PLLDIV_2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        DEBUG_MSG("ERROR\n");
    }

    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
    RCC_ClkInitStruct.ClockType      = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK; // 32 MHz
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;         // 32 MHz
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;           // 32 MHz
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;           // 32 MHz
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK) {
        DEBUG_MSG("ERROR\n");
    }
    RCC_PeriphCLKInitTypeDef PeriphClkInit;
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_LPTIM1;
    PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_HSI;
    PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
    PeriphClkInit.LptimClockSelection = RCC_LPTIM1CLKSOURCE_PCLK;

    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
        Error_Handler();
    }
    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);
    /**Configure the Systick */
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
    /* SysTick_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}


void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
#if DEBUG_TRACE == 1
    GPIO_InitTypeDef  GPIO_InitStruct;
    /*##-1- Enable peripherals and GPIO Clocks #################################*/
    /* Enable GPIO TX/RX clock */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /* Enable USART2 clock */
    __HAL_RCC_USART2_CLK_ENABLE(); 
    /*##-2- Configure peripheral GPIO ##########################################*/  
    /* UART TX GPIO pin configuration  */
    GPIO_InitStruct.Pin       = USARTx_TX_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH  ;
    GPIO_InitStruct.Alternate = USARTx_TX_AF;
    HAL_GPIO_Init(USARTx_TX_GPIO_PORT, &GPIO_InitStruct);
#endif
}
#if DEBUG_TRACE == 1
void vprint(const char *fmt, va_list argp)
{

    char string[200];
    if(0 < vsprintf(string,fmt,argp)) // build string
    {
        HAL_UART_Transmit(&UartHandle, (uint8_t*)string, strlen(string), 0xffffff); // send message via UART
    }
}
#endif
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
//void EXTI4_15_IRQHandler(void)
//{
//  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_10);
//}
void  EXTI4_15_IRQHandler ( void ){
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

void  EXTI2_3_IRQHandler ( void ){

    int pintmp = RX_TIMEOUT_IT;
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

void McuSTM32L0::InitMcu ( void ){
    HAL_Init();
    SystemClock_Config();
    SystemCoreClockUpdate();
    RtcInit();
    UartInit ( );
    __HAL_RCC_GPIOB_CLK_ENABLE();        
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin = (GPIO_PIN_0);
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH  ;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct); 
    // LoRa Cs      
    GPIO_InitStruct.Pin = (GPIO_PIN_6);
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH  ;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);    
    InitSpi();    
};

void  McuSTM32L0::Init_Irq ( PinName pin) {
        IRQn_Type IrqNum; 
        int pintmp;
      /* Enable GPIOC clock */
  __HAL_RCC_GPIOC_CLK_ENABLE();
        GPIO_InitTypeDef GPIO_InitStruct;
        GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH  ;
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
        }
        switch ( pintmp ) {
            case 0 :
            case 1 :             
                IrqNum = EXTI0_1_IRQn;
                break;
            case 2 :
            case 3 :             
                IrqNum = EXTI2_3_IRQn;
                break;
            default : 
                IrqNum = EXTI4_15_IRQn;
                break;
        }
        HAL_NVIC_SetPriority(IrqNum, 0, 0);
        //NVIC_SetVector(IrqNum, (uint32_t)IrqHandlerRadio);
        HAL_NVIC_EnableIRQ(IrqNum);
    };

    /******************************************************************************/
/*                             Mcu LOwPower timer Api                         */
/******************************************************************************/
void McuSTM32L0::LowPowerTimerLoRaInit ( ) {
    /* Peripheral clock enable */
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LPTIM1;
    PeriphClkInitStruct.LptimClockSelection = RCC_LPTIM1CLKSOURCE_LSE;
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
    //NVIC_SetVector(LPTIM1_IRQn, (uint32_t)LPTIM1_IRQHandler);
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
void McuSTM32L0::StartTimerMsecond ( void (* _Func) (void *) , void * _obj, int delay){
    Func =  _Func ;
    obj  = _obj;
    int DelayMs2tick = delay * 2 + ( ( 6 * delay ) >> 7);
    HAL_LPTIM_TimeOut_Start_IT(&hlptim1, 65535, DelayMs2tick); // MCU specific
};


void McuSTM32L0::UartInit ( void ) {
#if DEBUG_TRACE == 1
    UartHandle.Instance        = USART2;
    UartHandle.Init.BaudRate   = 115200;
    UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
    UartHandle.Init.StopBits   = UART_STOPBITS_1;
    UartHandle.Init.Parity     = UART_PARITY_NONE;
    UartHandle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
    UartHandle.Init.Mode       = UART_MODE_TX_RX;
    UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;

    if(HAL_UART_Init(&UartHandle) != HAL_OK)
    {
        Error_Handler();
    }
#endif
} 

void McuSTM32L0::MMprint( const char *fmt, ...) {  mcu.SetValueDigitalOutPin ( pinCS, 1);
#if DEBUG_TRACE == 1
  va_list argp;
  va_start(argp, fmt);
  vprint(fmt, argp);
  va_end(argp);
#endif
}
#endif
