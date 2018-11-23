  

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


void FreeStask (STask *task ) {
    task->HookId          = 0x0;
    task->StartTime       = 0;
    task->TaskDuration    = 0; 
    task->TaskType        = NONE;
    task->State           = TASK_FINISHED;
}


template <class R> RadioPLaner <R>::RadioPLaner( R * RadioUser) {
    mcu.AttachInterruptIn( &RadioPLaner< R >::CallbackIsrRadioPlaner,this); // attach it radio
    Radio = RadioUser;
    for ( int i = 0 ; i < NB_HOOK; i++ ) { 
        sTask[ i ].HookId         = i;
        sTask[ i ].TaskType       = NONE;
        sTask[ i ].StartTime      = 0;
        sTask[ i ].TaskDuration   = 0;
        sTask[ i ].State          = TASK_FINISHED;
        objHook[ i ]              = NULL;
    }
    FreeStask ( &sNextTask );
    RadioTaskId          = 0;
    TimerTaskId          = 0;
    HookToExecute        = 0;
    TimeOfHookToExecute  = 0;
    IrqTimeStampMs       = 0;
    PlanerTimerState     = TIMER_IDLE;
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
eHookStatus RadioPLaner<R>::InitHook ( uint8_t HookIdIn,  void (* AttachCallBack) (void * ), void * objHookIn ) {
    if ( HookIdIn > NB_HOOK ) {
        return ( HOOK_ERROR );
    }
    AttachCallBackHook [ HookIdIn ] = AttachCallBack ;
    objHook[ HookIdIn ]             = objHookIn; 
    return ( HOOK_OK );
}

/***********************************************************************************************/
/*                            RadioPlaner GetMyHookId Method                                   */
/***********************************************************************************************/
template <class R>
eHookStatus  RadioPLaner<R>::GetMyHookId  ( void * objHookIn, uint8_t * HookIdIn ){
    for (int i = 0 ; i < NB_HOOK ; i ++) {
        if ( objHookIn == objHook[i] ){
            *HookIdIn = i ;
            return ( HOOK_OK );
        } 
    }
    return (HOOK_ERROR);
}


/***********************************************************************************************/
/*                            RadioPlaner EnqueueTask Method                                   */
/***********************************************************************************************/
template <class R> 
eHookStatus RadioPLaner<R>::EnqueueTask( STask *staskIn, uint8_t *payload, uint8_t *payloadSize, SRadioParam *sRadioParamIn ){

// if running task up to now reject
     
    // change NOW AT TIME
    uint8_t HookId = staskIn->HookId;
    if ( sTask [ HookId ].State == TASK_RUNNING ) {
        DEBUG_MSGRP (" Enqueue Task Impossible because your task already running Too BE MANAGE LATER WITh ABORT current job\n");
        return ( HOOK_ERROR);  
    }
    
    sTask             [ HookId ] = *staskIn;
    sRadioParam       [ HookId ] = *sRadioParamIn;
    Payload           [ HookId ] = payload;
    PayloadSize       [ HookId ] = payloadSize;
    //tb implemented check if already running task and return error
   // DEBUG_MSGRP ("                     enqueu task for hook id = %d\n",HookId);
   
    sTask [ HookId ].Priority = (  sTask [ HookId ].State * NB_HOOK ) + HookId;
    ComputeRanking     ( );
    CallPlanerArbitrer ( );
    return(HOOK_OK);
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
        Index = FindHighestPriority ( RankTemp,  NB_HOOK );
        RankTemp [ Index ] = 0xFF;
        Ranking [ i ] = Index ;
    }
}  

