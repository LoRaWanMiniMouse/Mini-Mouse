  

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
   
    TaskType          [ HookId ] = TASK_TX_LORA;
    EndTimeTask       [ HookId ] = EndTime;
    StartTimeTask     [ HookId ] = 0;

    NextBW            [ HookId ] = BW;
    NextSF            [ HookId ] = SF;
    NextChannel       [ HookId ] = channel;
    NextPayload       [ HookId ] = payload;
    NextPayloadSize   [ HookId ] = payloadSize;
    NextPower         [ HookId ] = power;
    CurrentHookToExecute          = HookId ;
    Radio->SendLora( payload, payloadSize, SF, BW, channel, power );
}

template <class R> 
void RadioPLaner<R>::SendFsk ( uint8_t HookId, uint32_t EndTime, uint8_t *payload, uint8_t payloadSize, uint32_t channel, int8_t power ){
    Radio->SendFsk ( payload, payloadSize, channel, power );
}

template <class R> 
void RadioPLaner<R>::RxLora  (uint8_t HookId,  uint32_t StartTime , eBandWidth BW, uint8_t SF, uint32_t channel, uint16_t TimeOutMs ){
   
    TaskType      [ HookId ] = TASK_RX_LORA;
    EndTimeTask   [ HookId ] = 0;
    StartTimeTask [ HookId ] = StartTime;
    NextBW        [ HookId ] = BW;
    NextSF        [ HookId ] = SF;
    NextChannel   [ HookId ] = channel;
    NextTimeOutMs [ HookId ] = TimeOutMs;
    CurrentHookToExecute      = HookId ;
    SetAlarm ( StartTime );
}

template <class R> 
void RadioPLaner<R>::RxFsk   (uint8_t HookId, uint32_t StartTime , uint32_t channel, uint16_t timeout ){
 
    NextChannel   [ HookId ] = channel;
    NextTimeOutMs [ HookId ] = timeout;
    TaskType      [ HookId ] = TASK_RX_FSK;
    CurrentHookToExecute      = HookId ;
    SetAlarm ( StartTime );
}


template <class R> 
  IrqFlags_t RadioPLaner<R>::GetStatusPlaner ( uint32_t * IrqTimestampMs, ePlanerStatus *PlanerStatus ){
    *IrqTimestampMs = mcu.RtcGetTimeMs ();
    *PlanerStatus   = PLANER_REQUEST_DONE;
    IrqFlags_t status ;
    switch ( CurrentHookToExecute ) {
        case TASK_TX_FSK :  
        case TASK_RX_FSK   : 
            status =  ( Radio->GetIrqFlagsLora( ) );
            Radio->ClearIrqFlagsLora( );
            break;
        case TASK_TX_LORA :  
        case TASK_RX_LORA   : 
            status =  ( Radio->GetIrqFlagsLora( ) );
            Radio->ClearIrqFlagsLora( );
            break;
        case TASK_CAD:    
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
    PlanerTimerState = TIMER_BUSY ;
    mcu.StartTimerMsecond( &RadioPLaner<R>::CallbackIsrTimerRadioPlaner,this, alarmInMs);
}


/************************************************************************************/
/*                                 Timer Isr Routine                             */
/*                               Called when Alarm expires                          */
/************************************************************************************/

template <class R>     
void RadioPLaner<R>::IsrTimerRadioPlaner( void ) {
    PlanerTimerState = TIMER_IDLE ;
    switch  ( TaskType [ CurrentHookToExecute ] ) {
        case TASK_TX_LORA : 
            PlanerRadioState = RADIO_TX ;
            break;
        case TASK_TX_FSK  :
            PlanerRadioState = RADIO_TX ; 
            break;
        case TASK_RX_LORA   :
            Radio->RxLora ( NextBW[ CurrentHookToExecute ], NextSF[ CurrentHookToExecute ], NextChannel[ CurrentHookToExecute ], NextTimeOutMs[ CurrentHookToExecute ] );
            PlanerRadioState = RADIO_RX ;
            break;
        case TASK_RX_FSK    :
            Radio->RxFsk(NextChannel[ CurrentHookToExecute ], NextTimeOutMs[ CurrentHookToExecute ]);
            PlanerRadioState = RADIO_RX ;
            break;
        case TASK_CAD :
            PlanerRadioState = RADIO_CAD ;
            break;
        default :
            PlanerRadioState = RADIO_IDLE;
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
    PlanerRadioState = RADIO_IDLE;
} 

