/*

  __  __ _       _                                 
 |  \/  ( _)     ( _)                                
 | \  / |_ _ __  _ _ __ ___   ___  _   _ ___  ___  
 | |\/| | | '_ \| | '_ ` _ \ / _ \| | | / __|/ _ \
 | |  | | | | | | | | | | | | ( _) | |_| \__ \  __/ 
 |_|  |_|_|_| |_|_|_| |_| |_|\___/ \__,_|___/\___| 
                                                   
                                                   
Description       : Minimouse Services Specific objets.  


License           : Revised BSD License, see LICENSE.TXT file include in the project

Maintainer        : Fabien Holin ( SEMTECH)
*/
#ifndef SERVICES_H
#define SERVICES_H
#include "stdint.h"
#include "Define.h"
#include "LoraWanProcess.h"
#include "EncoderMp.h"

#define PROTECTED_PORT    200

template < template <class R> class T, class RADIOTYPE>
    class MMServices : public LoraWanObject<T,RADIOTYPE> {
    public :

    MMServices( sLoRaWanKeys LoRaWanKeys,RADIOTYPE* RadioUser, uint32_t FlashAdress ):LoraWanObject<T,RADIOTYPE>  ( LoRaWanKeys, RadioUser, FlashAdress ){
        NbPacketToSend = 0;
        encoderMp.SetEncoderParam (8, 1, 2); 
    };
    
     /*!
     * \brief Sends an uplink 
     * \param [IN] uint8_t           fPort          Uplink Fport
     * \param [IN] const uint8_t*    dataInFport    User Payload
     * \param [IN] const uint8_t     sizeIn         User Payload Size
     * \param [IN] const uint8_t     PacketType     User Packet Type : UNCONF_DATA_UP, CONF_DATA_UP,

     * \param [OUT] eLoraWan_Process_States         Current state of the LoraWan stack :
     * \param                                            => return LWPSATE_SEND if all is ok
     * \param                                            => return Error in case of payload too long
     * \param                                            => return Error In case of the Lorawan stack previous state is not equal to iddle
     */  
    eLoraWan_Process_States    SendPayload             ( uint8_t fPort, const uint8_t* dataIn, const uint8_t sizeIn, uint8_t PacketType ) {
        uint8_t                  SdataTmp[255]; 
        eLoraWan_Process_States  status;
        SfPort                   = fPort;
        SsizeIn                  = sizeIn;
        SPacketType              = PacketType;
        memcpy( SdataIn, dataIn, sizeIn);
        
        if ( fPort == PROTECTED_PORT ) {
            
            encoderMp.AddDataInFifo ( SdataIn , SdataTmp );
            status = LoraWanObject<T,RADIOTYPE>::SendPayload( fPort,  SdataTmp,  sizeIn + 1,  PacketType );
            if ( encoderMp.SendEncodedFrame == true ) {
                encoderMp.BuildRedundancyFrame ( SdataTmp );
                SsizeIn++;
                memcpy (SdataIn, SdataTmp, SsizeIn );
                NbPacketToSend = 1;
            } else {
                NbPacketToSend =  0;
            }
        } else {
            status = LoraWanObject<T,RADIOTYPE>::SendPayload( fPort,  dataIn,  sizeIn,  PacketType );
            NbPacketToSend = 0;
        }
        return(status);
    };
    
    eLoraWan_Process_States    LoraWanProcess          ( uint8_t* AvailableRxPacket ){
        eLoraWan_Process_States status;
        status = LoraWanObject<T,RADIOTYPE>::LoraWanProcess( AvailableRxPacket );
        if  ( (status == LWPSTATE_IDLE) && ( NbPacketToSend > 0 ) ) {
            status = LoraWanObject<T,RADIOTYPE>::SendPayload( SfPort,  SdataIn,  SsizeIn,  SPacketType );
            NbPacketToSend--;
        }
        return(status);
    };
private :
    EncoderMp encoderMp;
    uint8_t SfPort; 
    uint8_t SdataIn[255]; 
    uint8_t SsizeIn;
    uint8_t SPacketType;
    uint8_t NbPacketToSend;
};
#endif