template <class R> 
uint8_t  RadioPLaner<R>:: FindHighestPriority ( uint8_t * vec,  uint8_t length ) {
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

/************************************************************************************/
/*                                 Selec tNext Task                                 */
/*                                                                                  */
/************************************************************************************/


template <class R> 
uint8_t  RadioPLaner<R>::SelectTheNextTask( void ) {
    //DEBUG_MSGRP ("                                                  call Select Next Task");
    //PrintTask ( 0 );
    //PrintTask ( 2 );
    uint8_t HookToExecuteTmp = 0xFF;
    uint32_t TimeOfHookToExecuteTmp;
    uint32_t TempTime;
    uint8_t index ;
    int k ;
    for ( k = 0 ; k < NB_HOOK; k++ ) {
        index = Ranking [ k ] ;
        if (sTask[ index ].State < TASK_ABORTED) {  // Mean   TASK_SCHEDULE or TASK_ASAP or TASK_RUNNING,
            HookToExecuteTmp        = sTask[ index ].HookId ;
            TimeOfHookToExecuteTmp  = sTask[ index ].StartTime ;
            break;
        }
    }
    if (k == NB_HOOK ) {
        return (NO_MORE_TASK);
    }
    for (int i = k ; i < NB_HOOK; i++ ) {
        index = Ranking [ i ] ;
        if ( sTask[ index ].State < TASK_ABORTED ) {
            TempTime =  sTask[ index ].StartTime + sTask[ index ].TaskDuration ;
            if ( ( TempTime - TimeOfHookToExecuteTmp ) < 0 ) {   //@relative to avoid issues in case of wrapping
                TimeOfHookToExecuteTmp = sTask[ index ].StartTime ;
                HookToExecuteTmp       = sTask[ index ].HookId ;
            }
        }
    }
    sNextTask = sTask[ HookToExecuteTmp ];
    return (SCHEDULED_TASK);
}
/************************************************************************************/
/*                                 Planer Arbiter                                   */
/*                                                                                  */
/************************************************************************************/

template <class R> 
void  RadioPLaner<R>::CallPlanerArbitrer ( void ) {
    UpdateTaskTab      ( );
  
    if ( SelectTheNextTask() == SCHEDULED_TASK ) { // Next Task Exist
        uint32_t CurrentTime = mcu.RtcGetTimeMs () + MARGIN_DELAY ;
        int delay = sNextTask.StartTime - CurrentTime ;
        if ( delay > 0 ) { // Require A Timer
            if ( ( PlanerTimerState == TIMER_BUSY ) && ( sNextTask.HookId == TimerTaskId ) ) {  
            } else {
                TimerTaskId = sNextTask.HookId; 
                PlanerTimerState  = TIMER_BUSY ;
                SetAlarm ( delay );
            }
        } else { // Have To Launch Radio
                if ( sTask[RadioTaskId].State == TASK_RUNNING ) { // Radio is already Running Have to abort current Task
                    if ( sTask[RadioTaskId].HookId != sNextTask.HookId ) {
                      ///  DEBUG_MSGRP("\n");
                      //  DEBUG_PRINTFRP ("                        ABORTED                      RadioRunning Task = %d TimerRunningTask = %d Next TAsk = %d ",sRadioRunningTask.HookId , sTimerRunningTask.HookId,sNextTask.HookId);
                        sTask[RadioTaskId].State = TASK_ABORTED;
                        Radio->Sleep( false );
                        RadioTaskId = sNextTask.HookId;
                        sTask[RadioTaskId].State= TASK_RUNNING;
                        LaunchCurrentTask ( ); 
                    } else {
                       // radio  is already executing nextTask ? do nothing , faudra gerer le cas collision sur le meme thread
                    }

                } else { // radio is sleeping
                    RadioTaskId = sNextTask.HookId;
                    sTask[RadioTaskId].State= TASK_RUNNING;
                    LaunchCurrentTask ( );
                }
        }
    } else {
            DEBUG_MSGRP ("                                              ");
            DEBUG_MSGRP (" No More Active Task inside the RadioPlaner \n");
    }
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
 void RadioPLaner<R> :: GetIRQStatus ( uint8_t HookIdIn ) {   //rename
    uint8_t Id = HookIdIn;
    IrqFlags_t IrqRadio  = ( Radio->GetIrqFlagsLora( ) );
    Radio->ClearIrqFlagsLora( ); 
    switch ( IrqRadio ) {
        case SENT_PACKET_IRQ_FLAG    :
            RadioPlanerStatus = PLANER_TX_DONE ;  
            break;
        case RECEIVE_PACKET_IRQ_FLAG : 
            RadioPlanerStatus = PLANER_RX_PACKET;
            // Push Fifo
            Read_RadioFifo ( sTask[Id] );
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
    mcu.StartTimerMsecond( &RadioPLaner<R>::CallbackIsrTimerRadioPlaner,this, alarmInMs);
}

template <class R> 
eHookStatus  RadioPLaner<R>:: Read_RadioFifo ( STask TaskIn) {
    eHookStatus status = HOOK_OK;
    uint8_t Id = TaskIn.HookId;
    if (TaskIn.TaskType == RX_LORA ) {
        Radio->FetchPayloadLora( PayloadSize [ Id ],  Payload [ Id ], sRadioParam[Id].Snr, sRadioParam[Id].Rssi);
    } else if ( TaskIn.TaskType  == RX_FSK ) {
        Radio->FetchPayloadFsk( PayloadSize [ Id ],  Payload [ Id ], sRadioParam[Id].Snr, sRadioParam[Id].Rssi);
    } else {
        status = HOOK_ERROR;
    }
    return status;
}

template <class R> 
void  RadioPLaner<R>::UpdateTaskTab      ( void ) {
    uint32_t CurrentTime =  mcu.RtcGetTimeMs() ;
    for (int i = 0 ; i < NB_HOOK ; i++ ){
        if ( sTask [ i ].State == TASK_ASAP ) {
            sTask [ i ].StartTime = CurrentTime ;
        }
    }
};



template <class R> 
void  RadioPLaner<R>::CallAbortedTAsk ( void ) {
    for (int i = 0; i < NB_HOOK; i ++ ) {
        if ( sTask[i].State == TASK_ABORTED ) {
            DEBUG_PRINTFRP("callback for aborted hook %d\n",i);
            CallBackHook ( i );
            FreeStask ( &sTask[i] );
        }
    }
}

template <class R> 
void  RadioPLaner<R>::LaunchCurrentTask ( void ){
     uint8_t Id = RadioTaskId;
     DEBUG_MSGRP("                      Start Radio ");
     PrintTask ( sTask[Id] ); 
     switch ( sTask[Id].TaskType) {
        case TX_LORA :
            Radio->SendLora( Payload[Id], *PayloadSize[Id], sRadioParam[Id].Sf, sRadioParam[Id].Bw, sRadioParam[Id].Frequency, sRadioParam[Id].Power );
            break;
        case RX_LORA :
            Radio->RxLora ( sRadioParam[Id].Bw, sRadioParam[Id].Sf, sRadioParam[Id].Frequency, sRadioParam[Id].TimeOutMs);
            break;
        case TX_FSK :
        case RX_FSK :
        case CAD    :
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
    //DEBUG_MSGRP ("                                            Timer Expire\n");
    PlanerTimerState = TIMER_IDLE ;
    CallPlanerArbitrer ();
}              


/************************************************************************************/
/*                                 Radio Isr Routine                                */
/*                             Called when Radio raises It                          */
/************************************************************************************/
template <class R> 
void  RadioPLaner<R>::IsrRadioPlaner ( void ) {
    IrqTimeStampMs = mcu.RtcGetTimeMs( );

    GetIRQStatus ( RadioTaskId );
    DEBUG_PRINTFRP("                                                                 Receive It Radio for HookID = %d\n",RadioTaskId);
    CallBackHook ( RadioTaskId );
    FreeStask (&sTask[RadioTaskId]);
    CallAbortedTAsk ();
    Radio->Sleep( false );
    CallPlanerArbitrer (); 
} 




/************************************************************************************************/
/*                      DEBUG UTILITIES                                                         */
/************************************************************************************************/
template <class R> 
void  RadioPLaner<R>::PrintTask ( STask TaskIn ) {
    
    DEBUG_PRINTFRP (" RadioRunning Task = %d TimerRunningTask = %d HookId = %d ",RadioTaskId , TimerTaskId ,TaskIn.HookId );
    switch (TaskIn.TaskType) {
        case  RX_LORA : 
            DEBUG_MSGRP (" TASK_RX_LORA ");
            break;
        case  RX_FSK :
            DEBUG_MSGRP (" TASK_RX_FSK ");
            break;
        case  TX_LORA : 
            DEBUG_MSGRP (" TASK_TX_LORA ");
            break;
        case  TX_FSK :
            DEBUG_MSGRP (" TASK_TX_FSK ");
            break;
        case  CAD :
            DEBUG_MSGRP (" TASK_CAD ");
            break;
        case  NONE : 
            DEBUG_MSGRP (" TASK_EMPTY ");
            break;
        default :
           DEBUG_MSGRP (" TASK_ERROR ");
            break;
    };
    DEBUG_PRINTFRP (" StartTime  @%d with priority = %d",TaskIn.StartTime,TaskIn.Priority);
    DEBUG_MSGRP("\n");
}
