  

/*

______          _ _      ______ _                       
| ___ \        | (_)     | ___ \ |                      
| |_/ /__ _  __| |_  ___ | |_/ / | __ _ _ __   ___ _ __ 
|    // _` |/ _` | |/ _ \|  __/| |/ _` | '_ \ / _ \ '__|
| |\ \ (_| | (_| | | (_) | |   | | (_| | | | |  __/ |   
\_| \_\__,_|\__,_|_|\___/\_|   |_|\__,_|_| |_|\___|_|   
                                                        
                                                  
                                                   
Description       : RadioPlaner objets.  

License           : Revised BSD License, see LICENSE.TXT file include in the project

Maintainer        : Matthieu Verdy - Fabien Holin (SEMTECH)
*/
#include "RadioPlaner.h"
#include "sx1272.h"
#include "sx1276.h"
#include "SX126x.h"
#include "Define.h"
template class RadioPLaner<SX1276>;
template class RadioPLaner<SX1272>;
template class RadioPLaner<SX126x>;
template <class R> RadioPLaner <R>::RadioPLaner( R * RadioUser) {
    mcu.AttachInterruptIn( &RadioPLaner< R >::CallbackIsrRadioPlaner,this); // attach it radio
    Radio = RadioUser;
}; 
template <class R> RadioPLaner<R>::~RadioPLaner( ) {
}
template <class R>
ePlanerInitHookStatus RadioPLaner<R>::InitHook ( uint8_t HookId,  void (* AttachCallBack) (void * ), void * objHook ) {
    if ( HookId > NB_HOOK ) {
        return ( INIT_HOOK_ERROR );
    }
    switch ( HookId ){
        case 0 :
           AttachCallBackHook0 = AttachCallBack ;
           objHook0            = objHook;
           break;
        case 1 :
           AttachCallBackHook1 = AttachCallBack ;
           objHook1            = objHook;
           break;
        case 2 :
           AttachCallBackHook2 = AttachCallBack ;
           objHook2            = objHook;
           break;
        case 3 :
           AttachCallBackHook3 = AttachCallBack ;
           objHook3            = objHook;
           break;
        default :
            return (INIT_HOOK_ERROR);
    }
    return ( INIT_HOOK_OK );
}

template <class R>
ePlanerInitHookStatus  RadioPLaner<R>::GetMyHookId  ( void * objHook, uint8_t * HookId ){
    ePlanerInitHookStatus status = INIT_HOOK_ERROR;
    if ( objHook == objHook0 ) { 
       *HookId = 0;
        status = INIT_HOOK_OK ;
    } else if ( objHook == objHook1 ) { 
       *HookId = 1;
        status = INIT_HOOK_OK ;
    } else if ( objHook == objHook2 ) { 
       *HookId = 2;
        status = INIT_HOOK_OK ;
    } else if ( objHook == objHook3 ) { 
       *HookId = 3;
        status = INIT_HOOK_OK ;
    } 
    return (status);
}



template <class R> 
void RadioPLaner<R>::SendLora( uint8_t HookId, uint32_t EndTime, uint8_t *payload, uint8_t payloadSize, uint8_t SF, eBandWidth BW, uint32_t channel, int8_t power ){
    int HookNum = HookId;  //@tbd it should be a parameter of thge function
    NextBW            [ HookNum ] = BW;
    NextSF            [ HookNum ] = SF;
    NextChannel       [ HookNum ] = channel;
    NextTask          [ HookNum ] = TX_LORA;
    EndTimeTask       [ HookNum ] = EndTime;
    StartTimeTask     [ HookNum ] = 0;
    NextPayload       [ HookNum ] = payload;
    NextPayloadSize   [ HookNum ] = payloadSize;
    NextPower         [ HookNum ] = power;
    CurrentHookToExecute          = HookNum ;
    Radio->SendLora( payload, payloadSize, SF, BW, channel, power );
}

template <class R> 
void RadioPLaner<R>::SendFsk ( uint8_t HookId, uint32_t EndTime, uint8_t *payload, uint8_t payloadSize, uint32_t channel, int8_t power ){
    Radio->SendFsk ( payload, payloadSize, channel, power );
}

