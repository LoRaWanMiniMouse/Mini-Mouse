  

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
#include "DefineRadioPlaner.h"
#include "sx1272.h"
#include "sx1276.h"
#include "SX126x.h"
#include "Define.h"
template class RadioPLaner<SX1276>;
template class RadioPLaner<SX1272>;
template class RadioPLaner<SX126x>;
uint32_t debugt1;
uint32_t debugt2;
/************************************************************************************/
/*                                 Planer Utilities                                 */
/*                                                                                  */
/************************************************************************************/
void FreeStask (STask & task ) {
    task.HookId          = 0x0;
    task.StartTime       = 0;
    task.TaskDuration    = 0; 
    task.TaskType        = NONE;
    task.State           = TASK_FINISHED;
}


/************************************************************************************/
/*                                 Planer Class Implementation                      */
/*                                                                                  */
/************************************************************************************/
template <class R> RadioPLaner <R>::RadioPLaner( R * RadioUser) {
    mcu.AttachInterruptIn( &RadioPLaner< R >::CallbackIsrRadioPlaner,this); // attach it radio
    Radio = RadioUser;
    for ( int i = 0 ; i < NB_HOOK; i++ ) { 
        sTask [ i ].HookId         = i;
        sTask [ i ].TaskType       = NONE;
        sTask [ i ].StartTime      = 0;
        sTask [ i ].TaskDuration   = 0;
        sTask [ i ].State          = TASK_FINISHED;
        objHook [ i ]              = NULL;
        IrqTimeStampMs [ i ]       = 0;
        RadioPlanerStatus [ i ]    = PLANER_TASK_ABORTED;
    }
    FreeStask ( sPriorityTask );
    RadioTaskId          = 0;
    TimerTaskId          = 0;
    HookToExecute        = 0;
    TimeOfHookToExecute  = 0;
    PlanerTimerState     = TIMER_IDLE;
    SemaphoreRadio       = 0;  
    SemaphoreTimer       = 0;
    TimerValue           = 0;
    TimerHookId          = 0 ;
    GetNextStateStatus   = NO_MORE_TASK_SCHEDULE ;
    sStatisticRP.InitStat ( );  
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
        return ( HOOK_ID_ERROR );
    }
    AttachCallBackHook [ HookIdIn ] = AttachCallBack ;
    objHook            [ HookIdIn ] = objHookIn; 
    return ( HOOK_OK );
}

/***********************************************************************************************/
/*                            RadioPlaner GetMyHookId Method                                   */
/***********************************************************************************************/
template <class R>
eHookStatus  RadioPLaner<R>::GetMyHookId  ( void * objHookIn, uint8_t& HookIdIn ){
    for (int i = 0 ; i < NB_HOOK ; i ++) {
        if ( objHookIn == objHook[i] ){
            HookIdIn = i ;
            return ( HOOK_OK );
        } 
    }
    return (HOOK_ID_ERROR);
}


