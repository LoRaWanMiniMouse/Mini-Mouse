/*

  __  __ _       _                                 
 |  \/  ( _)     ( _)                                
 | \  / |_ _ __  _ _ __ ___   ___  _   _ ___  ___  
 | |\/| | | '_ \| | '_ ` _ \ / _ \| | | / __|/ _ \
 | |  | | | | | | | | | | | | ( _) | |_| \__ \  __/ 
 |_|  |_|_|_| |_|_|_| |_| |_|\___/ \__,_|___/\___| 
                                                   
                                                   
Description       : LoraWan Mac Layer objets.  


License           : Revised BSD License, see LICENSE.TXT file include in the project

Maintainer        : Fabien Holin ( SEMTECH)
*/
#include "mbed.h"
#include  "Define.h"
#include "PhyLayer.h"
#include "sx1276-hal.h"
#include "LoraMacDataStoreInFlash.h"
#ifndef MAC_LAYER_H
#define MAC_LAYER_H

#define LORA_MAC_SYNCWORD                           0x34


template <int NBCHANNEL>
class LoraWanContainer { 
public: 

    LoraWanContainer( PinName interrupt); 
    ~LoraWanContainer( );
    static const uint8_t  NUMBER_OF_CHANNEL = NBCHANNEL; // @note this is an issue it is region dependant so move in region but tbd...
    void BuildTxLoraFrame     ( void );
    void BuildJoinLoraFrame   ( void );
    void EncryptTxFrame       ( void );
    void ConfigureRadioAndSend( void );
    void ConfigureRadioForRx1 ( void );
    void ConfigureRadioForRx2 ( void );
    void ConfigureTimerForRx  ( int type );
    void UpdateMacLayer       ( void );
    void UpdateJoinProcedure  ( void );
    uint8_t IsFrameToSend;
    eRxPacketType   DecodeRxFrame            ( void );
    eStatusLoRaWan  ParseManagementPacket    ( void );

/* LoraWan Context */ 
/* Only 16 ch mask => ChMaskCntl not used */
/* Duty cycle is not managed */

    /*******************************************/
    /*      Update by Link ADR command         */
    /*******************************************/
    uint8_t      MacTxDataRate;
    uint8_t      MacTxDataRateAdr;
    uint8_t      MacTxPower;
    uint16_t     MacChMask;
    uint8_t      MacNbTrans; 
    uint8_t      MacNbTransCpt;
    /********************************************/
    /*     Update by RxParamaSetupRequest       */
    /********************************************/
    uint8_t      MacRx2DataRate ;
    uint32_t     MacRx2Frequency ; 
    uint8_t      MacRx1DataRateOffset;
    /********************************************/
    /*     Update by NewChannelReq command      */
    /********************************************/
    uint32_t  MacTxFrequency        [ NBCHANNEL ];
    uint8_t   MacMinDataRateChannel [ NBCHANNEL ];
    uint8_t   MacMaxDataRateChannel [ NBCHANNEL ];
    uint8_t   MacChannelIndexEnabled[ NBCHANNEL ]; // Contain the index of the activated channel only NbOfActiveChannel value are valid

    
    /********************************************/
    /*   Update by RXTimingSetupReq command     */
    /********************************************/
    int        MacRx1Delay;
    /********************************************/
    /*   Other Data To store                    */
    /********************************************/
    uint16_t   FcntUp;
    uint32_t   FcntDwn;  // Wrapping 16 to 32 bits is managed in AcceptFcntDwn Method
    uint32_t   DevAddr;
    uint8_t    nwkSKey[16];
    uint8_t    appSKey[16];
    
    /*******************************************/
    /* what about keys: AppEUI:Nwskey:AppSkey  */
    /*******************************************/

/* LoRaWan Mac Data for uplink*/
    uint8_t    fPort;
    uint8_t    MType;
    uint8_t    MajorBits;
    uint8_t    Fctrl;
    uint8_t    AckBitForTx;
    uint8_t    UserPayloadSize;
    uint8_t    MacPayloadSize;



/* LoRaWan Mac Data for downlink*/
    uint8_t    fRxPort;
    uint8_t    MtypeRx;
    uint8_t    MajorRx;
    uint8_t    FctrlRx;
    uint8_t    FoptsLength;
    uint8_t    Fopts[16];
    uint8_t    FportRx;
    uint8_t    MacRxPayloadSize;  //@note Have to by replace by a fifo objet to manage class c
    uint8_t    MacRxPayload[255];  //@note Have to by replace by a fifo objet to manage class c
    uint8_t    AvailableRxPacketForUser ;

/* LoRaWan Mac Data for join */
    uint16_t DevNonce;
    uint8_t  CFList[16];
    
/* LoRaWan Mac Data for nwk Ans */
    uint8_t    MacNwkPayload[255];  //@note resize this buffer 
    uint8_t    MacNwkPayloadSize;

