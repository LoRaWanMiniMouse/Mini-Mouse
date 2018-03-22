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
#include "ApiTimers.h"
#include "Define.h"
#include "math.h"
//static RadioEvents_t RadioEvents;



RadioContainer::RadioContainer( )
                :Radio( NULL ) {
    StateRadioProcess = RADIOSTATE_IDLE;
    //TxInterrupt.rise( this,&RadioContainer::IsrRadio );
    //RxTimeoutInterrupt.rise( this,&RadioContainer::IsrRadio );
    TimestampRtcIsr =0;
    TxFrequency = 868100000;
    TxPower = 14;
    TxSf = 7;
    JoinedStatus = JOINED ;
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
     //TxInterrupt.rise( callback ( this, &RadioContainer::IsrRadio ) );
     RadioGlobalIt.rise( callback ( this, &RadioContainer::IsrRadio ) );
     RadioTimeOutGlobalIt.rise( callback ( this , &RadioContainer::IsrRadio ) );
}
void RadioContainer::DetachIsr ( void ) {
     RadioGlobalIt.rise( NULL );
     RadioTimeOutGlobalIt.rise( NULL );
}
void RadioContainer::IsrRadio( void ) {
    int status = OKLORAWAN;
    uint32_t tCurrentMillisec;
    GetIrqRadioFlag ( );
    ClearIrqRadioFlag ( );
    if ( RegIrqFlag == RECEIVE_PACKET_IRQ_FLAG ) {//@ note (important for phy ) remove all IT mask in config send or rx and check if regirqflag = rxdone + header crc valid 
        status = DumpRxPayloadAndMetadata ( );
        Radio.Sleep ( );
        if ( status != OKLORAWAN ) { // Case receive a packet but it isn't a valid packet 
            RegIrqFlag = BAD_PACKET_IRQ_FLAG ; // this case is exactly the same than the case of rx timeout
            tCurrentMillisec =  RtcGetTimeMs( );
            uint32_t timeoutMs = LastTimeRxWindowsMs - tCurrentMillisec ;
            if ( (int)( LastTimeRxWindowsMs - tCurrentMillisec - 5 * SymbolDuration ) > 0 ) {
                if ( RxMod == LORA ) {
                    Radio.SetChannel( RxFrequency);
                    Radio.SetRxConfig( MODEM_LORA, RxBw, RxSf, 1, 0, 8, timeoutMs, false, 0, false, 0, 0, true, false );
                    Radio.Rx(0); 
                } else {
                    Radio.SetChannel( RxFrequency );
                    Radio.SetRxConfig( MODEM_FSK, 50e3, 50e3, 0, 83.333e3, 5, 0, false, 0, true, 0, 0, true, false );
                    Radio.Rx(0); 
                }
            DEBUG_MSG( "Receive a packet But rejected\n");
            DEBUG_PRINTF( "tcurrent %u timeout = %d, end time %u \n ", tCurrentMillisec, timeoutMs, LastTimeRxWindowsMs);
            return;
            }
        }
    } else {
        Radio.Sleep ( );
    }

    //Radio.Sleep ( );
    switch ( StateRadioProcess ) { 
        case RADIOSTATE_TXON :
            TimestampRtcIsr = RtcGetTimeMs ( ); //@info Timestamp only on txdone it
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
 

void RadioContainer::Send(eModulationType TxModulation , uint32_t TxFrequencyMac, uint8_t TxPowerMac, uint8_t TxSfMac, uint32_t TxBwMac, uint16_t TxPayloadSizeMac ) { //@note could/should be merge with tx config
    TxFrequency   = TxFrequencyMac; 
    TxPower       = TxPowerMac;
    TxSf          = TxSfMac;
    TxBw          = TxBwMac;
    TxPayloadSize    = TxPayloadSizeMac ;
    StateRadioProcess = RADIOSTATE_TXON;
    Radio.SetChannel( TxFrequency );
    Radio.Write( REG_LR_SYNCWORD, LORA_MAC_SYNCWORD );
    if ( TxModulation == LORA ) {
        DEBUG_PRINTF ( "  TxFrequency = %d, RxSf = %d , RxBw = %d PayloadSize = %d\n", TxFrequency, TxSf,TxBw, TxPayloadSize) ; 
        Radio.SetTxConfig( MODEM_LORA, TxPower, 0, TxBw, TxSf, 1, 8, false, true, 0, 0, false, 10e3 );
    } else {
        DEBUG_MSG("FSK TRANSMISSION \n");

        Radio.SetTxConfig( MODEM_FSK, TxPower, 25e3, 0, 50e3, 0, 5, false, true, 0, 0, false, 3e6 );
    }
    wait_ms(1);
    Radio.Send( TxPhyPayload, TxPayloadSize );
};

void RadioContainer::SetRxConfig(eModulationType RxModulation ,uint32_t RxFrequencyMac, uint8_t RxSfMac, uint32_t RxBwMac ,uint32_t RxWindowMs) {
    RxFrequency  = RxFrequencyMac;
    RxBw         = RxBwMac;
    RxSf         = RxSfMac;
    RxMod        = RxModulation;
    Radio.SetChannel( RxFrequencyMac );
    int timeoutMs =  RxWindowMs;// @ rx timeot set to x symbols
    if ( RxMod == LORA ) {
        Radio.SetRxConfig( MODEM_LORA, RxBw, RxSf, 1, 0, 8, timeoutMs, false, 0, false, 0, 0, true, false );
    } else {
        Radio.SetRxConfig( MODEM_FSK, 50e3, 50e3, 0, 83.333e3, 5, 0, false, 0, true, 0, 0, true, false );
    }
    DEBUG_PRINTF ( "  RxFrequency = %d, RxSf = %d , RxBw = %d \n", RxFrequency, RxSf,RxBw ) ; 
}

int RadioContainer::GetRadioState( void ) {
    return StateRadioProcess;
};

void RadioContainer::RadioContainerInit( void ) {
    Radio.Write( REG_LR_SYNCWORD, LORA_MAC_SYNCWORD );
    Radio.Sleep( );
};


uint32_t RadioContainer::GetTxFrequency ( void ) {
    return( TxFrequency );
};
//void RadioContainer::SetTxPower ( uint8_t TxPower )
//{
//    TxPower = TxPower;
//};

//void RadioContainer::SetTxSf ( uint8_t TxSf ){
//    TxSf = TxSf;
//};
//void RadioContainer::SetTxBw ( uint8_t TxBw ){
//   TxBw = TxBw;
//};

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
    if (( MtypeRxtmp == JOINREQUEST) || ( MtypeRxtmp == UNCONF_DATA_UP ) || ( MtypeRxtmp == CONF_DATA_UP) || ( MtypeRxtmp == REJOIN_REQUEST )) {
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
