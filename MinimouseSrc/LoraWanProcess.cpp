/*

  __  __ _       _                                 
 |  \/  (_)     (_)                                
 | \  / |_ _ __  _ _ __ ___   ___  _   _ ___  ___  
 | |\/| | | '_ \| | '_ ` _ \ / _ \| | | / __|/ _ \
 | |  | | | | | | | | | | | | (_) | |_| \__ \  __/ 
 |_|  |_|_|_| |_|_|_| |_| |_|\___/ \__,_|___/\___| 
                                                   
                                                   
Description       : LoraWanProcess Class definition.  


License           : Revised BSD License, see LICENSE.TXT file include in the project

Maintainer        : Fabien Holin (SEMTECH)
*/
#include "LoraWanProcess.h"
#include "utilities.h"
#include "Define.h"
#include "ApiMcu.h"


template class LoraWanObject< LoraRegionsEU, SX1276 >;
template class LoraWanObject< LoraRegionsEU, SX126x >;

template <template <class R> class T, class RADIOTYPE>
LoraWanObject<T,RADIOTYPE>::LoraWanObject( sLoRaWanKeys LoRaWanKeys, RADIOTYPE * RadioUser,uint32_t FlashAdress ):packet(  LoRaWanKeys, RadioUser,FlashAdress ) {
    StateLoraWanProcess=LWPSTATE_IDLE;
    packet.MajorBits= LORAWANR1;
    FailSafeTimestamp = mcu.RtcGetTimeSecond( );
}; 
template <template <class R> class T, class RADIOTYPE> 
LoraWanObject <T,RADIOTYPE> ::~LoraWanObject() {
};
/************************************************************************************************/
/*                      Public  Methods                                                         */
/************************************************************************************************/


/***********************************************************************************************/
/*    LoraWanProcess Method                                                                    */
/***********************************************************************************************/