    uint8_t    MacNwkAns[255];  //@note reuse user payload data or at least reduce size or use opt byte
    uint8_t    MacNwkAnsSize;

/* LoraWan Config */
    uint8_t AdrModeSelect;
    int AdrAckCnt;
    int AdrAckLimit;
    int AdrAckDelay;
    uint8_t AdrAckReq;
    uint8_t AdrEnable;
    
    
/* Objet RadioContainer*/
    RadioContainer Phy;
    
/*  Timer */
    Timeout    TimerLora ;
    void       IsrTimerRx( void );
    int        StateTimer;
/* Join Duty cycle management */
    uint32_t   RtcNextTimeJoinSecond ;
    uint32_t   RetryJoinCpt;

/*  Flash */
    void LoadFromFlash             ( void );
/***************************************************************/
/*  Virtual Method overwritten by the Class  of the region     */
/***************************************************************/
    virtual void              RegionGiveNextChannel            ( void )                                 = 0; 
    virtual void              RegionSetRxConfig                ( eRxWinType type )                      = 0;
    virtual void              RegionSetPower                   ( uint8_t PowerCmd )                     = 0;
    virtual void              RegionSetMask                    ( void )                                 = 0;
    virtual void              RegionInitChannelMask            ( void )                                 = 0;
    virtual void              RegionGetCFList                  ( void )                                 = 0;
    virtual void              RegionDecreaseDataRate           ( void )                                 = 0;
    virtual eStatusChannel    RegionBuildChannelMask           ( uint8_t ChMaskCntl, uint16_t ChMaskIn) = 0;

    virtual eStatusLoRaWan    RegionIsValidRx1DrOffset     ( uint8_t Rx1DataRateOffset) = 0;
    virtual eStatusLoRaWan    RegionIsValidDataRate        ( uint8_t temp )             = 0;
    virtual eStatusLoRaWan    RegionIsAcceptableDataRate   ( uint8_t DataRate)          = 0;
    virtual eStatusLoRaWan    RegionIsValidMacFrequency    ( uint32_t Frequency)        = 0;
    virtual eStatusLoRaWan    RegionIsValidTxPower         ( uint8_t Power )            = 0;
    virtual eStatusLoRaWan    RegionIsValidChannelIndex    ( uint8_t ChannelIndex)      = 0;
    
/**************************************************************/
/*      Protected Methods and variables                       */
/**************************************************************/
protected :
    uint8_t      MacTxSfCurrent;
    eBandWidth   MacTxBwCurrent;
    uint32_t     MacTxFrequencyCurrent;
    uint8_t      MacRx1SfCurrent;
    eBandWidth   MacRx1BwCurrent;
    uint8_t      MacRx2SfCurrent;
    eBandWidth   MacRx2BwCurrent;
    int          FindEnabledChannel ( uint8_t Index);
    void         PrintMacContext ( void ) ;
private :
    static const uint16_t MAX_FCNT_GAP       = 16384 ;
    void SetMacHeader              ( void );
    void SetFrameHeader            ( void );// no opts
    uint8_t GiveNextChannel       ( void );
    int ExtractRxMhdr              ( void );
    int CheckRxPayloadLength       ( void );
    int ExtractRxFhdr              ( uint16_t *FcntDwnTemp ) ; 
    int AcceptFcntDwn              ( uint16_t FcntDwnTmp ) ;
    void SetAlarm                  ( uint32_t alarmInMs );
    void LinkCheckParser           ( void );
    void LinkADRParser             ( uint8_t NbMultiLinkAdrReq );
    void DutyCycleParser           ( void );
    void RXParamSetupParser        ( void );
    void DevStatusParser           ( void );
    void NewChannelParser          ( void );
    void RXTimingSetupParser       ( void );
    void UpdateDataRateForAdr      ( void );
    void SaveInFlash               ( void );
    sBackUpFlash BackUpFlash;
    uint8_t NwkPayloadIndex ;
    uint8_t RxEmptyPayload ;

}; 


#endif
