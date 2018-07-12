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
#include "sx1276.h"
#include "sx1272.h"
#include "SX126x.h"
#include "Define.h"
#ifndef PHY_LAYER_H
#define PHY_LAYER_H


template < class R >
class RadioContainer { 
public: 
    RadioContainer( R * RadioUser  ); 
    ~RadioContainer( );
    R* Radio;
    void Send              (eModulationType MacTxModulationCurrent, uint32_t TxFrequencyMac, uint8_t TxPowerMac, uint8_t TxSfMac, eBandWidth TxBwMac, uint16_t TxPayloadSizeMac );
    void Receive           ( void );
    void IsrRadio          ( void ); // Isr routine implemented in IsrRoutine.cpp file
    static void CallbackIsrRadio (void * obj){(reinterpret_cast<RadioContainer< R >*>(obj))->IsrRadio();} ;
    void AttachIsr         ( void ) ;
    void DetachIsr         ( void ); 
    int GetRadioState      ( void );
    void SetRxConfig       (eModulationType RxModulation, uint32_t RxFrequencyMac, uint8_t RxSfMac, eBandWidth RxBwMac , uint32_t RxWindowMs);
    uint32_t               GetTxFrequency ( void );
    uint8_t                TxPhyPayload[MAX_TX_PAYLOAD_SIZE]; 
    uint8_t                RxPhyPayload[MAX_TX_PAYLOAD_SIZE]; 
    uint8_t                RxPhyPayloadSize;
    int                    RxPhyPayloadSnr;
    int                    RxPhyPayloadRssi;
    uint16_t               TxPayloadSize;
    uint32_t               DevAddrIsr ; // a copy of the devaddr to be tested in the isr routine + devaddrclassCG0 & devaddrclasccG1
	  eClassCEnable          ClassCG0EnableIsr; 
	  eClassCEnable          ClassCG1EnableIsr; 
		uint32_t               DevAddrClassCG0Isr;
	  uint32_t               DevAddrClassCG1Isr;
    IrqFlags_t             RegIrqFlag;
    eJoinStatus            JoinedStatus; // used in isr routine to not filter on devaddr
    int                    StateRadioProcess;
    uint32_t               TimestampRtcIsr;
    uint32_t               LastTimeRxWindowsMs;
    uint32_t               SymbolDuration;
		uint32_t               LastItTimeFailsafe;
		eValidDevAddr          CurrentDevaddrType;
		/**********for class c************/
		eIsReceiveOnRXC        IsReceiveOnRXC;

private :
    uint32_t             RxFrequency;
    eBandWidth           RxBw;
    uint8_t              RxSf;
    eModulationType      RxMod;
    eModulationType      CurrentMod;
    uint32_t             TxFrequency;
    uint8_t              TxPower;
    uint8_t              TxSf;
    eBandWidth           TxBw;

    int                  DumpRxPayloadAndMetadata ( void );
	  eValidDevAddr        CheckDevAddr (uint32_t devAddrToTest); 
};
#endif
