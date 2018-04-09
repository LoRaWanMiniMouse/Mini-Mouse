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
#include "UserDefine.h"
#include "ApiSpi.h"
#include "ApiGpio.h"
/********************************************************************************/
/*             The section behind haven't to be modified by user                */
/********************************************************************************/
extern MMInterruptIn RadioGlobalIt ;
extern MMInterruptIn RadioTimeOutGlobalIt ;

#if DEBUG_TRACE == 1
extern Serial pcf;
#define DEBUG_MSG(str)               pcf.printf(str)
//#define DEBUG_PRINTF(fmt, args...)   DEBUG_PRINTF("  %s:%d: "fmt, __FUNCTION__, __LINE__, args)
#define DEBUG_PRINTF(fmt, args...)   pcf.printf(fmt, args)
#define DEBUG_SPRINTF(fmt, args...)  pcf.printf("  %s:%d: "fmt, args)
#define DEBUG_ARRAY(a,b,c)           for(a=0;a!=0;){}
    
#define CHECK_NULL(a)                if(a==NULL){return LGW_HAL_ERROR;}
#else
#define DEBUG_MSG(str)
#define DEBUG_PRINTF(fmt, args...)
#define DEBUG_ARRAY(a,b,c)            for(a=0;a!=0;){}
#define CHECK_NULL(a)                 if(a==NULL){return LGW_HAL_ERROR;}
#endif


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
    UNCONF_DATA_UP,
    UNCONF_DATA_DOWN,
    CONF_DATA_UP,
    CONF_DATA_DOWN,
    REJOIN_REQUEST,
    PROPRIETARY,
};

enum{
    LORAWANR1,
    RFU,
};

typedef enum {
    OTA_DEVICE,
    APB_DEVICE,
}eDeviceTypeOTA_APB;
enum {
    LINK_CHECK_REQ = 2,
    LINK_ADR_REQ,
    DUTY_CYCLE_REQ,
    RXPARRAM_SETUP_REQ,
    DEV_STATUS_REQ,
    NEW_CHANNEL_REQ,
    RXTIMING_SETUP_REQ,
    TXPARAM_SETUP_REQ,
    DIC_CHANNEL_REQ,
};
enum {
    LINK_CHECK_ANS = 2,
    LINK_ADR_ANS,
    DUTY_CYCLE_ANS,
    RXPARRAM_SETUP_ANS,
    DEV_STATUS_ANS,
    NEW_CHANNEL_ANS,
    RXTIMING_SETUP_ANS,
    TXPARAM_SETUP_ANS,
    DIC_CHANNEL_ANS,
};

#define    LINK_CHECK_REQ_SIZE 
#define    LINK_CHECK_ANS_SIZE        3
#define    LINK_ADR_REQ_SIZE          5
#define    LINK_ADR_ANS_SIZE          2
#define    DUTY_CYCLE_REQ_SIZE        2
#define    DUTY_CYCLE_ANS_SIZE        1
#define    RXPARRAM_SETUP_REQ_SIZE    5
#define    RXPARRAM_SETUP_ANS_SIZE    2
#define    DEV_STATUS_REQ_SIZE        1
#define    DEV_STATUS_ANS_SIZE        3
#define    NEW_CHANNEL_REQ_SIZE       6
#define    NEW_CHANNEL_ANS_SIZE       2
#define    RXTIMING_SETUP_REQ_SIZE    2
#define    RXTIMING_SETUP_ANS_SIZE    1
#define    DIC_CHANNEL_REQ_SIZE       5
#define    DIC_CHANNEL_ANS_SIZE       2
#define    TIMEONAIR_JOIN_SF7_MS      65 // ms  
#define    MAX_RETRY_JOIN_DUTY_CYCLE_1000 10

#define MIN_LORAWAN_PAYLOAD_SIZE 12
#define PORTNWK 0

#define MAX_CONFUP_MSG 4
/*****************************************************************************/
/*                   Lora Phy Irg Flags Parameters                           */
/*****************************************************************************/

typedef enum{
    RADIO_IRQ_NONE,
    RXTIMEOUT_IRQ_FLAG      = 0x80,
    RECEIVE_PACKET_IRQ_FLAG = 0x40,
    BAD_PACKET_IRQ_FLAG     = 0x60,
    
}IrqFlags_t;


/********************************************************************************/
/*                   Code implementation Parameters                             */
/********************************************************************************/
#define MSB32FIRST( x ) ( ( ( x & 0x000000FF ) << 24 ) + ( ( x & 0x0000FF00 ) << 8 ) + ( ( x & 0x00FF0000 ) >> 8 ) + ( ( x & 0xFF000000 ) >> 24 ) )
#define MSB16FIRST( x ) ( ( ( x & 0x00FF ) << 8 ) + ( ( x & 0xFF00 ) >> 8 ) )
#define MAX_TX_PAYLOAD_SIZE 255
#define FHDROFFSET 9 // MHDR+FHDR offset if OPT = 0 + fport
#define MICSIZE 4

/*!
 * Frame direction definition for up-link communications
 */
#define UP_LINK                                     0

/*!
 * Frame direction definition for down-link communications
 */
#define DOWN_LINK                                   1

typedef enum { 
    BW125,
    BW250,
    BW500
}eBandWidth;    

enum {
    CHANNEL_DISABLED,
    CHANNEL_ENABLED,
};

typedef enum {
    UNVALID_CHANNEL,
    VALID_CHANNEL,
}eVALID_CHANNEL;

/*User Confi for Adr Mode select*/
typedef enum eDataRateStrategy{
    STATIC_ADR_MODE,
    MOBILE_LONGRANGE_DR_DISTRIBUTION,
    MOBILE_LOWPER_DR_DISTRIBUTION,
    JOIN_DR_DISTRIBUTION,
    USER_DR_DISTRIBUTION,
} eDataRateStrategy;

typedef enum { 
    ERRORLORAWAN = -1,
    OKLORAWAN    = 0,
}eStatusLoRaWan;

typedef enum {
    ERROR_CHANNEL_CNTL = -2,    
    ERROR_CHANNEL_MASK = -1,
    OKCHANNEL    = 0,
}eStatusChannel;
typedef enum {
    NO_MORE_VALID_RX_PACKET,
    USERRX_FOPTSPACKET,
    NWKRXPACKET,
    JOIN_ACCEPT_PACKET,
} eRxPacketType;
typedef enum {
    RX1,
    RX2
}eRxWinType;
/*************************/
/*    SHARE WITH USER    */
/*************************/
enum {
    NO_LORA_RXPACKET_AVAILABLE,
    LORA_RX_PACKET_AVAILABLE,
};

typedef enum { 
    NOT_JOINED,
    JOINED,
} eJoinStatus;

enum { 
    NOFRAME_TOSEND,
    NWKFRAME_TOSEND,
    USERACK_TOSEND,
    USRFRAME_TORETRANSMIT,
};

typedef enum { 
    LORA,
    FSK
}eModulationType;

/*************************/
/*    API CRYPTO         */
/*************************/
enum { 
    UNICASTKEY,
};

/********************************************************************************/
/*                         LORA KEYS USER Specific                              */
/********************************************************************************/
typedef struct sLoRaWanKeys {
    
    uint8_t *              LoRaMacNwkSKey;
    uint8_t *              LoRaMacAppSKey;
    uint8_t *              LoRaMacAppKey;
    uint8_t *              AppEui;
    uint8_t *              DevEui;    
    uint32_t               LoRaDevAddr ;
    eDeviceTypeOTA_APB     OtaDevice;
}sLoRaWanKeys;
#endif

