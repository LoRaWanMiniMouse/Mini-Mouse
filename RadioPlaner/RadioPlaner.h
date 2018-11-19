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
    eHookStatus InitHook     ( uint8_t HookId,  void (* AttachCallBack) (void * ), void * objHook ) ;
    eHookStatus GetMyHookId  ( void * objHook, uint8_t * HookId );


    /* Note : @tbd : return a status enqueue error case if task already running */
 

    void EnqueueTask     ( STask staskIn, uint8_t *payload, uint8_t *payloadSize, SRadioParam sRadioParamIn );
    void GetStatusPlaner ( uint32_t * IrqTimestampMs, ePlanerStatus *PlanerStatus );
   

private :

  void (* AttachCallBackHook0) (void * ) ;
  void * objHook0;
  void (* AttachCallBackHook1) (void * ) ;
  void * objHook1;
  void (* AttachCallBackHook2) (void * ) ;
  void * objHook2;
  void (* AttachCallBackHook3) (void * ) ;
  void * objHook3;
  void CallBackHook0 (void) { AttachCallBackHook0 ( objHook0 ); };   
  void CallBackHook1 (void) { AttachCallBackHook1 ( objHook1 ); };  
  void CallBackHook2 (void) { AttachCallBackHook2 ( objHook2 ); };  
  void CallBackHook3 (void) { AttachCallBackHook3 ( objHook3 ); };  
  
  SRadioParam       sRadioParam   [ NB_HOOK ];
  STask             sTask         [ NB_HOOK ];
  uint8_t*          Payload       [ NB_HOOK ];
  uint8_t*          PayloadSize   [ NB_HOOK ]; 
  uint8_t           CurrentHookToExecute;
  

  void CallPlanerArbitrer  ( void );
  void LaunchTask          ( void );
  void ComputePlanerStatus ( void );
  
  eHookStatus Read_RadioFifo ( eRadioPlanerTask  TaskType );
  ePlanerStatus RadioPlanerStatus;
/*     isr  Timer Parameters */
        
  ePlanerTimerState PlanerTimerState;

  void              SetAlarm                    ( uint32_t alarmInMs ); 
  void              IsrTimerRadioPlaner         ( void );
  static void       CallbackIsrTimerRadioPlaner ( void * obj ) { ( reinterpret_cast<RadioPLaner<R>*>(obj) )->IsrTimerRadioPlaner(); };


/*     isr Radio Parameter   */
  eRadioState       RadioPlanerState;
  uint32_t          IrqTimeStampMs;
  eRadioState       CurrentRadioState;
  void IsrRadioPlaner                ( void ); // Isr routine implemented in IsrRoutine.cpp file
  static void CallbackIsrRadioPlaner (void * obj){(reinterpret_cast<RadioPLaner< R >*>(obj))->IsrRadioPlaner();} ; 
  
  R* Radio;


};

#endif