/***********************************************************************************************/
/*                            RadioPlaner EnqueueTask Method                                   */
/***********************************************************************************************/
template <class R> 
eHookStatus RadioPLaner<R>::EnqueueTask ( STask& staskIn, uint8_t *payload, uint8_t *payloadSize, SRadioParam& sRadioParamIn ) {
    mcu.DisableIrq ( );
    
 DEBUG_PRINTF("get it Enqueur %d  %d %d %d\n", NVIC_GetPendingIRQ( EXTI4_15_IRQn), NVIC_GetPendingIRQ( EXTI2_3_IRQn), NVIC_GetPendingIRQ( EXTI0_1_IRQn), NVIC_GetPendingIRQ( LPTIM1_IRQn));
                   
    uint8_t HookId = staskIn.HookId;
    if ( HookId > NB_HOOK ) {
        mcu.EnableIrq ( ); 
        return ( HOOK_ID_ERROR );
    }
    if ( sTask [ HookId ].State == TASK_RUNNING ) {
        DEBUG_MSGRP (" Enqueue Task Impossible because the Task is already running\n");
        mcu.EnableIrq ( );
        return ( TASK_ALREADY_RUNNING );  
    }
    
    sTask       [ HookId ] = staskIn;
    sRadioParam [ HookId ] = sRadioParamIn;
    Payload     [ HookId ] = payload;
    PayloadSize [ HookId ] = payloadSize;
    sTask [ HookId ].Priority = (  sTask [ HookId ].State * NB_HOOK ) + HookId;
    DEBUG_PRINTFRP ("enqueu task for task id = %d priority = %d\n",HookId, sTask [ HookId ].Priority);
    ComputeRanking     ( );
    if ( SemaphoreRadio == 0 ) {
        CallPlanerArbitrer ( __FUNCTION__ );
    }
    DEBUG_PRINTF("get it Enqueur %d  %d %d %d\n", NVIC_GetPendingIRQ( EXTI4_15_IRQn), NVIC_GetPendingIRQ( EXTI2_3_IRQn), NVIC_GetPendingIRQ( EXTI0_1_IRQn), NVIC_GetPendingIRQ( LPTIM1_IRQn));

    mcu.EnableIrq      ( );
    return ( HOOK_OK );
}
/***********************************************************************************************/
/*                            RadioPlaner Abort Method                                         */
/***********************************************************************************************/
template <class R> 
eHookStatus RadioPLaner<R>::AbortTask ( STask& staskIn ) { //Open Question Callaback or not uptoNow No
    mcu.DisableIrq ( );
    uint8_t HookId = staskIn.HookId;
    if ( HookId > NB_HOOK ) {
        mcu.EnableIrq ( ); 
        return ( HOOK_ID_ERROR );
    } 
    if ( sTask [ HookId ].State == TASK_RUNNING ) {
        Radio->Sleep ( false );
        sStatisticRP.UpdateState (  mcu.RtcGetTimeMs ( ), HookId ) ;  
    }
    FreeStask ( sTask [ HookId ] );
    RadioPlanerStatus [ HookId ] = PLANER_TASK_ABORTED ;
    if ( SemaphoreRadio == 0 ) {
        CallPlanerArbitrer ( __FUNCTION__ );
    }
    mcu.EnableIrq ( );
    return ( HOOK_OK ); 
}

/***********************************************************************************************/
/*                            RadioPlaner GetPlanerStatus Method                               */
/*       ePlanerStatus could be :                                                              */     
/*                PLANER_RX_CANCELED,                                                          */
/*                PLANER_TX_CANCELED,                                                          */
/*                PLANER_RX_CRC_ERROR,                                                         */
/*                PLANER_CAD_POSITIVE,                                                         */
/*                PLANER_CAD_NEGATIVE,                                                         */
/*                PLANER_TX_DONE,                                                              */
/*                PLANER_RX_PACKET,                                                            */
/*                PLANER_RX_TIMEOUT                                                            */
/*                                                                                             */
/***********************************************************************************************/

template <class R> 
void RadioPLaner<R>::GetStatusPlaner ( uint8_t HookIdIn, uint32_t & IrqTimestampMs, ePlanerStatus & PlanerStatus ) {
    IrqTimestampMs = IrqTimeStampMs    [ HookIdIn ];
    PlanerStatus   = RadioPlanerStatus [ HookIdIn ];
}


/************************************************************************************************/
/*                      Privates   Methods                                                      */
/************************************************************************************************/

/************************************************************************************/
/*                                 Planer Arbiter                                   */
/*                                                                                  */
/************************************************************************************/