template <template <class R> class T, class RADIOTYPE> 
eLoraWan_Process_States LoraWanObject <T,RADIOTYPE> ::LoraWanProcess( uint8_t* AvailableRxPacket ) {

    *AvailableRxPacket = NO_LORA_RXPACKET_AVAILABLE;
    #if LOW_POWER_MODE == 1
    if ( ( IsJoined ( ) == NOT_JOINED ) && ( mcu.RtcGetTimeSecond( ) < packet.RtcNextTimeJoinSecond ) ){
        DEBUG_PRINTF("TOO SOON TO JOIN time is  %d time target is : %d \n",mcu.RtcGetTimeSecond( ), packet.RtcNextTimeJoinSecond);
        StateLoraWanProcess = LWPSTATE_IDLE ;
    }        
    #endif
    if ( ( mcu.RtcGetTimeSecond( ) - FailSafeTimestamp ) > 120 ) {
        RadioReset ( ) ;
        StateLoraWanProcess = LWPSTATE_ERROR ;
        DEBUG_MSG ( "ERROR : FAILSAFE EVENT OCCUR \n");
        // NVIC_SystemReset() move into the user main;
    }        
    switch ( StateLoraWanProcess ) {
    /************************************************************************************/
    /*                                    STATE IDLE                                    */
    /************************************************************************************/
        case LWPSTATE_IDLE :
            break;
    /************************************************************************************/
    /*                                    STATE TX                                      */
    /************************************************************************************/
        case LWPSTATE_SEND:
            switch ( GetRadioState( ) ) {
                
                case  RADIOSTATE_IDLE :
                    AttachRadioIsr ( );                    
                    packet.ConfigureRadioAndSend( );
                    DEBUG_MSG( "\n" );
                    DEBUG_MSG( "  **************************\n " );
                    DEBUG_MSG( " *       Send Payload     *\n " );
                    DEBUG_MSG( " **************************\n " );
                    break ; 
            
                case RADIOSTATE_TXFINISHED : 
                    packet.ConfigureTimerForRx ( RX1 );
                    StateLoraWanProcess = LWPSTATE_RX1;
                    break ;
                
                default :
                    break;
            }
            break;
    /************************************************************************************/
    /*                                   STATE RX1                                      */
    /* RX1DELAY is defined in ms                                                        */
    /************************************************************************************/
        case LWPSTATE_RX1:
            if ( GetRadioState( ) == RADIOSTATE_RX1FINISHED ) {
                if ( GetRadioIrqFlag ( ) == RECEIVE_PACKET_IRQ_FLAG) {
                    DEBUG_MSG( "\n" );
                    DEBUG_MSG( "  **************************\n " );
                    DEBUG_MSG( " * Receive a downlink RX1 *\n " );
                    DEBUG_MSG( " **************************\n " );
                    StateLoraWanProcess = LWPSTATE_PROCESSDOWNLINK;
                } else { 
                    DEBUG_MSG( "\n" );
                    DEBUG_MSG( "  **************************\n " );
                    DEBUG_MSG( " *      RX1 Timeout       *\n " );
                    DEBUG_MSG( " **************************\n " );
                    packet.ConfigureTimerForRx ( RX2 );
                    StateLoraWanProcess = LWPSTATE_RX2;
                }
            }
            break;
    /************************************************************************************/
    /*                                   STATE RX2                                      */
    /************************************************************************************/
        case LWPSTATE_RX2:
                            
            if ( GetRadioState( ) == RADIOSTATE_IDLE ) {
                if ( GetRadioIrqFlag ( ) == RECEIVE_PACKET_IRQ_FLAG) {
                    DEBUG_MSG( "\n" );
                    DEBUG_MSG( "  **************************\n " );
                    DEBUG_MSG( " * Receive a downlink RX2 *\n " );
                    DEBUG_MSG( " **************************\n " );
                    StateLoraWanProcess = LWPSTATE_PROCESSDOWNLINK; 
                } else {
                    DEBUG_MSG( "\n" );
                    DEBUG_MSG( "  **************************\n " );
                    DEBUG_MSG( " *      RX2 Timeout       *\n " );
                    DEBUG_MSG( " **************************\n " );
                    StateLoraWanProcess = LWPSTATE_UPDATEMAC;
                }
            }
            break;
    /************************************************************************************/
    /*                              STATE PROCESS DOWNLINK                              */
    /* At this step crc is valid                                                        */
    /*    Step 1 : CheckRxPayloadLength                                                 */
    /*    Step 2 : ExtractRxMhdr                                                        */
    /*    Step 3 : ExtractRxFhdr                                                        */
    /*    Step 4 : Check Mic                                                            */
    /*    Step 5 : Decrypt Payload                                                      */
    /*    Step 6 : Extract Fport to select Between App/nwm Payload                      */
    /************************************************************************************/
        case LWPSTATE_PROCESSDOWNLINK:
            DEBUG_MSG( "\n" );
            DEBUG_MSG( "  **************************\n " );
            DEBUG_MSG( " * Process Downlink       *\n " );
            DEBUG_MSG( " **************************\n " );
            ValidRxPacket = packet.DecodeRxFrame( ); // return NOVALIDRXPACKET or  USERRX_FOPTSPACKET or NWKRXPACKET or JOIN_ACCEPT_PACKET.
            StateLoraWanProcess = LWPSTATE_UPDATEMAC;
            break;
    /************************************************************************************/
    /*                              STATE UPDATE MAC                                    */
    /************************************************************************************/
        case LWPSTATE_UPDATEMAC:
            DetachRadioIsr ( );
            packet.Phy.StateRadioProcess = RADIOSTATE_IDLE;  
            DEBUG_MSG( "\n" );
            DEBUG_MSG( "  **************************\n " );
            DEBUG_MSG( " *       UpdateMac        *\n " );
            DEBUG_MSG( " **************************\n " );
            if ( ValidRxPacket == JOIN_ACCEPT_PACKET){
                packet.UpdateJoinProcedure( );
                packet.RegionSetDataRateDistribution( packet.AdrModeSelect);//@note because datarate Distribution has been changed during join
            }
            if ( ( ValidRxPacket == NWKRXPACKET) || ( ValidRxPacket == USERRX_FOPTSPACKET) ) {
                packet.ParseManagementPacket( );
            }
            packet.UpdateMacLayer();
            *AvailableRxPacket = packet.AvailableRxPacketForUser;
            if ( ( packet.IsFrameToSend == NWKFRAME_TOSEND ) || ( packet.IsFrameToSend == USRFRAME_TORETRANSMIT) ) {// @note ack send during the next tx|| ( packet.IsFrameToSend == USERACK_TOSEND ) ) {
                packet.IsFrameToSend = NOFRAME_TOSEND;
                RtcTargetTimer = mcu.RtcGetTimeSecond( ) + randr( 2, 6 ); 
                StateLoraWanProcess = LWPSTATE_TXwait;
            } else {
                RadioReset ( ) ; 
                StateLoraWanProcess = LWPSTATE_IDLE;
            }
            ValidRxPacket = NO_MORE_VALID_RX_PACKET;
            break;
    /************************************************************************************/
    /*                              STATE TXwait MAC                                    */
    /************************************************************************************/
        case LWPSTATE_TXwait:
            DEBUG_MSG(".");
            if ( mcu.RtcGetTimeSecond( ) > RtcTargetTimer) {
                StateLoraWanProcess = LWPSTATE_SEND; //@note the frame have already been prepare in Upadate Mac Layer
            }
            break;

        default: 
            DEBUG_MSG( " Illegal state\n " );
            break;
        }
    return ( StateLoraWanProcess );
}
 
