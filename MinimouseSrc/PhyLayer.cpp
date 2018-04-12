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
#include "sx1276.h"
#include "sx1276Regs-LoRa.h"
#include "MacLayer.h"
#include "LoraWanProcess.h"
#include "Define.h"
#include "UserDefine.h"

template class RadioContainer<SX1276>;
template class RadioContainer<SX126x>;

template <class R> RadioContainer <R>::RadioContainer( R * RadioUser ){
    StateRadioProcess = RADIOSTATE_IDLE;
    TimestampRtcIsr =0;
    TxFrequency = 868100000;
    TxPower = 14;
    TxSf = 7;
    Radio = RadioUser;
}; 
template <class R> RadioContainer<R>::~RadioContainer( ) {
};

/*******************Isr Radio  ***************************************/
/*          Timestamp isr with RTC                                   */
/*          Read IrqFlags Register /clear Irq flag                   */
/*          Update the StateRadioProcess                             */
/*          Cpy data +meta data in case of reception & crc ok        */
/*          Set Radio in Sleep Mode                                  */
/*******************Isr Radio  ***************************************/
template <class R> void RadioContainer <R>::AttachIsr ( void ) {
    
     RadioGlobalIt.rise( callback ( this, &RadioContainer::IsrRadio ) );
     RadioTimeOutGlobalIt.rise( callback ( this , &RadioContainer::IsrRadio ) );
}
template <class R> void RadioContainer <R>::DetachIsr ( void ) {
     RadioGlobalIt.rise( NULL );
     RadioTimeOutGlobalIt.rise( NULL );
}

/************************************************************************************************/
/*                      Public  Methods                                                         */
/************************************************************************************************/
 //@note Partionning Public/private not yet finalized
 

template <class R> void RadioContainer <R>::Send(eModulationType TxModulation , uint32_t TxFrequencyMac, uint8_t TxPowerMac, uint8_t TxSfMac, eBandWidth TxBwMac, uint16_t TxPayloadSizeMac ) { //@note could/should be merge with tx config
    TxFrequency       = TxFrequencyMac; 
    TxPower           = TxPowerMac;
    TxSf              = TxSfMac;
    TxBw              = TxBwMac;
    TxPayloadSize     = TxPayloadSizeMac ;
    StateRadioProcess = RADIOSTATE_TXON;
    Radio->Reset( ); 
    if ( TxModulation == LORA ) {
        DEBUG_PRINTF ( "  TxFrequency = %d, RxSf = %d , RxBw = %d PayloadSize = %d\n", TxFrequency, TxSf,TxBw, TxPayloadSize) ; 
        Radio->SendLora( TxPhyPayload, TxPayloadSize, TxSf, TxBw, TxFrequency, TxPower );
    } else {
        DEBUG_MSG("FSK TRANSMISSION \n");
        //@TODO FSK
    }
    wait_ms(1);
};

template <class R> void RadioContainer <R>::SetRxConfig(eModulationType RxModulation ,uint32_t RxFrequencyMac, uint8_t RxSfMac, eBandWidth RxBwMac ,uint32_t RxWindowMs) {
    RxFrequency  = RxFrequencyMac;
    RxBw         = RxBwMac;
    RxSf         = RxSfMac;
    RxMod        = RxModulation;
    if ( RxModulation == LORA ) {
        Radio->RxLora( RxBw, RxSf, RxFrequency, RxWindowMs );
    } else {
        // @TODO: FSK
        // Radio.SetRxConfig( MODEM_FSK, 50e3, 50e3, 0, 83.333e3, 5, 0, false, 0, true, 0, 0, true, false );//@note rxtimeout 400ms!!!! // @TODO: Implementation
    }
    DEBUG_PRINTF ( "  RxFrequency = %d, RxSf = %d , RxBw = %d \n", RxFrequency, RxSf,RxBw );
}

template <class R>int RadioContainer<R>::GetRadioState( void ) {
    return StateRadioProcess;
};


template <class R> uint32_t RadioContainer<R>::GetTxFrequency ( void ) {
    return( TxFrequency );
};


/************************************************************************************************/
/*                      Private  Methods                                                         */
/************************************************************************************************/



template <class R> int RadioContainer<R>::DumpRxPayloadAndMetadata ( void ) {
    int16_t snr;
    int16_t rssi;
    Radio->FetchPayload( &RxPhyPayloadSize, RxPhyPayload, &snr, &rssi );
    RxPhyPayloadSnr = (int) snr;
    RxPhyPayloadRssi= (int) rssi;
   /* check Mtype */
    int status = OKLORAWAN;
    uint8_t MtypeRxtmp = RxPhyPayload[0] >> 5 ;
    if (( MtypeRxtmp == JOINREQUEST) || ( MtypeRxtmp == UNCONF_DATA_UP ) || ( MtypeRxtmp == CONF_DATA_UP) || ( MtypeRxtmp == REJOIN_REQUEST )) {
        status += ERRORLORAWAN;
        DEBUG_PRINTF(" BAD Mtype = %d for RX Frame \n", MtypeRxtmp );
    }
    /* check devaddr */
    if ( JoinedStatus == JOINED ){
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