template <class R> 
void  RadioPLaner<R>::CallPlanerArbitrer ( std::string   WhoCallMe ) {
    
    uint32_t CurrentTime = mcu.RtcGetTimeMs ( ) ;
    UpdateTimeTaskASAP ( CurrentTime );
    if ( SelectPriorityTask (  CurrentTime ) == SOMETHING_TO_DO ) { // Next Task Exist
        int delay = (int ) ( sPriorityTask.StartTime - CurrentTime ) ;
        DEBUG_PRINTFRP ( " arbitrer has been  call  who call arbitrer %s and Priority Task = %d TimerHookId = %d delay = %d\n ", WhoCallMe.c_str(), sPriorityTask.HookId, TimerHookId ,delay);
        if ( delay >  MARGIN_DELAY ) { // Have To Launch A Timer so the next task is a schedule task 
        } else if ( delay < 0  ) { // Have To Abort task (just set flag aborted is impossible because may be no more task in scheduler) 
            if ( sPriorityTask.State != TASK_RUNNING ) {
                DEBUG_PRINTF ( " ERROR IN RADIO PLANER  delay = % d Hook Id = %d  \n", delay, sPriorityTask.HookId) ;
                sTask [  sPriorityTask.HookId ].State = TASK_ABORTED;
                if ( sTask [ RadioTaskId ].State != TASK_RUNNING ) { 
                    if (sTask [ RadioTaskId ].HookId == TimerHookId) { // Stop Timer
                        mcu.StopTimerMsecond ( );
                    }
                    CallAbortedTask () ;
                }
            }
        } else { // Have To Launch Radio
            if ( sTask [ RadioTaskId ].State == TASK_RUNNING ) { // Radio is already Running       
                if ( sTask [ RadioTaskId ].HookId != sPriorityTask.HookId ) { // Priority task not equal to radio task => abort radio task
                    sTask [ RadioTaskId ].State = TASK_ABORTED;
                    DEBUG_PRINTFRP ("abort running task with hookid = %d in Arbitrer \n",RadioTaskId);
                    Radio->ClearIrqFlagsLora( );
                    Radio->Sleep ( false );
                    DEBUG_PRINTF("get it %d  %d %d \n", NVIC_GetPendingIRQ( EXTI4_15_IRQn), NVIC_GetPendingIRQ( EXTI2_3_IRQn), NVIC_GetPendingIRQ( EXTI0_1_IRQn));
                    if ( NVIC_GetPendingIRQ( EXTI0_1_IRQn) == 1 ) {
                        DEBUG_MSG ("Set Semaphore Timer \n");
                        SemaphoreTimer = 1;
                    }
                    sStatisticRP.UpdateState ( mcu.RtcGetTimeMs ( ) , RadioTaskId ) ;
                    RadioTaskId = sPriorityTask.HookId;
                    sTask [ RadioTaskId ].State = TASK_RUNNING;
                    LaunchCurrentTask ( );
                } // else case already managed during enqueue Task 
            } else { // radio is sleeping start priority task on radio
                RadioTaskId = sPriorityTask.HookId;
                sTask [ RadioTaskId ].State = TASK_RUNNING;
                LaunchCurrentTask ( );
            }
        }

        int tmp = (int) (sTask [ TimerHookId ].StartTime - CurrentTime ); // Have to keep Currentime else A priority task with delay > 0 could be aborted here 
        if (  ( tmp > 0 ) && ( tmp < MARGIN_DELAY )  && (GetNextStateStatus == HAVE_TO_SET_TIMER ) && ( TimerHookId !=RadioTaskId ) ){
            DEBUG_PRINTFRP ( " Aborted Task with hook id %d : Not a priority task\n ",TimerHookId );
            sTask [ TimerHookId ].State = TASK_ABORTED;
            if ( sTask [ RadioTaskId ].State != TASK_RUNNING ) { 
                CallAbortedTask () ;
            }
        }
        GetNextStateStatus = GetNextTask ( TimerValue, TimerHookId,  mcu.RtcGetTimeMs ( ) ) ;
        if ( GetNextStateStatus ==  HAVE_TO_SET_TIMER ) {// at this step still schedule task not done
            if ( TimerValue > MARGIN_DELAY ) { // if Timer is alway running i stop and restard the timer event if it is already the TimerHookid on the timer  
                SetAlarm ( TimerValue - MARGIN_DELAY ) ;
                DEBUG_PRINTFRP ("Set Timer Timer Value = %d on hood id = %d time = %d\n",TimerValue - MARGIN_DELAY ,TimerHookId, CurrentTime ) ;
            } else {

                SetAlarm ( MARGIN_DELAY ) ;
                DEBUG_PRINTFRP ("Set Timer Timer Value = %d on hood id = %d\n",1,TimerHookId   ) ;
            }
            
        }
    } else {
            DEBUG_MSGRP ( " No More Active Task inside the RadioPlaner \n" );
    }
// DEBUG_PRINTFRP ( " End of arbitrer Timerstate = %d , RadioState = %d  if Radio is Running task is %d \n", PlanerTimerState,  sTask [ RadioTaskId ].State, sTask [ RadioTaskId ].HookId );
}

