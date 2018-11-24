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
#include <string>   

template <  class R , uint32_t TAB_LENGTH> 
struct MMTab {
  std::string Name;
  R& operator[] (uint32_t index) {
      if ( index >= TAB_LENGTH ) {
        DEBUG_PRINTF(" ERROR Index of Tab named : %s is out of scope : Index MAX = %d and index is %d \n",Name.c_str(),TAB_LENGTH,index);
        while(1){}
        return Tab[0];
      } else {
        return Tab[index];
      };
  };
  R  Tab [ TAB_LENGTH ];
  //uint32_t MaxLength ;
};

#define INIT_TAB( Type, Length, Name )   MMTab < Type, Length > Name = {#Name " Declared In File " __FILE__ }

template < class R > 
class RadioPLaner  { 
public:
    RadioPLaner( R* RadioUser );
    ~RadioPLaner ( ); 
    eHookStatus InitHook         ( uint8_t HookIdIn,  void (* AttachCallBack) (void * ), void * objHookIn ) ;
    eHookStatus GetMyHookId      ( void * objHookIn, uint8_t * HookIdIn );
    eHookStatus EnqueueTask      ( STask* staskIn, uint8_t *payload, uint8_t *payloadSize, SRadioParam *sRadioParamIn );
    void        GetStatusPlaner  ( uint32_t * IrqTimestampMs, ePlanerStatus *PlanerStatus );
   
private :
 
  R*                Radio;     
  STask             sNextTask;
  INIT_TAB( SRadioParam, NB_HOOK, sRadioParam );
  INIT_TAB( STask      , NB_HOOK, sTask       );
  INIT_TAB( uint8_t*   , NB_HOOK, Payload     );
  INIT_TAB( uint8_t*   , NB_HOOK, PayloadSize );
  INIT_TAB( uint8_t    , NB_HOOK, Ranking     );
  INIT_TAB( void*      , NB_HOOK, objHook     );

  uint8_t           HookToExecute;
  uint32_t          TimeOfHookToExecute;
  ePlanerStatus     RadioPlanerStatus;
  ePlanerTimerState PlanerTimerState;
  uint8_t           RadioTaskId;  
  uint8_t           TimerTaskId;
  uint32_t          IrqTimeStampMs;          
  
/************************************************************************************/
/*                                 Planer Utilities                                 */
/*                                                                                  */
/************************************************************************************/
  void        UpdateTaskTab                   ( void );
  void        CallPlanerArbitrer              ( void );
  void        GetIRQStatus                    ( uint8_t HookIdIn );
  void        ComputeRanking                  ( void );
  void        LaunchCurrentTask               ( void );
  uint8_t     SelectTheNextTask               ( void );
  uint8_t     FindHighestPriority             ( uint8_t * vec, uint8_t length );
  eHookStatus Read_RadioFifo                  ( STask TaskIn );
  void        SetAlarm                        ( uint32_t alarmInMs ); 
  void        IsrTimerRadioPlaner             ( void );
  void        IsrRadioPlaner                  ( void ); // Isr routine implemented in IsrRoutine.cpp file
  void        AbortTaskInRadio                ( void );
  void        CallAbortedTAsk                 ( void );
  void        (* AttachCallBackHook[NB_HOOK]) (void * ) ;
  static void CallbackIsrTimerRadioPlaner     ( void * obj )   { ( reinterpret_cast<RadioPLaner<R>*>(obj) )->IsrTimerRadioPlaner(); };
  static void CallbackIsrRadioPlaner          ( void * obj )   { ( reinterpret_cast<RadioPLaner< R >*>(obj))->IsrRadioPlaner();} ; 
  void        CallBackHook                    (uint8_t HookId) { AttachCallBackHook[HookId] ( objHook[HookId] ); };  
/************************************************************************************/
/*                                 DEBUG Utilities                                 */
/*                                                                                  */
/************************************************************************************/
void PrintTask ( STask TaskIn);

};

#endif
