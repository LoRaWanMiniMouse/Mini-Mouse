  

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
        DurationTask  [ i ] = 0;
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
void RadioPLaner<R>::Send( STask stask, uint8_t *payload, uint8_t payloadSize, SRadioParam sRadioParamIn){
    uint8_t HookId = stask.HookId;
    if ( sRadioParamIn.Modulation == LORA) {
        TaskType [ HookId ] = TASK_TX_LORA;  
    } else {
        TaskType [ HookId ] = TASK_TX_FSK;  
    }
    DurationTask      [ HookId ] = stask.TaskDuration;
    StartTimeTask     [ HookId ] = stask.StartTime;
    sRadioParam       [ HookId ] = sRadioParamIn;
    Payload           [ HookId ] = payload;
    PayloadSize       [ HookId ] = payloadSize;
    CallPlanerArbitrer ( );
  
}


template <class R> 
void RadioPLaner<R>::Rx (STask stask , SRadioParam sRadioParamIn, uint16_t TimeOutMsec ){
    uint8_t HookId = stask.HookId;
    TaskType      [ HookId ] = TASK_RX_LORA;
    DurationTask  [ HookId ] = mcu.RtcGetTimeMs() + 0;
    StartTimeTask [ HookId ] = mcu.RtcGetTimeMs() + stask.StartTime;
    sRadioParam   [ HookId ] = sRadioParamIn;
    TimeOutMs [ HookId ]     = TimeOutMsec;
    CallPlanerArbitrer ( );
   
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
                if (RadioPlanerState == RADIO_IN_TX_LORA ) {
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
void RadioPLaner<R>::FetchPayload( uint8_t *payloadSize, uint8_t payload[255], int16_t *snr, int16_t *signalRssi){
    if (RadioPlanerState == RADIO_IN_RX_LORA ) {
        Radio->FetchPayloadLora( payloadSize, payload, snr, signalRssi);
    } else {
        Radio->FetchPayloadFsk( payloadSize, payload, snr, signalRssi);
    }
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
            Radio->SendLora( Payload[Id], PayloadSize[Id], sRadioParam[Id].Sf, sRadioParam[Id].Bw, sRadioParam[Id].Frequency, sRadioParam[Id].Power );
            RadioPlanerState = RADIO_IN_TX_LORA ;
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
    uint8_t Id = CurrentHookToExecute;
    PlanerTimerState = TIMER_IDLE ;
    switch  ( TaskType [ CurrentHookToExecute ] ) {
        case TASK_TX_LORA : 
            RadioPlanerState = RADIO_IN_TX_LORA ;
            break;
        case TASK_TX_FSK  :
            RadioPlanerState = RADIO_IN_TX_FSK ; 
            break;
        case TASK_RX_LORA   :
            Radio->RxLora ( sRadioParam[Id].Bw, sRadioParam[Id].Sf, sRadioParam[Id].Frequency, TimeOutMs[ CurrentHookToExecute ] );
            RadioPlanerState = RADIO_IN_RX_LORA ;
            break;
        case TASK_RX_FSK    :
            Radio->RxFsk(sRadioParam[Id].Frequency, TimeOutMs[ CurrentHookToExecute ]);
            RadioPlanerState = RADIO_IN_RX_FSK ;
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
    // get fifo finally not put here becaus if  status error no data fifo to push or put inside the status to be decide
    CallBackHook0 ( );
    // launch arbitrer 
    Radio->Sleep( false );
    RadioPlanerState = RADIO_IN_IDLE;
} 