/************************************************************************************/
/*                                 Timer Isr Routine                                */
/*                               Called when Alarm expires                          */
/************************************************************************************/

template <class R>     
void RadioPLaner<R>::IsrTimerRadioPlaner ( void ) {
    mcu.DisableIrq     ( );
    PlanerTimerState = TIMER_IDLE ;
    CallPlanerArbitrer (  __FUNCTION__ );
    mcu.EnableIrq      ( );
}              


/************************************************************************************/
/*                                 Radio Isr Routine                                */
/*                             Called when Radio raises It                          */
/************************************************************************************/
template <class R> 
void  RadioPLaner<R>::IsrRadioPlaner ( void ) {
    if ( SemaphoreTimer == 1 ) {
        SemaphoreTimer = 0;
        DEBUG_MSG (" Clea Semaphore Timer \n");
    } else {
    SemaphoreRadio = 1;
    uint32_t Now = mcu.RtcGetTimeMs( );
    DEBUG_PRINTFRP ( "Radio Duration %d\n", (Now - debugt1 ));
    IrqTimeStampMs [ RadioTaskId ] = Now ;
    DEBUG_PRINTFRP           ( " Receive It Radio for HookID = %d\n", RadioTaskId );
    GetIRQStatus             ( RadioTaskId );
    sStatisticRP.UpdateState ( IrqTimeStampMs [ RadioTaskId ], RadioTaskId ) ;
    FreeStask                ( sTask [ RadioTaskId ] );  // be careful have to free task before callback because call back can enqueue task and so call arbitrer
    Radio->Sleep             ( false ); // be careful put radio in sleep before callback because callback can restart the radio immediatly
    CallBackHook             ( RadioTaskId );
    CallAbortedTask          ( );
    SemaphoreRadio = 0;
    CallPlanerArbitrer       (   __FUNCTION__ );
    }
} 


/************************************************************************************/
/*                                 Planer Utilities                                 */
/*                                                                                  */
/************************************************************************************/
template <class R> 
uint8_t  RadioPLaner<R>::SelectPriorityTask ( uint32_t Now ) {
    uint8_t  HookToExecuteTmp = 0xFF;
    uint32_t TimeOfHookToExecuteTmp;
    uint32_t TempTime;
    uint8_t  index ;
    uint8_t k; 
    for ( k = 0; k < NB_HOOK; k++ ) {
        index = Ranking [ k ] ;
        if ( ( ( sTask [ index ].State < TASK_RUNNING ) && ( (int) (sTask [ index ].StartTime - Now >= 0) ) ) || ( sTask [ index ].State  == TASK_RUNNING ) ) {  // Mean   TASK_SCHEDULE or TASK_SOMETHING_TO_DO or TASK_RUNNING,
            HookToExecuteTmp        = sTask [ index ].HookId ;
            TimeOfHookToExecuteTmp  = sTask [ index ].StartTime ;
            break;
        }
    }
    if ( k == NB_HOOK ) {
        return ( NO_MORE_TASK );
    }
    for ( int i = k; i < NB_HOOK; i++ ) {
        index = Ranking [ i ] ;
        if ( ( ( sTask [ index ].State < TASK_RUNNING ) && ( (int) (sTask [ index ].StartTime - Now >= 0) ) )  || ( sTask [ index ].State  == TASK_RUNNING ) ) {  // Mean   TASK_SCHEDULE or TASK_SOMETHING_TO_DO or TASK_RUNNING,
            TempTime =  sTask[ index ].StartTime + sTask[ index ].TaskDuration ;
            int tmp = (int) ( TempTime - TimeOfHookToExecuteTmp );
            if ( ( tmp ) < 0 ) {   //@relative to avoid issues in case of wrapping
                TimeOfHookToExecuteTmp = sTask[ index ].StartTime ;
                HookToExecuteTmp       = sTask[ index ].HookId ;
            }
        }
    }
    sPriorityTask = sTask [ HookToExecuteTmp ];
    return ( SOMETHING_TO_DO );
}

