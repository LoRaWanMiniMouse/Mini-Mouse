/*

  __  __ _       _                                 
 |  \/  (_)     (_)                                
 | \  / |_ _ __  _ _ __ ___   ___  _   _ ___  ___  
 | |\/| | | '_ \| | '_ ` _ \ / _ \| | | / __|/ _ \
 | |  | | | | | | | | | | | | (_) | |_| \__ \  __/ 
 |_|  |_|_|_| |_|_|_| |_| |_|\___/ \__,_|___/\___| 
                                                   
                                                   
Description       : User Define for loraMac Layer.  


License           : Revised BSD License, see LICENSE.TXT file include in the project

Maintainer        : Fabien Holin (SEMTECH)
*/
#ifndef USERDEFINE_H
#define USERDEFINE_H

//#define BOARD_L4 1
/********************************************************************************/
/*                         Application     dependant                            */
/********************************************************************************/
#define DEBUG_TRACE    1      // Set to 1 to activate debug traces
#define LOW_POWER_MODE 0     // Set to 1 to activate sleep mode , set to 0 to replace by wait functions (easier in debug mode) 
#define DEBUG_TRACE_ENABLE 0  // Set to 1 to activate DebugTrace 

#define LOW_SPEED_CLK  LSE    //

#ifdef MURATA_BOARD
    #define UART_NUM                  USART2
    #define UART_TX                   PA_2
    #define UART_RX                   PA_3
    #define LORA_SPIx                 SPI1   // select your spi number
    #define LORA_SPI_MOSI             PA_7
    #define LORA_SPI_MISO             PA_6
    #define LORA_SPI_SCLK             PB_3
    #define LORA_CS                   PA_15
    #define LORA_RESET                PC_0
    #define TX_RX_IT                  PB_4     // Interrupt TX/RX Done
    #define RX_TIMEOUT_IT             PB_1     // Interrupt RX TIME OUT 
    #define RADIO_ANT_SWITCH_RX       PA_1
    #define RADIO_ANT_SWITCH_TX_RF0   PC_2
    #define RADIO_ANT_SWITCH_TX_BOOST PC_1
    #define RADIO_TCX0_POWER          PA_12
    #define DEBUG                     PA_10
    #define CRYSTAL_ERROR              50 // Crystal error of the MCU to fine adjust the rx window for lorawan ( ex: set 3� for a crystal error = 0.3%)
    #define BOARD_DELAY_RX_SETTING_MS  2  // Delay introduce by the mcu Have to fine tune to adjust the window rx for lorawan
    #define PA_BOOST_CONNECTED         0 //  Set to 1 to select Pa_boost outpin pin on the sx127x 
    #define USERFLASHADRESS 0x8080000U   // start flash adress to store lorawan context

/*SX1276 BOARD specific */
#else 
    #ifdef SX126x_BOARD
        #define UART_NUM           USART2
        #define UART_TX            PA_2
        #define UART_RX            PA_3

        #define LORA_SPIx           SPI1   // select your spi number
        #define LORA_SPI_MOSI       D11
        #define LORA_SPI_MISO       D12
        #define LORA_SPI_SCLK       D13
        #define LORA_CS             D7
        #define LORA_RESET          A0
        #define TX_RX_IT            D5     // Interrupt TX/RX Done
        #define LORA_BUSY           D3
        #define DEBUG               PA_10
        #define CRYSTAL_ERROR              20 // Crystal error of the MCU to fine adjust the rx window for lorawan ( ex: set 3� for a crystal error = 0.3%)
        #define BOARD_DELAY_RX_SETTING_MS  4  // Delay introduce by the mcu Have to fine tune to adjust the window rx for lorawan
        #define PA_BOOST_CONNECTED         1 //  Set to 1 to select Pa_boost outpin pin on the sx127x 
        #define USERFLASHADRESS 0x807E000U   // start flash adress to store lorawan context 

    #else
        #define UART_NUM           USART2
        #define UART_TX            PA_2
        #define UART_RX            PA_3

        #define LORA_SPIx           SPI1   // select your spi number
        #define LORA_SPI_MOSI       D11
        #define LORA_SPI_MISO       D12
        #define LORA_SPI_SCLK       D13
        #define LORA_CS             D10
        #define LORA_RESET          A0
        #define DEBUG               PA_10
        #define TX_RX_IT            D2     // Interrupt TX/RX Done
        #define CRYSTAL_ERROR              60 // Crystal error of the MCU to fine adjust the rx window for lorawan ( ex: set 3� for a crystal error = 0.3%)
        #define BOARD_DELAY_RX_SETTING_MS  20  // Delay introduce by the mcu Have to fine tune to adjust the window rx for lorawan
        #define PA_BOOST_CONNECTED         0 //  Set to 1 to select Pa_boost outpin pin on the sx127x 
        #define RX_TIMEOUT_IT       D3     // Interrupt RX TIME OUT 
        #define USERFLASHADRESS 0x807E000U   // start flash adress to store lorawan context
    #endif
    #endif 
#define FLASH_UPDATE_PERIOD 32      // The Lorawan context is stored in memory with a period equal to FLASH_UPDATE_PERIOD packets transmitted


#define USER_NUMBER_OF_RETRANSMISSION   1// Only used in case of user defined darate distribution strategy
#define USER_DR_DISTRIBUTION_PARAMETERS 0x00000001  // Only used in case of user defined darate distribution strategy refered to doc that explain this value
#endif
