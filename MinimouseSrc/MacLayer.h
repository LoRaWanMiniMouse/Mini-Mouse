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



class LoraWanContainer { 
public: 
    LoraWanContainer( PinName interrupt ); 
    ~LoraWanContainer( );
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
    eBandWidth    MacRx1Bw;
    /*******************************************/
    /*      Update by Link ADR command         */
    /*******************************************/
    uint8_t      MacTxSf;
    eBandWidth   MacTxBw;
    uint8_t      MacTxPower;
    uint16_t     MacChMask;
    uint8_t      MacNbRepUnconfirmedTx; 
    /********************************************/
    /*     Update by TxParamaSetupRequest       */
    /********************************************/
    uint32_t     MacRx2Frequency ; 
    uint8_t      MacRx2Sf;
    eBandWidth   MacRx2Bw;
    uint8_t      MacRx1SfOffset;
    /********************************************/
    /*     Update by NewChannelReq command      */
    /********************************************/
    uint32_t   MacTxFrequency[16];
    uint32_t   MacMinMaxSFChannel[16];
    uint8_t    NbOfActiveChannel;
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

    
/* LoRaWan Mac Data for nwk Ans */
    uint8_t    MacNwkPayload[255];  //@note resize this buffer 
    uint8_t    MacNwkPayloadSize;

    uint8_t    MacNwkAns[255];  //@note reuse user payload data or at least reduce size or use opt byte
    uint8_t    MacNwkAnsSize;

/* LoraWan Config */
    uint8_t AdrModeSelect;
    
    
/* Objet RadioContainer*/
    RadioContainer Phy;
    
/*  Timer */
    Timeout    TimerLora ;
    void       IsrTimerRx( void );
    int        StateTimer;

/*  Flash */
    void LoadFromFlash             ( void );

private :
    void SetMacHeader              ( void );
    void SetFrameHeader            ( void );// no opts
    void GiveNextSf                ( void );
    uint8_t GiveNextChannel       ( void );
    int ExtractRxMhdr              ( void );
    int CheckRxPayloadLength       ( void );
    int ExtractRxFhdr              ( uint16_t *FcntDwnTemp ) ; 
    int AcceptFcntDwn              ( uint16_t FcntDwnTmp ) ;
    void SetAlarm                  ( uint32_t alarmInMs );
    void LinkCheckParser           ( void );
    void LinkADRParser             ( void );
    void DutyCycleParser           ( void );
    void RXParamSetupParser        ( void );
    void DevStatusParser           ( void );
    void NewChannelParser          ( void );
    void RXTimingSetupParser       ( void );
    uint8_t isValidRx1SfOffset     ( uint8_t );
    uint8_t isValidMacRx2Sf        ( uint8_t );
    uint8_t isValidMacRx2Frequency ( uint32_t );
    uint8_t isValidDataRate        ( uint8_t );
    uint8_t isValidTxPower         ( uint8_t );
    uint8_t isValidChannelMask     ( uint16_t );
    uint8_t isValidNbRep           ( uint8_t ) ;
    void SaveInFlash               ( void );
    //uint8_t crypto_verifyMICandDecrypt ( uint8_t *frame_header, const uint8_t *encrypted_payload ,uint32_t micIn, uint8_t keySet, uint8_t *decrypted_payload, uint8_t PayloadSize);
    sBackUpFlash BackUpFlash;
}; 


#endif
