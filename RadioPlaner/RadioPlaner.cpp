  

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
        sTask[ i ].HookId         = i;
        sTask[ i ].TaskType       = TASK_IDLE;
        sTask[ i ].StartTime      = 0;
        sTask[ i ].TaskDuration   = 0;
        sTask[ i ].TaskTimingType = NO_TASK;
    }
}
template <class R> RadioPLaner<R>::~RadioPLaner( ) {
}
/************************************************************************************************/
/*                      Public  Methods                                                         */
/************************************************************************************************/
/***********************************************************************************************/
/*                       RadioPlaner Init Method                                               */
/***********************************************************************************************/

template <class R>
eHookStatus RadioPLaner<R>::InitHook ( uint8_t HookId,  void (* AttachCallBack) (void * ), void * objHookIn ) {
    if ( HookId > NB_HOOK ) {
        return ( HOOK_ERROR );
    }
    AttachCallBackHook [ HookId ] = AttachCallBack ;
    objHook[ HookId ]             = objHookIn; 
    return ( HOOK_OK );
}

/***********************************************************************************************/
/*                            RadioPlaner GetMyHookId Method                                   */
/***********************************************************************************************/
template <class R>
eHookStatus  RadioPLaner<R>::GetMyHookId  ( void * objHookIn, uint8_t * HookId ){
    for (int i = 0 ; i < NB_HOOK ; i ++) {
        if ( objHookIn == objHook[i] ){
            * HookId = i ;
            return ( HOOK_OK );
        } 
    }
    return (HOOK_ERROR);
}

  
/***********************************************************************************************/
/*                            RadioPlaner EnqueueTask Method                                   */
/***********************************************************************************************/
template <class R> 
void RadioPLaner<R>::EnqueueTask( STask *staskIn, uint8_t *payload, uint8_t *payloadSize, SRadioParam *sRadioParamIn ){
    uint8_t HookId = staskIn->HookId;
    sTask             [ HookId ] = *staskIn;
    sRadioParam       [ HookId ] = *sRadioParamIn;
    Payload           [ HookId ] = payload;
    PayloadSize       [ HookId ] = payloadSize;
    //tb implemented check if already running task and return error
    CallPlanerArbitrer ( );
}


/***********************************************************************************************/
/*                            RadioPlaner GetPlanerStatus Method                               */
/***********************************************************************************************/

template <class R> 
  void RadioPLaner<R>::GetStatusPlaner ( uint32_t * IrqTimestampMs, ePlanerStatus *PlanerStatus ){
    *IrqTimestampMs = IrqTimeStampMs;
    *PlanerStatus   = RadioPlanerStatus;
};


/************************************************************************************************/
/*                      Private  Methods                                                        */
/************************************************************************************************/

/************************************************************************************/
/*                                 Planer Utilities                                 */
/*                                                                                  */
/************************************************************************************/

template <class R>  //@tbd ma   nage all error case
 void RadioPLaner<R> :: ComputePlanerStatus (void ) {
    uint8_t Id = HookToExecute;
    IrqFlags_t IrqRadio  = ( Radio->GetIrqFlagsLora( ) );
    Radio->ClearIrqFlagsLora( ); 
    switch ( IrqRadio ) {
        case SENT_PACKET_IRQ_FLAG    :
            RadioPlanerStatus = PLANER_TX_DONE ;  
            break;
        case RECEIVE_PACKET_IRQ_FLAG : 
            RadioPlanerStatus = PLANER_RX_PACKET;
            // Push Fifo
            Read_RadioFifo ( sTask[Id].TaskType );
            break; 
        case RXTIMEOUT_IRQ_FLAG      : 
            RadioPlanerStatus = PLANER_RX_TIMEOUT;
            break;
        default:
            break;
    }
}
 
template <class R>     
void RadioPLaner<R>::SetAlarm (uint32_t alarmInMs ) {
    PlanerTimerState = TIMER_BUSY ;
    mcu.StartTimerMsecond( &RadioPLaner<R>::CallbackIsrTimerRadioPlaner,this, alarmInMs);
}

template <class R> 
eHookStatus  RadioPLaner<R>:: Read_RadioFifo ( eRadioPlanerTask  TaskType) {
    eHookStatus status = HOOK_OK;
    if (TaskType == TASK_RX_LORA ) {
        Radio->FetchPayloadLora( PayloadSize [ HOOK_0 ],  Payload [ HOOK_0 ], sRadioParam[HOOK_0].Snr, sRadioParam[HOOK_0].Rssi);
    } else if ( TaskType == TASK_RX_FSK ) {
        Radio->FetchPayloadFsk( PayloadSize [ HOOK_0 ],  Payload [ HOOK_0 ], sRadioParam[HOOK_0].Snr, sRadioParam[HOOK_0].Rssi);
    } else {
        status = HOOK_ERROR;
    }
    return status;
}
template <class R> 
void  RadioPLaner<R>:: ComputePriority ( void ) {
    for (int i = 0 ; i < NB_HOOK ; i++ ){
        sTask [ i ].Priority = (  sTask [ i ].TaskTimingType * NB_HOOK ) + i ; // the lowest value is the highest priority.
    }
}
template <class R> 
uint8_t  RadioPLaner<R>:: FindHighestPriotity ( uint8_t * vec,  uint8_t length ) {
    uint8_t HighPrio = 0xFF ;
    uint8_t Index = 0;
    for (int i = 0 ; i < length ; i++ ){
        if ( vec [ i ] <= HighPrio ) {
            HighPrio = vec [ i ];
            Index = i; 
        }
    }
    return ( Index );
}

