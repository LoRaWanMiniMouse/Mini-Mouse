/*

  __  __ _       _                                 
 |  \/  (_)     (_)                                
 | \  / |_ _ __  _ _ __ ___   ___  _   _ ___  ___  
 | |\/| | | '_ \| | '_ ` _ \ / _ \| | | / __|/ _ \
 | |  | | | | | | | | | | | | (_) | |_| \__ \  __/ 
 |_|  |_|_|_| |_|_|_| |_| |_|\___/ \__,_|___/\___| 
                                                   
                                                   
Description       : Dedine for loraMac Layer.  


License           : Revised BSD License, see LICENSE.TXT file include in the project

Maintainer        : Fabien Holin (SEMTECH)
*/
#ifndef DEFINE_H
#define DEFINE_H
#include "mbed.h"

/********************************************************************************/
/*                         PINOUT Platform dependant                            */
/********************************************************************************/
                    // MURATA   //    sx1276 MBAS
#define LORA_SPI_MOSI   PA_7    //      D11
#define LORA_SPI_MISO   PA_6    //      D12
#define LORA_SPI_SCLK   PB_3    //      D13
#define LORA_CS         PA_15   //      D10
#define LORA_RESET      PC_0    //      A0
#define TX_RX_IT        PB_4    //      D2
#define RX_TIMEOUT_IT   PB_1    //      D3
#define SERIAL_TX       PA_9    //      D0
#define SERIAL_RX       PA_10   //      D1


/********************************************************************************/
/*                         LORA KEYS USER Specific                              */
/********************************************************************************/
static uint8_t LoRaMacNwkSKey[] =
{ 0x94, 0x02, 0x9E, 0xAB, 0xE9, 0x0C, 0xE3, 0xCF, 0x51, 0xA2, 0x31, 0x7F, 0x03, 0x27, 0xEF, 0xA6 };

/*!
 * AES encryption/decryption cipher application session key
 */
static uint8_t LoRaMacAppSKey[] =
{ 0xBB, 0xEB, 0xEB, 0x4D, 0x5C, 0xAF, 0x95, 0x5D, 0x33, 0xD1, 0xC0, 0xC4, 0x41, 0x55, 0xB5, 0xA1 };


static uint8_t LoRaMacAppKey[] =
{ 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11};
static uint8_t AppEui[] = 
{ 0x70, 0xB3, 0xD5, 0x7E, 0xF0, 0x00, 0x36, 0x12 };


static uint8_t DevEui[] = 
{ 0x33, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x33 };    





/********************************************************************************/
/*                         LoraWan Process States                               */
/********************************************************************************/

typedef enum LoraWan_Process_States {
    LWPSTATE_IDLE ,
    LWPSTATE_SEND ,
    LWPSTATE_RX1 ,
    LWPSTATE_RX2 ,
    LWPSTATE_PROCESSDOWNLINK ,
    LWPSTATE_UPDATEMAC,
    LWPSTATE_TXWAIT,    
    LWPSTATE_ERROR
} eLoraWan_Process_States;
/********************************************************************************/
/*                           Timer Process States                               */
/********************************************************************************/
enum{
    TIMERSTATE_SLEEP,
    TIMERSTATE_RUNNING
};

/*****************************************************************************/
/*                                 Radio Process States                      */
/*****************************************************************************/

enum{
    RADIOSTATE_IDLE,
    RADIOSTATE_TXON,
    RADIOSTATE_TXFINISHED,
    RADIOSTATE_RX1FINISHED,
//    RADIOSTATE_RX2FINISHED
};


/********************************************************************************/
/*                   LoraWan Mac Layer Parameters                               */
/********************************************************************************/
enum{
    JOINREQUEST,
    JOINACCEPT,
    UNCONFDATAUP,
    UNCONFDATADOWN,
    CONFDATAUP,
    CONFDATADOWN,
    REJOINREQUEST,
    PROPRIETARY,
};

enum{
    LORAWANR1,
    RFU,
};

