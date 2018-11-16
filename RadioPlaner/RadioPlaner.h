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
#ifndef RADIOPLANER_H
#define RADIOPLANER_H
#include "Define.h"
#include "DefineRadioPlaner.h"


template < class R > 
class RadioPLaner  { 
public:
    RadioPLaner( R* RadioUser );
    ~RadioPLaner ( ); 
    ePlanerInitHookStatus InitHook     ( uint8_t HookId,  void (* AttachCallBack) (void * ), void * objHook ) ;
    ePlanerInitHookStatus GetMyHookId  ( void * objHook, uint8_t * HookId );


    /* Note : StartTime , EndTime : relative value to be discuss */

    void Send ( STask stask, uint8_t *payload, uint8_t payloadSize, SRadioParam sRadioParamIn );
    void Rx   ( uint8_t HookId, uint32_t StartTime , SRadioParam sRadioParamIn, uint16_t TimeOutMsec );
    void GetStatusPlaner ( uint32_t * IrqTimestampMs, ePlanerStatus *PlanerStatus );
    void FetchPayload    ( uint8_t *payloadSize, uint8_t payload[255], int16_t *snr, int16_t *signalRssi);
    
    
    
      
   

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

  eRadioPlanerTask  TaskType          [ NB_HOOK ];
  uint32_t          StartTimeTask     [ NB_HOOK ];
  uint32_t          DurationTask      [ NB_HOOK ];

  void CallPlanerArbitrer  ( void );
  void LaunchTask          ( void );
  void ComputePlanerStatus ( void );
  ePlanerStatus RadioPlanerStatus;
/*     isr  Timer Parameters */
           


  ePlanerTimerState PlanerTimerState;
  SRadioParam       sRadioParam   [ NB_HOOK ];
  eBandWidth        Bw            [ NB_HOOK ];
  uint8_t           Sf            [ NB_HOOK ];
  uint32_t          Channel       [ NB_HOOK ];
  uint16_t          TimeOutMs     [ NB_HOOK ];
  uint8_t*          Payload       [ NB_HOOK ];
  uint8_t           PayloadSize   [ NB_HOOK ];
  uint8_t           Power         [ NB_HOOK ];
  uint8_t           CurrentHookToExecute;
  
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
