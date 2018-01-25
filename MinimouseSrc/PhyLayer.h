/*

  __  __ _       _                                 
 |  \/  (_)     (_)                                
 | \  / |_ _ __  _ _ __ ___   ___  _   _ ___  ___  
 | |\/| | | '_ \| | '_ ` _ \ / _ \| | | / __|/ _ \
 | |  | | | | | | | | | | | | (_) | |_| \__ \  __/ 
 |_|  |_|_|_| |_|_|_| |_| |_|\___/ \__,_|___/\___| 
                                                   
                                                   
Description       : LoraWan Phy Layer objets.  


License           : Revised BSD License, see LICENSE.TXT file include in the project

Maintainer        : Fabien Holin (SEMTECH)
*/
#include "sx1276-hal.h"
#include "Define.h"
#ifndef PHY_LAYER_H
#define PHY_LAYER_H

/*   BW enum for LoRa */

class RadioContainer { 
public: 
    RadioContainer( PinName interrupt ); 
    ~RadioContainer( );
    SX1276MB1xAS Radio;
    void RadioContainerInit( void );
    void Send              (eModulationType MacTxModulationCurrent, uint32_t TxFrequencyMac, uint8_t TxPowerMac, uint8_t TxSfMac, uint32_t TxBwMac, uint16_t TxPayloadSizeMac );
    void Receive           ( void );
    void IsrRadio          ( void ); // call back it tx done
    void AttachIsr         ( void ) ;
    void DetachIsr         ( void ); 
    int GetRadioState      ( void );
    void SetRxConfig       (eModulationType RxModulation, uint32_t RxFrequencyMac, uint8_t RxSfMac, uint32_t RxBwMac );
    uint32_t GetTxFrequency ( void );
    uint8_t    TxPhyPayload[MAXTXPAYLOADSIZE]; // @note should be private to be safer , in this case have to create a set function for send in lorawan process
    uint8_t    RxPhyPayload[MAXTXPAYLOADSIZE]; 
    uint16_t   RxPhyPayloadSize;
    int RxPhyPayloadSnr;
    int RxPhyPayloadRssi;
    uint16_t   TxPayloadSize;
    uint32_t   DevAddrIsr ; // copy of the devaddr to be tested in the isr routine
    uint8_t    RegIrqFlag;
    eJoinStatus  JoinedStatus; //@note used in isr routine to filter or not on devaddr
    int StateRadioProcess;
    
    //@note probably have to split with a timer objet

    uint32_t   TimestampRtcIsr;
private :
    uint32_t     RxFrequency;
    uint32_t     RxBw;
    uint8_t      RxSf;
    uint32_t     TxFrequency;
    uint8_t      TxPower;
    uint8_t      TxSf;
    uint32_t     TxBw;
    InterruptIn  TxInterrupt;
    InterruptIn  RxTimeoutInterrupt;
    int DumpRxPayloadAndMetadata ( void );
    void ClearIrqRadioFlag ( void );
    void GetIrqRadioFlag ( void );
};
#endif
