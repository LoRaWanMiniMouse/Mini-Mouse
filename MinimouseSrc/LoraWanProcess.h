/*

  __  __ _       _                                 
 |  \/  (_)     (_)                                
 | \  / |_ _ __  _ _ __ ___   ___  _   _ ___  ___  
 | |\/| | | '_ \| | '_ ` _ \ / _ \| | | / __|/ _ \
 | |  | | | | | | | | | | | | (_) | |_| \__ \  __/ 
 |_|  |_|_|_| |_|_|_| |_| |_|\___/ \__,_|___/\___| 
                                                   
                                                   
Description       : LoraWanProcess Class description.  


License           : Revised BSD License, see LICENSE.TXT file include in the project

Maintainer        : Fabien Holin (SEMTECH)
*/
#ifndef LORAWANPROCESS_H
#define LORAWANPROCESS_H
#include "mbed.h"
#include "Define.h"
#include "MacLayer.h"
#include "Regions.h"


template <class T>
class LoraWanObjet { 
public: 
    LoraWanObjet( PinName interrupt ); 
    ~LoraWanObjet();
    eLoraWan_Process_States    SendPayload   ( uint8_t fPort, const uint8_t* dataIn, const uint16_t sizeIn, uint8_t PacketType );
    uint8_t    ReceivePayload( uint8_t* UserRxFport, uint8_t* UserRxPayload, uint8_t* UserRxPayloadSize );
    eLoraWan_Process_States    Join          ( void );
    eJoinStatus    IsJoined      ( void );
    void       SetDataRateStrategy ( eDataRateStrategy adrModeSelect );
    eLoraWan_Process_States    LoraWanProcess( uint8_t* AvailableRxPacket );
    uint8_t    GetRadioState ( void );
    void       RestoreContext( void ); 
    T packet;
/* not implemented yet*/
    uint8_t    TryToJoin               ( void );
    uint32_t   GetNextMaxPayloadLength ( void );
    uint32_t   GetDevAddr              ( void );
    uint8_t    GetNextPower            ( void );
    uint8_t    GetNextDataRate         ( void );
    uint8_t    GetLorawanProcessState  ( void );
    
    eLoraWan_Process_States StateLoraWanProcess; // for debug not private
private :
    //int StateLoraWanProcess;
    void CopyUserPayload( const uint8_t* dataIn, const uint16_t sizeIn );
    uint8_t GetStateTimer( void );
    uint8_t GetRadioIrqFlag ( void );
    uint8_t ValidRxPacket; 
    uint32_t RtcTargetTimer;
    void RadioReset ( void ) ;
};
//extern LoraWanObjet Lp;
#endif