template <class R> 
void RadioPLaner<R>::RxLora  (uint8_t HookId,  uint32_t StartTime , eBandWidth BW, uint8_t SF, uint32_t channel, uint16_t TimeOutMs ){
    int HookNum = HookId;  //@tbd it should be a parameter of thge function
    NextBW        [ HookNum ] = BW;
    NextSF        [ HookNum ] = SF;
    NextChannel   [ HookNum ] = channel;
    NextTimeOutMs [ HookNum ] = TimeOutMs;
    NextTask      [ HookNum ] = RX_LORA;
    EndTimeTask   [ HookNum ] = 0;
    StartTimeTask [ HookNum ] = StartTime;
    CurrentHookToExecute      = HookNum ;
    SetAlarm ( StartTime );
}

template <class R> 
void RadioPLaner<R>::RxFsk   (uint8_t HookId, uint32_t StartTime , uint32_t channel, uint16_t timeout ){
    int HookNum = HookId;  //@tbd it should be a parameter of thge function
    NextChannel   [ HookNum ] = channel;
    NextTimeOutMs [ HookNum ] = timeout;
    NextTask      [ HookNum ] = RX_FSK;
    CurrentHookToExecute      = HookNum ;
    SetAlarm ( StartTime );
}


template <class R> 
  IrqFlags_t RadioPLaner<R>::GetStatusPlaner ( uint32_t * IrqTimestampMs, ePlanerStatus *PlanerStatus ){
    *IrqTimestampMs = mcu.RtcGetTimeMs ();
    *PlanerStatus   = PLANER_REQUEST_DONE;
    IrqFlags_t status ;
    switch ( CurrentHookToExecute ) {
        case TX_FSK :  
        case RX_FSK   : 
            status =  ( Radio->GetIrqFlagsLora( ) );
            Radio->ClearIrqFlagsLora( );
            break;
        case TX_LORA :  
        case RX_LORA   : 
            status =  ( Radio->GetIrqFlagsLora( ) );
            Radio->ClearIrqFlagsLora( );
            break;
        case CAD:    
        default : 
            status =  ( Radio->GetIrqFlagsLora( ) );
            Radio->ClearIrqFlagsLora( );
            break;
    }
    return (status);
};



template <class R>     
void RadioPLaner<R>::FetchPayloadLora( uint8_t *payloadSize, uint8_t payload[255], int16_t *snr, int16_t *signalRssi){
    Radio->FetchPayloadLora( payloadSize, payload, snr, signalRssi);
}
template <class R>     
void RadioPLaner<R>::FetchPayloadFsk( uint8_t *payloadSize, uint8_t payload[255], int16_t *snr, int16_t *signalRssi){
  Radio->FetchPayloadFsk( payloadSize, payload, snr, signalRssi);
}


template <class R>     
void RadioPLaner<R>::SetAlarm (uint32_t alarmInMs ) {
    mcu.StartTimerMsecond( &RadioPLaner<R>::CallbackIsrTimerRadioPlaner,this, alarmInMs);
}


/************************************************************************************/
/*                                 Timer Isr Routine                             */
/*                               Called when Alarm expires                          */
/************************************************************************************/

template <class R>     
void RadioPLaner<R>::IsrTimerRadioPlaner( void ) {
    switch  ( NextTask [ CurrentHookToExecute ] ) {
        case TX_LORA : 
            break;
        case TX_FSK  : 
            break;
        case RX_LORA   :
            Radio->RxLora ( NextBW[ CurrentHookToExecute ], NextSF[ CurrentHookToExecute ], NextChannel[ CurrentHookToExecute ], NextTimeOutMs[ CurrentHookToExecute ] );
            break;
        case RX_FSK    :
            Radio->RxFsk(NextChannel[ CurrentHookToExecute ], NextTimeOutMs[ CurrentHookToExecute ]);
            break;
        case CAD :
            break;
        default :
            break;    
    }
}              


/************************************************************************************/
/*                                 Radio Isr Routine                                */
/*                             Called when Radio raises It                          */
/************************************************************************************/
template <class R> 
void  RadioPLaner<R>::IsrRadioPlaner ( void ) {
    IrqTimeStampMs = mcu.RtcGetTimeMs( );
    CallBackHook0 ( );
    Radio->Sleep( false );
} 