template <class R> 
eGetNextStateStatus  RadioPLaner<R>::GetNextTask ( uint32_t& duration, uint8_t& TaskIdOut, uint32_t NowIn ) {
    uint8_t k;
    uint8_t  index ;
    uint32_t TempTime = NowIn;
    for ( k = 0 ; k < NB_HOOK ; k++ ) {
        if ( ( sTask [ k ].State == TASK_SCHEDULE ) && ( ( (int) ( sTask [ k ].StartTime - TempTime ) >= 0 ) ) ) {
            TempTime =  sTask [ k ].StartTime;  
            index    = k ;
            break ;
        }
    }
    if ( k == NB_HOOK ) {
        return ( NO_MORE_TASK_SCHEDULE );
    } 
    for ( uint8_t i = k; i < NB_HOOK; i++ ) {
        if ( ( sTask [ i ].State == TASK_SCHEDULE ) &&  ( (int) ( sTask [ i ].StartTime - TempTime ) < 0 )  && ( (int) ( sTask [ k ].StartTime - NowIn ) >= 0 ) )   {
            TempTime =  sTask [ i ].StartTime ; 
            index    = i ;
            break ;
        }
    }
    TaskIdOut = index;
    duration  = TempTime - NowIn ;
    return ( HAVE_TO_SET_TIMER ) ;
}

template <class R> 
void  RadioPLaner<R>::ComputeRanking ( void ) { //@tbd implementation should be optimized but very few hooks
    int i;
    uint8_t Index;
    DECLARE_ARRAY( uint8_t, NB_HOOK, RankTemp );
    for ( int i = 0; i < NB_HOOK; i++ ) {
        RankTemp [ i ] =  sTask [ i ].Priority;
    } 
    for ( i = 0 ; i < NB_HOOK; i ++ ) {
        Index = FindHighestPriority ( &( RankTemp [ 0 ] ),  NB_HOOK );
        RankTemp [ Index ] = 0xFF;
        Ranking [ i ] = Index ;
    }
}  

template <class R> 
uint8_t  RadioPLaner<R>::FindHighestPriority ( uint8_t * vec,  uint8_t length ) {
    uint8_t HighPrio = 0xFF ;
    uint8_t Index = 0;
    for ( int i = 0; i < length; i++ ) {
        if ( vec [ i ] <= HighPrio ) {
            HighPrio = vec [ i ];
            Index = i; 
        }
    }
    return ( Index );
}

template <class R> 
void RadioPLaner<R>::GetIRQStatus ( uint8_t HookIdIn ) {  
    IrqFlags_t IrqRadio  = ( Radio->GetIrqFlagsLora ( ) );
     DEBUG_PRINTFRP ( " Get Irq Status 0x%x\n", IrqRadio );
    Radio->ClearIrqFlagsLora( ); 
    switch ( IrqRadio ) {
        case SENT_PACKET_IRQ_FLAG    :
            RadioPlanerStatus [ HookIdIn ] = PLANER_TX_DONE ;  
            break;
        case RECEIVE_PACKET_IRQ_FLAG : 
            RadioPlanerStatus [ HookIdIn ] = PLANER_RX_PACKET;
            ReadRadioFifo ( sTask [ HookIdIn ] );
            break; 
        case RXTIMEOUT_IRQ_FLAG      : 
            RadioPlanerStatus [ HookIdIn ] = PLANER_RX_TIMEOUT;
            break;
        case BAD_PACKET_IRQ_FLAG :
            RadioPlanerStatus [ HookIdIn ] = PLANER_RX_CRC_ERROR;
            break;    
        default:
            DEBUG_PRINTFRP ( " Error in Get Irq Status 0x%x\n", IrqRadio );
            RadioPlanerStatus [ HookIdIn ] = PLANER_TASK_ABORTED;
            break;
    }
}