enum {
    LINK_CHECK_REQ,
    LINK_CHECK_ANS,
    LINK_ADR_REQ,
    LINK_ADR_ANS,
    DUTY_CYCLE_REQ,
    DUTY_CYCLE_ANS,
    RXPARRAM_SETUP_REQ,
    RXPARRAM_SETUP_ANS,
    DEV_STATUS_REQ,
    DEV_STATUS_ANS,
    NEW_CHANNEL_REQ,
    NEW_CHANNEL_ANS,
    RXTIMING_SETUP_REQ,
    RXTIMING_SETUP_ANS,
};
enum {
    LINK_CHECK_REQ_SIZE ,
    LINK_CHECK_ANS_SIZE ,
    LINK_ADR_REQ_SIZE,
    LINK_ADR_ANS_SIZE,
    DUTY_CYCLE_REQ_SIZE,
    DUTY_CYCLE_ANS_SIZE,
    RXPARRAM_SETUP_REQ_SIZE,
    RXPARRAM_SETUP_ANS_SIZE,
    DEV_STATUS_REQ_SIZE,
    DEV_STATUS_ANS_SIZE,
    NEW_CHANNEL_REQ_SIZE,
    NEW_CHANNEL_ANS_SIZE,
    RXTIMING_SETUP_REQ_SIZE,
    RXTIMING_SETUP_ANS_SIZE ,
};

#define RX1DELAY              1000 // define in ms
#define RX1DELAYJOIN          5000 // define in ms
#define MAX_FCNT_GAP          16384
#define MINLORAWANPAYLOADSIZE 13
#define PORTNWK 0
/*****************************************************************************/
/*                   Lora Phy Irg Flags Parameters                           */
/*****************************************************************************/

enum{
    RXTIMEOUTIRQFLAG     = 0x80,
    RECEIVEPACKETIRQFLAG = 0x40,
    BADPACKETIRQFLAG = 0x60,
};


/********************************************************************************/
/*                   Code implementation Parameters                             */
/********************************************************************************/
#define MSB32FIRST( x ) ( ( ( x & 0x000000FF ) << 24 ) + ( ( x & 0x0000FF00 ) << 8 ) + ( ( x & 0x00FF0000 ) >> 8 ) + ( ( x & 0xFF000000 ) >> 24 ) )
#define MSB16FIRST( x ) ( ( ( x & 0x00FF ) << 8 ) + ( ( x & 0xFF00 ) >> 8 ) )
#define MAXTXPAYLOADSIZE 255
#define FHDROFFSET 9 // MHDR+FHDR offset if OPT = 0 + fport
#define MICSIZE 4
#define FLASH_UPDATE_PERIOD 4
/*!
 * Frame direction definition for up-link communications
 */
#define UP_LINK                                     0

/*!
 * Frame direction definition for down-link communications
 */
#define DOWN_LINK                                   1

/*User Confi for Adr Mode select*/
typedef enum eDataRateStrategy{
    STATICADRMODE,
    MOBILELONGRANGEADRMODE,
    MOBILELOWPOWERADRMODE,
} eDataRateStrategy;

typedef enum { 
    ERRORLORAWAN = -1,
    OKLORAWAN    = 0,
}eStatusLoRaWan;

typedef enum {
    NOVALIDRXPACKET,
    USERRXPACKET,
    USERRX_FOPTSPACKET,
    NWKRXPACKET,
    JOINACCEPTPACKET,
} eRxPacketType;
enum {
    RX1,
    RX2
};
/*************************/
/*    SHARE WITH USER    */
/*************************/
enum {
    NOLORARXPACKETAVAILABLE,
    LORARXPACKETAVAILABLE,
};

enum { 
    NOTJOINED,
    ISJOINED,
};

enum { 
    NOFRAME_TOSEND,
    NWKFRAME_TOSEND,
    USERACK_TOSEND,
};

/*************************/
/*    API CRYPTO         */
/*************************/
enum { 
    UNICASTKEY,
};
#endif
/* pense bete minimouse)

 SendPayload(uint8_t fPort,const uint8_t* dataIn,const uint16_t sizeIn, uint8_t* dataReceive,uint16_t *sizeOut) api
loraWanContainer.MType = UNCONFDATAUP;//tbupdate Mtype should become a parameter of SendPayload */
