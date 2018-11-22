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
 * \remark        : 
 * Each Tasks enqueued inside the radio planer should Provide :
 *              Task descriptor : 
                                    struct STask {
                                        uint8_t           HookId ;
                                        uint32_t          StartTime ; // absolute Ms
                                        uint32_t          TaskDuration  ;  
                                        eTimingTypeTask   TaskTimingType ;
                                        eRadioPlanerTask  TaskType  ; 
                                    };

                RadioParameter:     struct SRadioParam {
                                            uint32_t             Frequency;
                                            eBandWidth           Bw;
                                            uint8_t              Sf;
                                            uint8_t              Power;
                                            eCrcMode             CrcMode;
                                            eIqMode              IqMode;
                                            eHeaderMode          HeaderMode;
                                            uint8_t              PreambuleLength;
                                            eModulationType      Modulation;
                                };
                BufferParameter : *Buffer + BufferSize 
         
                                                                                                

*/
#ifndef RADIOPLANER_H
#define RADIOPLANER_H
#include "Define.h"
#include "DefineRadioPlaner.h"


template < class R > 
class RadioPLaner  { 
public:
    RadioPLaner( R* RadioUser );
    ~RadioPLaner ( ); 
    eHookStatus InitHook     ( uint8_t HookIdIn,  void (* AttachCallBack) (void * ), void * objHookIn ) ;
    eHookStatus GetMyHookId  ( void * objHookIn, uint8_t * HookIdIn );


    /* Note : @tbd : return a status enqueue error case if task already running */
 

    void EnqueueTask     ( STask* staskIn, uint8_t *payload, uint8_t *payloadSize, SRadioParam *sRadioParamIn );
    void GetStatusPlaner ( uint32_t * IrqTimestampMs, ePlanerStatus *PlanerStatus );
   

//private :

  void (* AttachCallBackHook[NB_HOOK]) (void * ) ;
  void * objHook[NB_HOOK];
  void CallBackHook (uint8_t HookId) { 
          AttachCallBackHook[HookId] ( objHook[HookId] ); 
  }     
  SRadioParam       sRadioParam   [ NB_HOOK ];
  STask             sTask         [ NB_HOOK ];
  STask             sRadioRunningTask;
  STask             sTimerRunningTask;
  STask             sNextTask;
  uint8_t*          Payload       [ NB_HOOK ];
  uint8_t*          PayloadSize   [ NB_HOOK ]; 
  uint8_t           HookToExecute;
  uint32_t          TimeOfHookToExecute;
  
/************************************************************************************/
/*                                 Planer Utilities                                 */
/*                                                                                  */
/************************************************************************************/
  void UpdateTaskTab          ( void );
  void CallPlanerArbitrer     ( void );
  void GetIRQStatus           ( uint8_t HookIdIn );

  void ComputeRanking         ( void );
  void LaunchCurrentTask      ( void );
  uint8_t SelectTheNextTask   ( void );
  uint8_t FindHighestPriority ( uint8_t * vec, uint8_t length );
  uint8_t Ranking [ NB_HOOK ]; 

  eHookStatus Read_RadioFifo ( STask TaskIn );
  ePlanerStatus RadioPlanerStatus;
/*     isr  Timer Parameters */
        
  ePlanerTimerState PlanerTimerState;
  //void              LaunchTimer ( void );
  void              SetAlarm                    ( uint32_t alarmInMs ); 
  void              IsrTimerRadioPlaner         ( void );
  static void       CallbackIsrTimerRadioPlaner ( void * obj ) { ( reinterpret_cast<RadioPLaner<R>*>(obj) )->IsrTimerRadioPlaner(); };


/*     isr Radio Parameter   */
  uint32_t          IrqTimeStampMs;

  void IsrRadioPlaner                ( void ); // Isr routine implemented in IsrRoutine.cpp file
  void AbortTaskInRadio              ( void );
  void CallAbortedTAsk               ( void );
  static void CallbackIsrRadioPlaner (void * obj){(reinterpret_cast<RadioPLaner< R >*>(obj))->IsrRadioPlaner();} ; 
  
  R* Radio;


/************************************************************************************/
/*                                 DEBUG Utilities                                 */
/*                                                                                  */
/************************************************************************************/
void PrintTask ( STask TaskIn);




};

#endif