/***********************************************************************************************/
/*    End Of LoraWanProcess Method                                                             */
/***********************************************************************************************/


/**************************************************/
/*            LoraWan  Join  Method               */
/**************************************************/
template <template <class R> class T, class RADIOTYPE> 
eLoraWan_Process_States LoraWanObject <T,RADIOTYPE> ::Join ( void ) {
    if ( StateLoraWanProcess != LWPSTATE_IDLE ) {
        DEBUG_MSG( " ERROR : LP STATE NOT EQUAL TO IDLE \n" );
        return ( LWPSTATE_ERROR );
    }
    if ( GetIsOtaDevice ( ) == APB_DEVICE ) {
        DEBUG_MSG( " ERROR : APB DEVICE CAN'T PROCCED A JOIN REQUEST\n" );
        return ( LWPSTATE_ERROR );
    }
    FailSafeTimestamp = mcu.RtcGetTimeSecond( ) ;
    packet.Phy.JoinedStatus = NOT_JOINED;
    packet.MacNbTransCpt = packet.MacNbTrans = 1;
    packet.RegionSetDataRateDistribution( JOIN_DR_DISTRIBUTION ); 
    packet.RegionGiveNextDataRate ( );
    packet.BuildJoinLoraFrame( );
    packet.MacRx2DataRate = packet.RX2DR_INIT;
    packet.MacRx1Delay = packet.JOIN_ACCEPT_DELAY1;
    StateLoraWanProcess = LWPSTATE_SEND;
    return( StateLoraWanProcess );
};


/**************************************************/
/*          LoraWan  IsJoined  Method             */
/**************************************************/
template <template <class R> class T, class RADIOTYPE> 
eJoinStatus LoraWanObject <T,RADIOTYPE> ::IsJoined( void ) {
    eJoinStatus status = NOT_JOINED;
    status = packet.Phy.JoinedStatus;
    return ( status );
}

/**************************************************/
/*          LoraWan  IsJoined  Method             */
/**************************************************/
template <template <class R> class T, class RADIOTYPE> 
void LoraWanObject <T,RADIOTYPE> ::NewJoin ( void ) {
    packet.Phy.JoinedStatus = NOT_JOINED; 
}
/**************************************************/
/*         LoraWan  SendPayload  Method           */
/**************************************************/
template <template <class R> class T, class RADIOTYPE> 
eLoraWan_Process_States LoraWanObject <T,RADIOTYPE> ::SendPayload ( uint8_t fPort, const uint8_t* dataIn, const uint8_t sizeIn, uint8_t PacketType ) {
    eStatusLoRaWan status;
    FailSafeTimestamp = mcu.RtcGetTimeSecond( ) ;
    packet.RegionGiveNextDataRate ( ); // both choose  the next tx data rate but also compute the Sf and Bw (region )
    status = packet.RegionMaxPayloadSize ( sizeIn );
    if ( status == ERRORLORAWAN ) {
        DEBUG_MSG( " ERROR : PAYLOAD SIZE TOO HIGH \n" );
        return ( LWPSTATE_ERROR );
    }
    if ( GetIsOtaDevice ( ) == OTA_DEVICE ) {
        if ( packet.Phy.JoinedStatus ==  NOT_JOINED ) {
            DEBUG_MSG( " ERROR : OTA DEVICE NOT JOINED YET\n" );
            return ( LWPSTATE_ERROR );
        }
    }
    RadioReset ( ) ; 
    CopyUserPayload( dataIn,sizeIn );
    packet.UserPayloadSize = sizeIn;
    packet.fPort = fPort;
    packet.MType = PacketType;

    packet.BuildTxLoraFrame( );
    packet.EncryptTxFrame( );
    if (PacketType == CONF_DATA_UP){
        packet.MacNbTransCpt = MAX_CONFUP_MSG;
    } else {
        packet.MacNbTransCpt = packet.MacNbTrans;
    }
    StateLoraWanProcess = LWPSTATE_SEND;
    return( StateLoraWanProcess );
};

/**************************************************/
/*        LoraWan  Receive  Method                */
/**************************************************/
template <template <class R> class T, class RADIOTYPE> 
eStatusLoRaWan LoraWanObject <T,RADIOTYPE> ::ReceivePayload ( uint8_t* UserRxFport, uint8_t* UserRxPayload, uint8_t* UserRxPayloadSize ) {
    eStatusLoRaWan status = OKLORAWAN; 
    if (packet.AvailableRxPacketForUser == NO_LORA_RXPACKET_AVAILABLE) {
        status = ERRORLORAWAN ;
    } else {
        *UserRxPayloadSize = packet.MacRxPayloadSize;
        *UserRxFport = packet.FportRx;
        memcpy( UserRxPayload, &packet.MacRxPayload[0], packet.MacRxPayloadSize);
        packet.AvailableRxPacketForUser = NO_LORA_RXPACKET_AVAILABLE ;
    }
    return( status );
};