template <class R> 
eHookStatus  RadioPLaner<R>::ReadRadioFifo ( STask TaskIn ) { 
    eHookStatus status = HOOK_OK;
    uint8_t Id = TaskIn.HookId;
    if (TaskIn.TaskType == RX_LORA ) {
        Radio->FetchPayloadLora ( PayloadSize [ Id ],  Payload [ Id ], sRadioParam[Id].Snr, sRadioParam[Id].Rssi);
    } else if ( TaskIn.TaskType  == RX_FSK ) {
        Radio->FetchPayloadFsk ( PayloadSize [ Id ],  Payload [ Id ], sRadioParam[Id].Snr, sRadioParam[Id].Rssi);
    } else {
        status = HOOK_ID_ERROR;
    }
    return status;
}

template <class R>     
void RadioPLaner<R>::SetAlarm (uint32_t alarmInMs ) {
    mcu.StopTimerMsecond ( );
    mcu.StartTimerMsecond( &RadioPLaner<R>::CallbackIsrTimerRadioPlaner,this, alarmInMs);
}

template <class R> 
void  RadioPLaner<R>::UpdateTimeTaskASAP ( uint32_t CurrentTimeIn ) {
    for ( int i = 0 ; i < NB_HOOK ; i++ ) {
        if ( sTask [ i ].State == TASK_ASAP ) {
            sTask [ i ].StartTime = CurrentTimeIn ;
        }
    }
    if ( ( sTask [ RadioTaskId ].State == TASK_RUNNING ) && ( sTask [ RadioTaskId ].TaskType == RX_LORA ) ) {
        sTask [ RadioTaskId ].TaskDuration = CurrentTimeIn + MARGIN_DELAY + 2 - sTask [ RadioTaskId ].StartTime ;
        DEBUG_PRINTFRP (" Extended duration of task ih = %d time = %d\n" ,RadioTaskId, CurrentTimeIn ) ;
    }
}

template <class R> 
void  RadioPLaner<R>::CallAbortedTask ( void ) {
    for (int i = 0; i < NB_HOOK; i ++ ) {
        if ( sTask [ i ].State == TASK_ABORTED ) {
            DEBUG_PRINTFRP("callback for aborted hook %d\n",i);
            FreeStask ( sTask [ i ] );  // be careful have to free task before callback because call back can enqueue task and so call arbitrer
            RadioPlanerStatus [ i ] = PLANER_TASK_ABORTED ;
            CallBackHook ( i );
        }
    }
}

template <class R> 
void  RadioPLaner<R>::LaunchCurrentTask ( void ) {
    uint8_t Id = RadioTaskId;
    DEBUG_PRINTFRP ( " Launch Task ID %d and start Radio state = %d \n",Id, sTask[Id].State  );
    //PrintTask  ( sTask [ Id ] ); 
    switch ( sTask [ Id ].TaskType ) {
        case TX_LORA :
            #ifdef SX1272_BOARD  
                Radio->SendGen( Payload [ Id ], *PayloadSize [ Id ], sRadioParam [ Id ].Sf, sRadioParam [ Id ].Bw, sRadioParam [ Id ].Frequency, sRadioParam[ Id ].Power,  sRadioParam [ Id ].IqMode ,  sRadioParam [ Id ].CrcMode );            
            #else
               Radio->SendLora( Payload [ Id ], *PayloadSize [ Id ], sRadioParam [ Id ].Sf, sRadioParam [ Id ].Bw, sRadioParam [ Id ].Frequency, sRadioParam[ Id ].Power );
            #endif
            sStatisticRP.StartTxCounter ( ); 
            break;
        case RX_LORA :
            Radio->RxGen ( sRadioParam [ Id ].Bw, sRadioParam [ Id ].Sf, sRadioParam [ Id ].Frequency, sRadioParam [ Id ].TimeOutMs, sRadioParam [ Id ].IqMode );
            //Radio->RxLora ( sRadioParam [ Id ].Bw, sRadioParam [ Id ].Sf, sRadioParam [ Id ].Frequency, sRadioParam [ Id ].TimeOutMs);
            sStatisticRP.StartRxCounter ( ); 
            break;
        case TX_FSK :
        case RX_FSK :
        case CAD    :
        default :
            DEBUG_MSGRP("Error Radio Planer\n");
        break;
    }
    debugt1 = mcu.RtcGetTimeMs ();
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
