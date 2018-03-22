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
#include "ApiTimers.h"
#include "utilities.h"
#include "Define.h"
#if DEBUG_TRACE == 1
    Serial pcf( SERIAL_TX, SERIAL_RX );
#endif
InterruptIn RadioGlobalIt ( TX_RX_IT ) ;
InterruptIn RadioTimeOutGlobalIt ( RX_TIMEOUT_IT ); 
template class LoraWanObjet<LoraRegionsEU>;
template <class T> 
LoraWanObjet <T> ::LoraWanObjet( sLoRaWanKeys LoRaWanKeys ):packet(  LoRaWanKeys ){
    StateLoraWanProcess=LWPSTATE_IDLE;
    packet.MajorBits= LORAWANR1;
    FailSafeTimestamp = RtcGetTimeSecond( );
}; 
template <class T> LoraWanObjet <T> ::~LoraWanObjet() {
};
/************************************************************************************************/
/*                      Public  Methods                                                         */
/************************************************************************************************/


/***********************************************************************************************/
/*    LoraWanProcess Method                                                                    */
/***********************************************************************************************/

template <class T> 
eLoraWan_Process_States LoraWanObjet <T> ::LoraWanProcess( uint8_t* AvailableRxPacket ) {

    *AvailableRxPacket = NO_LORA_RXPACKET_AVAILABLE; //@note AvailableRxPacket should be set to "yes" only in Last state before to return to LWPSTATE_IDLE
//    if ( ( IsJoined ( ) == NOT_JOINED ) && ( RtcGetTimeSecond( ) < packet.RtcNextTimeJoinSecond ) ){
//        DEBUG_PRINTF("TOO SOON TO JOIN time is  %d time target is : %d \n",RtcGetTimeSecond( ), packet.RtcNextTimeJoinSecond);
//        StateLoraWanProcess = LWPSTATE_IDLE ;
//    }        
    
    if ( ( RtcGetTimeSecond( ) - FailSafeTimestamp ) > 120 ) {
        //RadioReset ( ) ;
        StateLoraWanProcess = LWPSTATE_IDLE ;
        DEBUG_MSG ( "ERROR : FAILSAFE EVENT OCCUR \n");
        NVIC_SystemReset();
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
                    //@todo process downlink
                    DEBUG_MSG( "\n" );
                    DEBUG_MSG( "  **************************\n " );
                    DEBUG_MSG( " * Receive a downlink RX1 *\n " );
                    DEBUG_MSG( " **************************\n " );
                    StateLoraWanProcess = LWPSTATE_PROCESSDOWNLINK;
                } else { // So rxtimeout case @note manage error case
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
                packet.RegionSetDataRateDistribution( packet.AdrModeSelect );//@note because datarate Distribution has been changed during join
            }
            if ( ( ValidRxPacket == NWKRXPACKET) || ( ValidRxPacket == USERRX_FOPTSPACKET) ) {
                packet.ParseManagementPacket( );
            }
            packet.UpdateMacLayer();
            *AvailableRxPacket = packet.AvailableRxPacketForUser;
            if ( ( packet.IsFrameToSend == NWKFRAME_TOSEND ) || ( packet.IsFrameToSend == USRFRAME_TORETRANSMIT) ) {// @note ack send during the next tx|| ( packet.IsFrameToSend == USERACK_TOSEND ) ) {
                packet.IsFrameToSend = NOFRAME_TOSEND;
                RtcTargetTimer = RtcGetTimeSecond( ) + randr( 2, 6 ); 
                StateLoraWanProcess = LWPSTATE_TXWAIT;
            } else {
                RadioReset ( ) ; 
                StateLoraWanProcess = LWPSTATE_IDLE;
            }
            ValidRxPacket = NO_MORE_VALID_RX_PACKET;
            break;
    /************************************************************************************/
    /*                              STATE TXWAIT MAC                                    */
    /************************************************************************************/
        case LWPSTATE_TXWAIT:
            DEBUG_MSG(".");
            if ( RtcGetTimeSecond( ) > RtcTargetTimer) {
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
template <class T> 
eLoraWan_Process_States LoraWanObjet <T> ::Join ( void ) {
    if ( StateLoraWanProcess != LWPSTATE_IDLE ) {
        DEBUG_MSG( " ERROR : LP STATE NOT EQUAL TO IDLE \n" );
        return ( LWPSTATE_ERROR );
    }
//    if ( GetIsOtaDevice ( ) == APB_DEVICE ) {
//        DEBUG_MSG( " ERROR : APB DEVICE CAN'T PROCCED A JOIN REQUEST\n" );
//        return ( LWPSTATE_ERROR );
//    }
    FailSafeTimestamp = RtcGetTimeSecond( ) ;
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
template <class T> 
eJoinStatus LoraWanObjet <T> ::IsJoined( void ) {
    eJoinStatus status = NOT_JOINED;
    status = packet.Phy.JoinedStatus;
    return ( status );
}

/**************************************************/
/*          LoraWan  IsJoined  Method             */
/**************************************************/
template <class T> 
void LoraWanObjet <T> ::NewJoin ( void ) {
    packet.Phy.JoinedStatus = NOT_JOINED; 
}
/**************************************************/
/*         LoraWan  SendPayload  Method           */
/**************************************************/
template <class T> 
eLoraWan_Process_States LoraWanObjet <T> ::SendPayload ( uint8_t fPort, const uint8_t* dataIn, const uint8_t sizeIn, uint8_t PacketType ) {
    eStatusLoRaWan status;
    FailSafeTimestamp = RtcGetTimeSecond( ) ;
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
template <class T> 
eStatusLoRaWan LoraWanObjet <T> ::ReceivePayload ( uint8_t* UserRxFport, uint8_t* UserRxPayload, uint8_t* UserRxPayloadSize ) {
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
template <class T>
void LoraWanObjet <T> ::SetDataRateStrategy( eDataRateStrategy adrModeSelect ) {
    packet.AdrModeSelect = adrModeSelect;
    packet.RegionSetDataRateDistribution( adrModeSelect );
};


/**************************************************/
/*         LoraWan  GetDevAddr  Method            */
/**************************************************/
template <class T> 
uint32_t LoraWanObjet <T> ::GetDevAddr ( void ) {
    return(packet.DevAddr);
}

/**************************************************/
/*         LoraWan  GetNextPower  Method          */
/**************************************************/
template <class T>
uint8_t LoraWanObjet <T> ::GetNextPower ( void ) {
    return(packet.MacTxPower);
}

/**************************************************/
/*    LoraWan  GetLorawanProcessState  Method     */
/**************************************************/
template <class T> 
eLoraWan_Process_States LoraWanObjet <T> ::GetLorawanProcessState ( void ) {
    return(StateLoraWanProcess);
}
 
/**************************************************/
/*    LoraWan  RestoreContext  Method     */
/**************************************************/
template <class T> 
void LoraWanObjet <T> ::RestoreContext ( void ) {
    packet.LoadFromFlash ( );
}; 

/***************************************************************************************/
/* NOT yet implemented */
/***************************************************************************************/


/**************************************************/
/*   LoraWan  GetNextMaxPayloadLength  Method     */
/**************************************************/
template <class T>
uint32_t LoraWanObjet <T> ::GetNextMaxPayloadLength ( void ) {// error return during tx send to be replace by get datarate?
    return(0);//@NOTE NOT YET IMPLEMENTED
}


/**************************************************/
/*        LoraWan  GetNextDataRate  Method        */
/**************************************************/
template <class T> 
uint8_t LoraWanObjet <T> ::GetNextDataRate ( void ) { // note return datareate in case of adr
    return(0);//@NOTE NOT YET IMPLEMENTED
}


template <class T> 
 void  LoraWanObjet <T> :: MacFactoryReset ( void ) {
     packet.Phy.JoinedStatus = NOT_JOINED;
     packet.SaveInFlash();
     
 }
/************************************************************************************************/
/*                      Private  Methods                                                        */
/************************************************************************************************/
template <class T> 
void LoraWanObjet <T> ::CopyUserPayload( const uint8_t* dataIn, const uint8_t sizeIn ) {
    memcpy( &packet.Phy.TxPhyPayload[ FHDROFFSET + packet.FoptsTxLengthCurrent ], dataIn, sizeIn );
};
 
template <class T> 
uint8_t LoraWanObjet <T> ::GetStateTimer(void) {
    return (packet.StateTimer);
}

template <class T> 
uint8_t LoraWanObjet <T> ::GetRadioState ( void ) {
    return packet.Phy.GetRadioState( );
};
 
template <class T> 
uint8_t LoraWanObjet <T> ::GetRadioIrqFlag ( void ) {
    return packet.Phy.RegIrqFlag;
};
template <class T>
void LoraWanObjet <T> ::RadioReset ( void ) {
//    packet.Phy.Radio.Reset();
//    wait_ms ( 30 ) ;
}
template <class T>
bool LoraWanObjet <T> ::GetIsOtaDevice (void){
    return packet.otaDevice;
}
template <class T>
uint8_t LoraWanObjet <T> ::GetNbOfReset (void){
    return packet.NbOfReset;
}
