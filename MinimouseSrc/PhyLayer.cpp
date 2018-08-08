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
#include "ApiMcu.h"
#include "UserDefine.h"
#include "utilities.h"
#define FileId 3
template class RadioContainer<SX1276>;
template class RadioContainer<SX1272>;
template class RadioContainer<SX126x>;

template <class R> RadioContainer <R>::RadioContainer( R * RadioUser ){
    StateRadioProcess = RADIOSTATE_IDLE;
    TimestampRtcIsr =0;
    TxFrequency = 868100000;
    TxPower = 14;
    TxSf = 7;
    Radio = RadioUser;
    LastItTimeFailsafe = mcu.RtcGetTimeSecond( );
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
    InsertTrace ( __COUNTER__, FileId );    
   //  RadioGlobalIt.rise( callback ( this, &RadioContainer::IsrRadio ) );
    mcu.AttachInterruptIn( &RadioContainer< R >::CallbackIsrRadio,this);
}
template <class R> void RadioContainer <R>::DetachIsr ( void ) {
    InsertTrace ( __COUNTER__, FileId );
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
    TxPayloadSize     = TxPayloadSizeMac;
    StateRadioProcess = RADIOSTATE_TXON;
    CurrentMod        = TxModulation;
    Radio->Reset( ); 
    if ( TxModulation == LORA ) {
        InsertTrace ( __COUNTER__, FileId );
        DEBUG_PRINTF ( "  TxFrequency = %d, RxSf = %d , RxBw = %d PayloadSize = %d\n", TxFrequency, TxSf,TxBw, TxPayloadSize) ; 
        Radio->SendLora( TxPhyPayload, TxPayloadSize, TxSf, TxBw, TxFrequency, TxPower );
    } else {
        InsertTrace ( __COUNTER__, FileId );
        DEBUG_MSG("FSK TRANSMISSION \n");
        Radio->SendFsk( TxPhyPayload, TxPayloadSize, TxFrequency, TxPower );
    }
    mcu.mwait_ms(1);
};

template <class R> void RadioContainer <R>::SetRxConfig(eModulationType RxModulation ,uint32_t RxFrequencyMac, uint8_t RxSfMac, eBandWidth RxBwMac ,uint32_t RxWindowMs) {
    RxFrequency  = RxFrequencyMac;
    RxBw         = RxBwMac;
    RxSf         = RxSfMac;
    RxMod        = RxModulation;
    CurrentMod   = RxModulation;
    if ( RxModulation == LORA ) {
        InsertTrace ( __COUNTER__, FileId );
        Radio->RxLora( RxBw, RxSf, RxFrequency, RxWindowMs );
        DEBUG_PRINTF ( "  RxFrequency = %d, RxSf = %d , RxBw = %d \n", RxFrequency, RxSf,RxBw );
    } else {
        InsertTrace ( __COUNTER__, FileId );
        Radio->RxFsk( RxFrequency, RxWindowMs );
        DEBUG_PRINTF ( "  RxFrequency = %d, FSK \n", RxFrequency );
    }
}

template <class R>int RadioContainer<R>::GetRadioState( void ) {
    InsertTrace ( __COUNTER__, FileId );
    return StateRadioProcess;
};


template <class R> uint32_t RadioContainer<R>::GetTxFrequency ( void ) {
    InsertTrace ( __COUNTER__, FileId );
    return( TxFrequency );
};


/************************************************************************************************/
/*                      Private  Methods                                                         */
/************************************************************************************************/
/********************************************************/
/*               Check is valid devaddr                 */
/********************************************************/
template <class R> eValidDevAddr RadioContainer<R>::CheckDevAddr (uint32_t devAddrToTest){

    if ( devAddrToTest == DevAddrIsr ) {
        return VALID_DEV_ADDR_UNICAST;
    }
    if (( devAddrToTest == 	DevAddrClassCG0Isr ) && ( ClassCG0EnableIsr ==CLASS_CG0_ENABLE )){
        return VALID_DEV_ADDR_MULTI_CAST_G0;
    }
    if (( devAddrToTest == 	DevAddrClassCG1Isr ) && ( ClassCG0EnableIsr ==CLASS_CG1_ENABLE )){
        return VALID_DEV_ADDR_MULTI_CAST_G1;
    }
    return(UNVALID_DEV_ADDR);
}


template <class R> int RadioContainer<R>::DumpRxPayloadAndMetadata ( void ) {
    int16_t snr;
    int16_t rssi;
    if( CurrentMod == LORA ) {
        Radio->FetchPayloadLora( &RxPhyPayloadSize, RxPhyPayload, &snr, &rssi );
    } else {
        Radio->FetchPayloadFsk( &RxPhyPayloadSize, RxPhyPayload, &snr, &rssi );
    }
    RxPhyPayloadSnr = (int) snr;
    RxPhyPayloadRssi= (int) rssi;
   /* check Mtype */
    int status = OKLORAWAN;
    InsertTrace ( __COUNTER__, FileId );	
    uint8_t MtypeRxtmp = RxPhyPayload[0] >> 5 ;
    if (( MtypeRxtmp == JOINREQUEST) || ( MtypeRxtmp == UNCONF_DATA_UP ) || ( MtypeRxtmp == CONF_DATA_UP) || ( MtypeRxtmp == REJOIN_REQUEST )) {
        status += ERRORLORAWAN;
        InsertTrace ( __COUNTER__, FileId );
        DEBUG_PRINTF(" BAD Mtype = %d for RX Frame \n", MtypeRxtmp );
    }
    /* check devaddr */
    if ( JoinedStatus == JOINED ){
        uint32_t DevAddrtmp = RxPhyPayload[1] + ( RxPhyPayload[2] << 8 ) + ( RxPhyPayload[3] << 16 )+ ( RxPhyPayload[4] << 24 );
        CurrentDevaddrType = CheckDevAddr ( DevAddrtmp );
        if ( CurrentDevaddrType == UNVALID_DEV_ADDR ) {
            status += ERRORLORAWAN;
            InsertTrace ( __COUNTER__, FileId );
            DEBUG_PRINTF( " BAD DevAddr = %x for RX Frame \n", DevAddrtmp );
        }
        if ( status != OKLORAWAN ) {
            RxPhyPayloadSize = 0;
            InsertTrace ( __COUNTER__, FileId );
        }
    }
    if (status == OKLORAWAN) {
        IsReceiveOnRXC = (StateRadioProcess == RADIOSTATE_RXC) ? RECEIVE_ON_RXC : NOT_RECEIVE_ON_RXC ;
    }
    return (status);
}
