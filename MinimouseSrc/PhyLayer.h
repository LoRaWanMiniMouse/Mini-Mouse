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
enum { 
    BW125,
    BW250,
    BW500
};    
class RadioContainer { 
public: 
    RadioContainer( PinName interrupt ); 
    ~RadioContainer( );
    SX1276MB1xAS Radio;
    void RadioContainerInit( void );
    void Send( void );
    void Receive (void );
    void IsrRadio( void ); // call back it tx done
    int GetRadioState( void );
    void SetTxConfig( void );
    void SetRxConfig( void );
    uint8_t    TxPhyPayload[MAXTXPAYLOADSIZE]; // @note should be private to be safer , in this case have to create a set function for send in lorawan process
    uint16_t   TxPayloadSize;
    uint8_t    RxPhyPayload[MAXTXPAYLOADSIZE]; 
    uint16_t   RxPhyPayloadSize;
    int RxPhyPayloadSnr;
    int RxPhyPayloadRssi;
    uint32_t   TxFrequency;
    uint8_t    TxPower;
    uint8_t    TxSf;
    uint32_t   RxFrequency;
    uint8_t    RxSf;
    uint32_t   DevAddrIsr ; // copy of the devaddr to be tested in the isr routine
    uint8_t    RegIrqFlag;
    eJoinStatus  JoinedStatus; //@note used in isr routine to filter or not on devaddr
    int StateRadioProcess;
    
    //@note probably have to split with a timer objet

    uint32_t   TimestampRtcIsr;
private :
    InterruptIn TxInterrupt;
    InterruptIn RxTimeoutInterrupt;
    int DumpRxPayloadAndMetadata ( void );
    void ClearIrqRadioFlag ( void );
    void GetIrqRadioFlag ( void );
};
#endif
