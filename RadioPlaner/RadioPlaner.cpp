  

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
    task->HookId          = 0xFF;
    task->StartTime       = 0;
    task->TaskDuration    = 0; 
    task->TaskTimingType  = TASK_BACKGROUND;
    task->TaskType        = TASK_EMPTY;
}


template <class R> RadioPLaner <R>::RadioPLaner( R * RadioUser) {
    mcu.AttachInterruptIn( &RadioPLaner< R >::CallbackIsrRadioPlaner,this); // attach it radio
    Radio = RadioUser;
    for ( int i = 0 ; i < NB_HOOK; i++ ) { 
        sTask[ i ].HookId         = i;
        sTask[ i ].TaskType       = TASK_EMPTY;
        sTask[ i ].StartTime      = 0;
        sTask[ i ].TaskDuration   = 0;
        sTask[ i ].TaskTimingType = TASK_BACKGROUND;
        objHook[ i ]              = NULL;
    }
    FreeStask ( &sTimerRunningTask );
    FreeStask ( &sRadioRunningTask );
    FreeStask ( &sNextTask );
    HookToExecute = 0xFF;
    RadioPlanerState = RADIO_IDLE;
    PlanerTimerState = TIMER_IDLE;
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
void RadioPLaner<R>::EnqueueTask( STask *staskIn, uint8_t *payload, uint8_t *payloadSize, SRadioParam *sRadioParamIn ){
    uint8_t HookId = staskIn->HookId;
    sTask             [ HookId ] = *staskIn;
    sRadioParam       [ HookId ] = *sRadioParamIn;
    Payload           [ HookId ] = payload;
    PayloadSize       [ HookId ] = payloadSize;
    //tb implemented check if already running task and return error
   // DEBUG_MSGRP ("                     enqueu task for hook id = %d\n",HookId);
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
 void RadioPLaner<R> :: ComputePlanerStatus ( uint8_t HookIdIn ) {   //rename
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
    if (TaskIn.TaskType == TASK_RX_LORA ) {
        Radio->FetchPayloadLora( PayloadSize [ Id ],  Payload [ Id ], sRadioParam[Id].Snr, sRadioParam[Id].Rssi);
    } else if ( TaskIn.TaskType  == TASK_RX_FSK ) {
        Radio->FetchPayloadFsk( PayloadSize [ Id ],  Payload [ Id ], sRadioParam[Id].Snr, sRadioParam[Id].Rssi);
    } else {
        status = HOOK_ERROR;
    }
    return status;
}

template <class R> 
void  RadioPLaner<R>::UpdateTaskTab      ( void ) {
    for (int i = 0 ; i < NB_HOOK ; i++ ){
        if ( sTask [ i ].TaskTimingType == TASK_ASSAP ) {
            sTask [ i ].StartTime = mcu.RtcGetTimeMs() + 1; //@note manage timeout 
        }
    }

};
template <class R> 
void  RadioPLaner<R>:: ComputePriority ( void ) {
    for (int i = 0 ; i < NB_HOOK ; i++ ){
        sTask [ i ].Priority = (  sTask [ i ].TaskTimingType * NB_HOOK ) + i ; // the lowest value is the highest priority.
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
        if (sTask[ index ].TaskType < TASK_EMPTY) {
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
        if ( sTask[ index ].TaskType < TASK_EMPTY ) {
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
    ComputePriority    ( );
    ComputeRanking     ( );
    if ( SelectTheNextTask() == SCHEDULED_TASK ) { // Store the result in the variable HookToExecut
         PrintTask ( sNextTask ); 
        if  ( ( sNextTask.HookId == sTimerRunningTask.HookId ) && (PlanerTimerState = TIMER_IDLE  ) ) {
              DEBUG_MSGRP ("                                             Nothing New \n");
        } else {
            uint32_t CurrentTime = mcu.RtcGetTimeMs () + MARGIN_DELAY ;
            int delay = sNextTask.StartTime - CurrentTime ;
            DEBUG_PRINTFRP ( "                                                         delay = %d, Currentime = %d \n ",delay,CurrentTime);
            if ( delay > 0 ) {
                 DEBUG_PRINTFRP ("                                                     New TAsk Schedule delay %d\n",delay);
                sTimerRunningTask = sNextTask; 
                PlanerTimerState  = TIMER_BUSY ;
                SetAlarm ( delay );
            } else {   // Meaning task should be running Now => call Radio
                 DEBUG_MSGRP ("                                                           New TAsk Schedule Immediate\n");
                sRadioRunningTask = sNextTask;
                FreeStask ( &sNextTask ) ;
                LaunchCurrentTask ();
            }
        }
    } else {
            DEBUG_MSGRP ("                                                ");
            DEBUG_MSGRP (" No More Active Task inside the RadioPlaner \n");
    }
    
}




template <class R> 
void  RadioPLaner<R>::LaunchCurrentTask ( void ){
     uint8_t Id = sRadioRunningTask.HookId;
     DEBUG_MSGRP("                                                                 Start Radio");
     PrintTask ( sTask[Id] ); 
     switch ( sTask[Id].TaskType) {
        case TASK_TX_LORA :
             Radio->SendLora( Payload[Id], *PayloadSize[Id], sRadioParam[Id].Sf, sRadioParam[Id].Bw, sRadioParam[Id].Frequency, sRadioParam[Id].Power );
            break;
        case TASK_RX_LORA :
            Radio->RxLora ( sRadioParam[Id].Bw, sRadioParam[Id].Sf, sRadioParam[Id].Frequency, sRadioParam[Id].TimeOutMs);
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
    DEBUG_MSGRP ("                                            Timer Expire\n");
    if (sRadioRunningTask.TaskType < TASK_EMPTY) {
                DEBUG_MSGRP ("                                                ");
                DEBUG_MSGRP ("Radio already running\n");
                while(1){}
    } 
    PlanerTimerState = TIMER_IDLE ;
    CallPlanerArbitrer ();
}              


/************************************************************************************/
/*                                 Radio Isr Routine                                */
/*                             Called when Radio raises It                          */
/************************************************************************************/
template <class R> 
void  RadioPLaner<R>::IsrRadioPlaner ( void ) {
    uint8_t Id = sRadioRunningTask.HookId;
     FreeStask ( &sRadioRunningTask );
    IrqTimeStampMs = mcu.RtcGetTimeMs( );
    DEBUG_PRINTFRP("                                                       receive it for hook %d\n",HookToExecute);
    ComputePlanerStatus ( Id );
    CallBackHook( Id );
    FreeStask (&sTask[Id]);
    Radio->Sleep( false );
    CallPlanerArbitrer (); 
} 











/************************************************************************************************/
/*                      DEBUG UTILITIES                                                         */
/************************************************************************************************/
template <class R> 
void  RadioPLaner<R>::PrintTask ( STask TaskIn ) {
    DEBUG_MSGRP("\n");
    DEBUG_PRINTFRP ("                                              RadioRunning Task = %d TimerRunningTask = %d HookId = %d ",sRadioRunningTask.HookId , sTimerRunningTask.HookId,TaskIn.HookId );
    switch (TaskIn.TaskType) {
        case  TASK_RX_LORA : 
            DEBUG_MSGRP (" TASK_RX_LORA ");
            break;
        case  TASK_RX_FSK :
            DEBUG_MSGRP (" TASK_RX_FSK ");
            break;
        case  TASK_TX_LORA : 
            DEBUG_MSGRP (" TASK_TX_LORA ");
            break;
        case  TASK_TX_FSK :
            DEBUG_MSGRP (" TASK_TX_FSK ");
            break;
        case  TASK_CAD :
            DEBUG_MSGRP (" TASK_CAD ");
            break;
        case  TASK_EMPTY : 
            DEBUG_MSGRP (" TASK_EMPTY ");
            break;
        default :
           DEBUG_MSGRP (" TASK_ERROR ");
            break;
    };
    DEBUG_PRINTFRP (" StartTime  @%d with priority = %d",TaskIn.StartTime,TaskIn.Priority);
    switch (TaskIn.TaskTimingType) {
        case  TASK_AT_TIME : 
            DEBUG_MSGRP (" TASK_AT_TIME ");
            break;
        case  TASK_NOW :
            DEBUG_MSGRP (" TASK_NOW ");
            break;
        case  TASK_ASSAP : 
            DEBUG_MSGRP (" TASK_ASSAP ");
            break;
        case  TASK_BACKGROUND : 
            DEBUG_MSGRP (" TASK_BACKGROUND ");
        default :
           DEBUG_MSGRP (" TASK_PRIORITY_ERROR ");
    };
    DEBUG_MSGRP("\n");
}
