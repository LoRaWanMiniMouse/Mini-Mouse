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
#include "PhyLayer.h"
#include "sx1276-hal.h"
#include "sx1276Regs-LoRa.h"
#include "MacLayer.h"
#include "LoraWanProcess.h"
#include "ApiRtc.h"
#include "Define.h"
#include "math.h"
//static RadioEvents_t RadioEvents;



RadioContainer::RadioContainer( PinName interrupt )
                :Radio( NULL ), TxInterrupt( interrupt ), RxTimeoutInterrupt ( RX_TIMEOUT_IT ) {
    StateRadioProcess = RADIOSTATE_IDLE;
    //TxInterrupt.rise( this,&RadioContainer::IsrRadio );
    RxTimeoutInterrupt.rise( this,&RadioContainer::IsrRadio );
    TimestampRtcIsr =0;
    TxFrequency = 868100000;
    TxPower = 14;
    TxSf = 7;
    JoinedStatus = NOTJOINED ;
}; 
RadioContainer::~RadioContainer( ) {
};

/*******************Isr Radio  ***************************************/
/*          Timestamp isr with RTC                                   */
/*          Read IrqFlags Register /clear Irq flag                   */
/*          Update the StateRadioProcess                             */
/*          Cpy data +meta data in case of reception & crc ok        */
/*          Set Radio in Sleep Mode                                  */
/*******************Isr Radio  ***************************************/
void RadioContainer::AttachIsr ( void ) {
     TxInterrupt.rise( this,&RadioContainer::IsrRadio );
}
void RadioContainer::DetachIsr ( void ) {
     TxInterrupt.rise( NULL );
}
void RadioContainer::IsrRadio( void ) {
    uint64_t tAlarm64bits ;
    int status = OKLORAWAN;
    GetIrqRadioFlag ( );
    ClearIrqRadioFlag ( );
    if ( RegIrqFlag == RECEIVEPACKETIRQFLAG ) {//@ note (important for phy ) remove all IT mask in config send or rx and check if regirqflag = rxdone + header crc valid 
        status = DumpRxPayloadAndMetadata ( );
        if ( status != OKLORAWAN ) { // Case receive a packet but it isn't a valid packet 
            RegIrqFlag = BADPACKETIRQFLAG ; // this case is exactly the same than the case of rx timeout
            DEBUG_MSG( "Receive a packet But rejected\n"); 
        }
    }
    Radio.Sleep ( );
    switch ( StateRadioProcess ) { 
        case RADIOSTATE_TXON :
            TimestampRtcIsr = RtcGetTimeMs ( &tAlarm64bits ); //@info Timestamp only on txdone it
            StateRadioProcess = RADIOSTATE_TXFINISHED;
            break;
        
        case RADIOSTATE_TXFINISHED :
            StateRadioProcess = RADIOSTATE_RX1FINISHED;
            break;
        
       case RADIOSTATE_RX1FINISHED :
            StateRadioProcess = RADIOSTATE_IDLE;
            break;
        
        default :
            DEBUG_MSG ("receive It radio error\n");
            break;
    }
};


/************************************************************************************************/
/*                      Public  Methods                                                         */
/************************************************************************************************/
 //@note Partionning Public/private not yet finalized
 
void RadioContainer::SetTxConfig( void ) {
    StateRadioProcess = RADIOSTATE_TXON;
    Radio.SetChannel( TxFrequency );
    Radio.Write( REG_LR_SYNCWORD, LORA_MAC_SYNCWORD );
    Radio.SetTxConfig( MODEM_LORA, TxPower, 0, TxBw, TxSf, 1, 8, false, true, 0, 0, false, 10e3 );
};
void RadioContainer::Send( ) { //@note could/should be merge with tx config
    Radio.Send( TxPhyPayload, TxPayloadSize );
};

void RadioContainer::SetRxConfig( void ) {
    Radio.SetChannel( RxFrequency );
    int nbSymbtimeout =  18;// @ note check the real signification of this timeout 
    Radio.SetRxConfig( MODEM_LORA, RxBw, RxSf, 1, 0, 8, nbSymbtimeout, false, 0, false, 0, 0, true, false );//@note rxtimeout 400ms!!!!
}

int RadioContainer::GetRadioState( void ) {
    return StateRadioProcess;
};

void RadioContainer::RadioContainerInit( void ) {
    Radio.Write( REG_LR_SYNCWORD, LORA_MAC_SYNCWORD );
    Radio.Sleep( );
}

/************************************************************************************************/
/*                      Private  Methods                                                         */
/************************************************************************************************/



int RadioContainer::DumpRxPayloadAndMetadata ( void ) {
    RxPhyPayloadSize = Radio.Read( REG_LR_RXNBBYTES );
    Radio.ReadFifo( RxPhyPayload, RxPhyPayloadSize );
    RxPhyPayloadSnr = Radio.Read( REG_LR_PKTSNRVALUE );
    RxPhyPayloadRssi = Radio.Read( REG_LR_PKTRSSIVALUE );
   /* check Mtype */
    int status = OKLORAWAN;
    uint8_t MtypeRxtmp = RxPhyPayload[0] >> 5 ;
    if (( MtypeRxtmp == JOINREQUEST) || ( MtypeRxtmp == UNCONFDATAUP ) || ( MtypeRxtmp == CONFDATAUP) || ( MtypeRxtmp == REJOINREQUEST )) {
        status += ERRORLORAWAN;
        DEBUG_PRINTF(" BAD Mtype = %d for RX Frame \n", MtypeRxtmp );
    }
    /* check devaddr */
    if ( JoinedStatus == JOINED ) {
            
        uint32_t DevAddrtmp = RxPhyPayload[1] + ( RxPhyPayload[2] << 8 ) + ( RxPhyPayload[3] << 16 )+ ( RxPhyPayload[4] << 24 );
        if ( DevAddrtmp != DevAddrIsr ) {
            status += ERRORLORAWAN;
            DEBUG_PRINTF( " BAD DevAddr = %x for RX Frame \n", DevAddrtmp );
        }
        if ( status != OKLORAWAN ) {
            RxPhyPayloadSize = 0;
        }
    }
    return (status);
}
void  RadioContainer::ClearIrqRadioFlag ( void ) {
    Radio.Write( REG_LR_IRQFLAGS, 0 );
}
void  RadioContainer::GetIrqRadioFlag ( void ) {
    RegIrqFlag = Radio.Read ( REG_LR_IRQFLAGS );
}