template <class R> 
void  RadioPLaner<R>:: ComputeRanking ( void ) { //@tbd implementation should be optimized but very few hooks
    int i;
    uint8_t Index;
    uint8_t RankTemp [NB_HOOK];
    for (int i = 0 ; i < NB_HOOK; i++ ) {
        RankTemp [ i ] =  sTask [ i ].Priority;
    } 
    for (i = 0 ; i < NB_HOOK; i ++) {
        Index = FindHighestPriotity ( RankTemp,  NB_HOOK );
        RankTemp [ Index ] = 0xFF;
        Ranking [ i ] = Index ;
    }
}
template <class R> 
void  RadioPLaner<R>::SelectTheNextTask( void ) {
    HookToExecute  = sTask[ Ranking [ 0 ] ].HookId ;
    TimeOfHookToExecute = sTask[ Ranking [ 0 ] ].StartTime ;
    uint32_t TempTime;
    uint8_t index ;
    for (int i = 1 ; i < NB_HOOK; i++ ) {
        index = Ranking [ i ] ;
        if ( sTask[ index ].TaskType < TASK_IDLE ) {
            TempTime =  sTask[ index ].StartTime + sTask[ index ].TaskDuration ;
            if ( ( TempTime - TimeOfHookToExecute ) > 0 ) {   //@relative to avoid issues in case of wrapping
                TimeOfHookToExecute = sTask[ index ].StartTime ;
                HookToExecute       = sTask[ index ].HookId ;
            }
        }
    }
}
/************************************************************************************/
/*                                 Planer Arbiter                                   */
/*                                                                                  */
/************************************************************************************/

template <class R> 
void  RadioPLaner<R>::CallPlanerArbitrer ( void ) {
    ComputePriority    ( );
    ComputeRanking     ( ); 
    SelectTheNextTask  ( ); // Store the result in the variable HookToExecute
    LaunchTask ( );
    
}

template <class R> 
void  RadioPLaner<R>::LaunchTask ( void ){
    uint8_t Id = HookToExecute;
     switch ( sTask[ Id ].TaskType) {
        case TASK_IDLE    :
            break;
        case TASK_TX_LORA :
            Radio->SendLora( Payload[Id], *PayloadSize[Id], sRadioParam[Id].Sf, sRadioParam[Id].Bw, sRadioParam[Id].Frequency, sRadioParam[Id].Power );
            RadioPlanerState = RADIO_BUSY ;
            break;
        case TASK_RX_LORA :
            SetAlarm ( sTask[ Id ].StartTime - mcu.RtcGetTimeMs() );
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
    uint8_t Id = HookToExecute;
    PlanerTimerState = TIMER_IDLE ;
    switch  ( sTask[ Id ].TaskType ) {
        case TASK_TX_LORA : 
            RadioPlanerState = RADIO_BUSY ;
            break;
        case TASK_TX_FSK  :
            RadioPlanerState = RADIO_BUSY ; 
            break;
        case TASK_RX_LORA   :
            Radio->RxLora ( sRadioParam[Id].Bw, sRadioParam[Id].Sf, sRadioParam[Id].Frequency, sRadioParam[Id].TimeOutMs);
            RadioPlanerState = RADIO_BUSY ;
            break;
        case TASK_RX_FSK    :
            Radio->RxFsk(sRadioParam[Id].Frequency, sRadioParam[Id].TimeOutMs);
            RadioPlanerState = RADIO_BUSY ;
            break;
        case TASK_CAD :
            RadioPlanerState = RADIO_BUSY ;
            break;
        default :
            RadioPlanerState = RADIO_IDLE;
            break;    
    }
}              


/************************************************************************************/
/*                                 Radio Isr Routine                                */
/*                             Called when Radio raises It                          */
/************************************************************************************/
template <class R> 
void  RadioPLaner<R>::IsrRadioPlaner ( void ) {
    uint8_t Id = HookToExecute;
    IrqTimeStampMs = mcu.RtcGetTimeMs( );
    ComputePlanerStatus ( ); 
    CallBackHook( Id );
    // launch arbitrer 
    Radio->Sleep( false );
    RadioPlanerState = RADIO_IDLE;
} 

