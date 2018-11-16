  

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
    for ( int i = 0 ; i < NB_HOOK; i++ ) { 
        TaskType      [ i ] = TASK_IDLE;
        StartTimeTask [ i ] = 0;
        EndTimeTask   [ i ] = 0;
    }
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
void RadioPLaner<R>::SendLora( uint8_t HookId, eTimingTypeTask TaskTimingType ,uint32_t StartTime, uint32_t EndTime, uint8_t *payload, uint8_t payloadSize, SRadioParam sRadioParam){
   
    TaskType          [ HookId ] = TASK_TX_LORA;
    EndTimeTask       [ HookId ] = mcu.RtcGetTimeMs() + EndTime;
    StartTimeTask     [ HookId ] = mcu.RtcGetTimeMs() + 0;

    Bw            [ HookId ] = sRadioParam.Bw;
    Sf            [ HookId ] = sRadioParam.Sf;
    Channel       [ HookId ] = sRadioParam.Frequency;
    Power         [ HookId ] = sRadioParam.Power;
    Payload       [ HookId ] = payload;
    PayloadSize   [ HookId ] = payloadSize;
    CallPlanerArbitrer ( );
  
}

template <class R> 
void RadioPLaner<R>::SendFsk ( uint8_t HookId, eTimingTypeTask TaskTimingType ,uint32_t StartTime, uint32_t EndTime, uint8_t *payload, uint8_t payloadSize, SRadioParam sRadioParam ){

    TaskType          [ HookId ] = TASK_TX_FSK;
    EndTimeTask       [ HookId ] = mcu.RtcGetTimeMs() + EndTime;
    StartTimeTask     [ HookId ] = mcu.RtcGetTimeMs() + 0;

    Channel       [ HookId ] = sRadioParam.Frequency;
    Power         [ HookId ] = sRadioParam.Power;
    Payload       [ HookId ] = payload;
    PayloadSize   [ HookId ] = payloadSize;

    CallPlanerArbitrer (  );
}

template <class R> 
void RadioPLaner<R>::RxLora  (uint8_t HookId,  uint32_t StartTime , eBandWidth BW, uint8_t SF, uint32_t channel, uint16_t TimeOutMsec ){
   
    TaskType      [ HookId ] = TASK_RX_LORA;
    EndTimeTask   [ HookId ] = mcu.RtcGetTimeMs() + 0;
    StartTimeTask [ HookId ] = mcu.RtcGetTimeMs() + StartTime;
    Bw        [ HookId ] = BW;
    Sf        [ HookId ] = SF;
    Channel   [ HookId ] = channel;
    TimeOutMs [ HookId ] = TimeOutMsec;
    CallPlanerArbitrer ( );
   
}

template <class R> 
void RadioPLaner<R>::RxFsk   (uint8_t HookId, uint32_t StartTime , uint32_t channel, uint16_t TimeOutMsec ){
    TaskType      [ HookId ] = TASK_RX_FSK;
    EndTimeTask   [ HookId ] = mcu.RtcGetTimeMs() + 0;
    StartTimeTask [ HookId ] = mcu.RtcGetTimeMs() + StartTime;

    Channel   [ HookId ] = channel;
    TimeOutMs [ HookId ] = TimeOutMsec;
    TaskType  [ HookId ] = TASK_RX_FSK;
    CallPlanerArbitrer (  );
   
}
template <class R> 
 void RadioPLaner<R> :: ComputePlanerStatus (void ) {
        IrqFlags_t status  = ( Radio->GetIrqFlagsLora( ) );
        Radio->ClearIrqFlagsLora( );
        switch (status) {
            case SENT_PACKET_IRQ_FLAG    :
                RadioPlanerStatus = PLANER_TX_DONE ;  
                break;
            case RECEIVE_PACKET_IRQ_FLAG : 
                RadioPlanerStatus = PLANER_RX_PACKET;
                break; 
            case RXTIMEOUT_IRQ_FLAG      : 
                RadioPlanerStatus = PLANER_RX_TIMEOUT;
                break;
            default:
                if (RadioPlanerState == RADIO_IN_TX ) {
                    RadioPlanerStatus = PLANER_TX_CANCELED ;
                } else {
                    RadioPlanerStatus = PLANER_RX_CANCELED ;
                }
                break;
        }
}
 
template <class R> 
  void RadioPLaner<R>::GetStatusPlaner ( uint32_t * IrqTimestampMs, ePlanerStatus *PlanerStatus ){
    *IrqTimestampMs = IrqTimeStampMs;
    *PlanerStatus   = RadioPlanerStatus;
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
/*                                 Planer Arbiter                                   */
/*                                                                                  */
/************************************************************************************/

template <class R> 
void  RadioPLaner<R>::CallPlanerArbitrer ( void ){
       // @ tbd done arbiter task in our case only one hook so always elected
       // probably abort task if it is amandatory 
       // callback to aborted task 
       //blabla
       // updatez time in scheduler
       int i;
       int j; 
       for ( i = TASK_IDLE ; i >= TASK_RX_LORA ; i-- ) { // Task Priority
           for ( j = ( NB_HOOK - 1 ) ; j >= HOOK_0 ; j-- ) { // Hook priority
               if ( TaskType [ j ] == i ) {
                   //CurrentHookToExecute  = (uint8_t) j ;//HOOKID0 
               }
           }
       
       }
    CurrentHookToExecute  = HOOK_0; 
    LaunchTask ( );
    
}

template <class R> 
void  RadioPLaner<R>::LaunchTask ( void ){
    uint8_t Id = CurrentHookToExecute;

    switch ( TaskType [ Id ] ) {
        case TASK_IDLE    :
            break;
        case TASK_TX_LORA :
            Radio->SendLora( Payload[Id], PayloadSize[Id], Sf[Id], Bw[Id], Channel[Id], Power[Id] );
            break;
        case TASK_RX_LORA :
            SetAlarm ( StartTimeTask [Id] - mcu.RtcGetTimeMs() );
            break;
        case TASK_TX_FSK :
        case TASK_RX_FSK :
        case TASK_CAD    :
        default :
        break;
    } 
}


/************************************************************************************/
/*                                 Timer Isr Routine                                */
/*                               Called when Alarm expires                          */
/************************************************************************************/

template <class R>     
void RadioPLaner<R>::IsrTimerRadioPlaner( void ) {
    PlanerTimerState = TIMER_IDLE ;
    switch  ( TaskType [ CurrentHookToExecute ] ) {
        case TASK_TX_LORA : 
            RadioPlanerState = RADIO_IN_TX ;
            break;
        case TASK_TX_FSK  :
            RadioPlanerState = RADIO_IN_TX ; 
            break;
        case TASK_RX_LORA   :
            Radio->RxLora ( Bw[ CurrentHookToExecute ], Sf[ CurrentHookToExecute ], Channel[ CurrentHookToExecute ], TimeOutMs[ CurrentHookToExecute ] );
            RadioPlanerState = RADIO_IN_RX ;
            break;
        case TASK_RX_FSK    :
            Radio->RxFsk(Channel[ CurrentHookToExecute ], TimeOutMs[ CurrentHookToExecute ]);
            RadioPlanerState = RADIO_IN_RX ;
            break;
        case TASK_CAD :
            RadioPlanerState = RADIO_IN_CAD ;
            break;
        default :
            RadioPlanerState = RADIO_IN_IDLE;
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
    ComputePlanerStatus ( ); // get irq flag 
    // get fifo 
    CallBackHook0 ( );
    // launch arbitrer 
    Radio->Sleep( false );
    RadioPlanerState = RADIO_IN_IDLE;
} 

