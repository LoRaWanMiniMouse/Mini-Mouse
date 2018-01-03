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
#include "ApiRtc.h"
#include "utilities.h"
#include "Define.h"
Serial pcf( SERIAL_TX, SERIAL_RX );

LoraWanObjet::LoraWanObjet( PinName interrupt ):packet( interrupt ){
    StateLoraWanProcess=LWPSTATE_IDLE;
    packet.MajorBits= LORAWANR1;
}; 
LoraWanObjet::~LoraWanObjet() {
};
/************************************************************************************************/
/*                      Public  Methods                                                         */
/************************************************************************************************/


/***********************************************************************************************/
/*    LoraWanProcess Method                                                                    */
/***********************************************************************************************/

 eLoraWan_Process_States LoraWanObjet::LoraWanProcess( uint8_t* AvailableRxPacket ) {
 
    *AvailableRxPacket = NOLORARXPACKETAVAILABLE; //@note AvailableRxPacket should be set to "yes" only in Last state before to return to LWPSTATE_IDLE*
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
                    packet.ConfigureRadioAndSend( );
                    pcf.printf( " **************************\n " );
                    pcf.printf( " *       Send Payload     *\n " );
                    pcf.printf( " **************************\n " );
                    break ; 
            
                case RADIOSTATE_TXFINISHED : 
                    packet.ConfigureRadioForRx1 ( );
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
                if ( GetRadioIrqFlag ( ) == RECEIVEPACKETIRQFLAG) {
                    //@todo process downlink
                    pcf.printf( "  **************************\n " );
                    pcf.printf( " * Receive a downlink RX1 *\n " );
                    pcf.printf( " **************************\n " );
                    StateLoraWanProcess = LWPSTATE_PROCESSDOWNLINK;
                } else { // So rxtimeout case @note manage error case
                    pcf.printf( "  **************************\n " );
                    pcf.printf( " *      RX1 Timeout       *\n " );
                    pcf.printf( " **************************\n " );
                    packet.ConfigureRadioForRx2 ( );
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
                 if ( GetRadioIrqFlag ( ) == RECEIVEPACKETIRQFLAG) {
                    pcf.printf( "  **************************\n " );
                    pcf.printf( " * Receive a downlink RX2 *\n " );
                    pcf.printf( " **************************\n " );
                    StateLoraWanProcess = LWPSTATE_PROCESSDOWNLINK; //@note to be discuss if it is necessary to create a dedicated state?
                } else {
                    pcf.printf( "  **************************\n " );
                    pcf.printf( " *      RX2 Timeout       *\n " );
                    pcf.printf( " **************************\n " );
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
            pcf.printf( " **************************\n " );
            pcf.printf( " * Process Downlink       *\n " );
            pcf.printf( " **************************\n " );
            ValidRxPacket = packet.DecodeRxFrame( ); // return NOVALIDRXPACKET or  USERRXPACKET or NWKRXPACKET or JOINACCEPTPACKET.
            StateLoraWanProcess = LWPSTATE_UPDATEMAC;
            break;
    /************************************************************************************/
    /*                              STATE UPDATE MAC                                    */
    /************************************************************************************/
        case LWPSTATE_UPDATEMAC:
            packet.Phy.StateRadioProcess = RADIOSTATE_IDLE;  
            pcf.printf( " **************************\n " );
            pcf.printf( " *       UpdateMac        *\n " );
            pcf.printf( " **************************\n " );
            if ( ValidRxPacket == JOINACCEPTPACKET){
                packet.UpdateJoinProcedure( );
            }
            if ( ( ValidRxPacket == NWKRXPACKET) || ( ValidRxPacket == USERRX_FOPTSPACKET) ) {
                packet.ParseManagementPacket( );
            }
            packet.UpdateMacLayer();
            *AvailableRxPacket = packet.AvailableRxPacketForUser;
            if ( ( packet.IsFrameToSend == NWKFRAME_TOSEND ) ) {// @note ack send during the next tx|| ( packet.IsFrameToSend == USERACK_TOSEND ) ) {
                packet.IsFrameToSend = NOFRAME_TOSEND;
                RtcTargetTimer = RtcGetTimeSecond( ) + randr( 1, 2 ); //@note RtcGetTime in s so no wrap before 136 year since 1970 discuss wait between 5s and 25s
                StateLoraWanProcess = LWPSTATE_TXWAIT;
            } else {
                RadioReset ( ) ; 
                StateLoraWanProcess = LWPSTATE_IDLE;
            }
            ValidRxPacket = NOMOREVALIDRXPACKET;
            break;
    /************************************************************************************/
    /*                              STATE TXWAIT MAC                                    */
    /************************************************************************************/
        case LWPSTATE_TXWAIT:
            pcf.printf( " **************************\n " );
            pcf.printf( " *       TXWAIT MAC       *\n " );
            pcf.printf( " **************************\n " );
            if ( RtcGetTimeSecond( ) > RtcTargetTimer) {
                StateLoraWanProcess = LWPSTATE_SEND; //@note the frame have already been prepare in Upadate Mac Layer
            }
            break;

        default: 
            pcf.printf( " Illegal state\n " );
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
eLoraWan_Process_States LoraWanObjet::Join ( void ) {
    packet.Phy.JoinedStatus = NOTJOINED;
    packet.BuildJoinLoraFrame( );
    packet.MacRx1Delay = RX1DELAYJOIN;
    packet.MacTxSf = 12;
    packet.MacRx2Sf = 12;
    StateLoraWanProcess = LWPSTATE_SEND;
    return( StateLoraWanProcess );
};


/**************************************************/
/*          LoraWan  IsJoined  Method             */
/**************************************************/
eJoinStatus  LoraWanObjet::IsJoined( void ) {
    eJoinStatus status = NOTJOINED;
    status = packet.Phy.JoinedStatus;
    return ( status );
}

/**************************************************/
/*         LoraWan  SendPayload  Method           */
/**************************************************/
eLoraWan_Process_States LoraWanObjet::SendPayload ( uint8_t fPort, const uint8_t* dataIn, const uint16_t sizeIn, uint8_t PacketType ) {
    
    if ( StateLoraWanProcess != LWPSTATE_IDLE ) {
        return ( LWPSTATE_ERROR );
    }
    RadioReset ( ) ; 
    CopyUserPayload( dataIn,sizeIn );
    packet.UserPayloadSize = sizeIn;
    packet.fPort = fPort;
    packet.MType = PacketType;
    packet.BuildTxLoraFrame( );
    packet.EncryptTxFrame( );
    StateLoraWanProcess = LWPSTATE_SEND;
    return( StateLoraWanProcess );
};

/**************************************************/
/*        LoraWan  Receive  Method                */
/**************************************************/
uint8_t LoraWanObjet::ReceivePayload ( uint8_t* UserRxFport, uint8_t* UserRxPayload, uint8_t* UserRxPayloadSize ) {
    int status = OKLORAWAN; 
    if (packet.AvailableRxPacketForUser == NOLORARXPACKETAVAILABLE) {
        status = ERRORLORAWAN ;
    } else {
        *UserRxPayloadSize = packet.MacRxPayloadSize;
        *UserRxFport = packet.FportRx;
        memcpy( UserRxPayload, &packet.MacRxPayload[0], packet.MacRxPayloadSize);
        packet.AvailableRxPacketForUser = NOLORARXPACKETAVAILABLE ;
    }
    return( status );
};

/**************************************************/
/*       LoraWan  AdrModeSelect  Method           */
/**************************************************/
void LoraWanObjet::SetDataRateStrategy( eDataRateStrategy adrModeSelect ) {
    packet.AdrModeSelect = adrModeSelect;
};
/**************************************************/
/*        LoraWan  TryToJoin  Method              */
/**************************************************/
uint8_t LoraWanObjet::TryToJoin ( void ) {
    //@NOTE NOT YET IMPLEMENTED
}

/**************************************************/
/*   LoraWan  GetNextMaxPayloadLength  Method     */
/**************************************************/
uint32_t LoraWanObjet::GetNextMaxPayloadLength ( void ) {
    //@NOTE NOT YET IMPLEMENTED
}

/**************************************************/
/*         LoraWan  GetDevAddr  Method            */
/**************************************************/
uint32_t LoraWanObjet::GetDevAddr ( void ) {
    //@NOTE NOT YET IMPLEMENTED
}

/**************************************************/
/*         LoraWan  GetNextPower  Method          */
/**************************************************/
uint8_t LoraWanObjet::GetNextPower ( void ) {
    //@NOTE NOT YET IMPLEMENTED
}

/**************************************************/
/*        LoraWan  GetNextDataRate  Method        */
/**************************************************/
uint8_t LoraWanObjet::GetNextDataRate ( void ) {
    //@NOTE NOT YET IMPLEMENTED
}

/**************************************************/
/*    LoraWan  GetLorawanProcessState  Method     */
/**************************************************/
uint8_t LoraWanObjet::GetLorawanProcessState ( void ) {
    //@NOTE NOT YET IMPLEMENTED
}
 
/**************************************************/
/*    LoraWan  RestoreContext  Method     */
/**************************************************/
void LoraWanObjet::RestoreContext ( void ) {
    packet.LoadFromFlash ( );
}; 

/************************************************************************************************/
/*                      Private  Methods                                                        */
/************************************************************************************************/
 void LoraWanObjet::CopyUserPayload( const uint8_t* dataIn, const uint16_t sizeIn ) {
    memcpy( &packet.Phy.TxPhyPayload[FHDROFFSET], dataIn, sizeIn );
 };
 
 uint8_t LoraWanObjet::GetStateTimer(void) {
     return (packet.StateTimer);
 }
 
uint8_t LoraWanObjet::GetRadioState ( void ) {
    return packet.Phy.GetRadioState( );
};
 
uint8_t LoraWanObjet::GetRadioIrqFlag ( void ) {
    return packet.Phy.RegIrqFlag;
};
void LoraWanObjet::RadioReset ( void ) {
    //NOT YET IMPLEMENTED
}