/**************************************************/
/*       LoraWan  AdrModeSelect  Method           */
/**************************************************/
template <template <class R> class T, class RADIOTYPE>
void LoraWanObject <T,RADIOTYPE> ::SetDataRateStrategy( eDataRateStrategy adrModeSelect ) {
    packet.AdrModeSelect = adrModeSelect;
    packet.RegionSetDataRateDistribution( adrModeSelect );
};


/**************************************************/
/*         LoraWan  GetDevAddr  Method            */
/**************************************************/
template <template <class R> class T, class RADIOTYPE> 
uint32_t LoraWanObject <T,RADIOTYPE> ::GetDevAddr ( void ) {
    return(packet.DevAddr);
}

/**************************************************/
/*         LoraWan  GetNextPower  Method          */
/**************************************************/
template <template <class R> class T, class RADIOTYPE>
uint8_t LoraWanObject <T,RADIOTYPE> ::GetNextPower ( void ) {
    return(packet.MacTxPower);
}

/**************************************************/
/*    LoraWan  GetLorawanProcessState  Method     */
/**************************************************/
template <template <class R> class T, class RADIOTYPE> 
eLoraWan_Process_States LoraWanObject <T,RADIOTYPE> ::GetLorawanProcessState ( void ) {
    return(StateLoraWanProcess);
}
 
/**************************************************/
/*    LoraWan  RestoreContext  Method     */
/**************************************************/
template <template <class R> class T, class RADIOTYPE> 
void LoraWanObject <T,RADIOTYPE> ::RestoreContext ( void ) {
    packet.LoadFromFlash ( );
}; 




/**************************************************/
/*   LoraWan  GetNextMaxPayloadLength  Method     */
/**************************************************/
template <template <class R> class T, class RADIOTYPE>
uint32_t LoraWanObject <T,RADIOTYPE> ::GetNextMaxPayloadLength ( void ) {// error return during tx send to be replace by get datarate?
    return(0);//@NOTE NOT YET IMPLEMENTED
}


/**************************************************/
/*        LoraWan  GetNextDataRate  Method        */
/**************************************************/
template <template <class R> class T, class RADIOTYPE> 
uint8_t LoraWanObject <T,RADIOTYPE> ::GetNextDataRate ( void ) { // note return datareate in case of adr
    return(0);//@NOTE NOT YET IMPLEMENTED
}


template <template <class R> class T, class RADIOTYPE> 
 void  LoraWanObject <T,RADIOTYPE> :: FactoryReset ( void ) {
     packet.SetBadCrcInFlash ( ) ;
 }
 
template <template <class R> class T, class RADIOTYPE> 
eDeviceTypeOTA_APB LoraWanObject <T,RADIOTYPE> ::GetIsOtaDevice (void){
    return (eDeviceTypeOTA_APB)packet.otaDevice;
}
template <template <class R> class T, class RADIOTYPE> 
void LoraWanObject <T,RADIOTYPE> ::SetOtaDevice (eDeviceTypeOTA_APB  deviceType){
    packet.otaDevice = deviceType;
}

/************************************************************************************************/
/*                      Private  Methods                                                        */
/************************************************************************************************/
template <template <class R> class T, class RADIOTYPE> 
void LoraWanObject <T,RADIOTYPE> ::CopyUserPayload( const uint8_t* dataIn, const uint8_t sizeIn ) {
    memcpy( &packet.Phy.TxPhyPayload[ FHDROFFSET + packet.FoptsTxLengthCurrent ], dataIn, sizeIn );
};
 
template <template <class R> class T, class RADIOTYPE> 
uint8_t LoraWanObject <T,RADIOTYPE> ::GetStateTimer(void) {
    return (packet.StateTimer);
}

template <template <class R> class T, class RADIOTYPE> 
uint8_t LoraWanObject <T,RADIOTYPE> ::GetRadioState ( void ) {
    return packet.Phy.GetRadioState( );
};
 
template <template <class R> class T, class RADIOTYPE> 
uint8_t LoraWanObject <T,RADIOTYPE> ::GetRadioIrqFlag ( void ) {
    return packet.Phy.RegIrqFlag;
};
template <template <class R> class T, class RADIOTYPE> 
void LoraWanObject <T,RADIOTYPE> ::RadioReset ( void ) {
    packet.Phy.Radio->Reset();
}
template <template <class R> class T, class RADIOTYPE> 
uint8_t LoraWanObject <T,RADIOTYPE> ::GetNbOfReset (void){
    return packet.NbOfReset;
}
