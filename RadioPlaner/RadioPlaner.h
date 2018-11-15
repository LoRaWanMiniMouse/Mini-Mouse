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

#define NB_HOOK 1
typedef enum { 
    SEND_LORA,
    SEND_FSK,
    RX_LORA,
    RX_FSK,
    CAD,
}eRadioPlanerTask;

typedef enum { 
    RADIO_IN_TX,
    RADIO_IN_RX,
    RADIO_IN_CAD,
    RADIO_IN_IDLE,
}eRadioState;

typedef enum { 
    PLANER_REQUEST_DONE,
    PLANER_REQUEST_CANCELED, 
}ePlanerStatus;

typedef enum { 
    INIT_HOOK_OK,
    INIT_HOOK_ERROR 
}ePlanerInitHookStatus;
template < class R > 
class RadioPLaner  { 
public:
    RadioPLaner( R* RadioUser );
    ~RadioPLaner ( ); 
    ePlanerInitHookStatus InitHook ( uint8_t HookId,  void (* AttachCallBack) (void * ), void * objHook ) ;

    void       SendLora( uint8_t *payload, uint8_t payloadSize, uint8_t SF, eBandWidth BW, uint32_t channel, int8_t power );
    void       SendFsk ( uint8_t *payload, uint8_t payloadSize, uint32_t channel, int8_t power );
    void       RxLora  ( uint32_t TimetoRadioPlaner , eBandWidth BW, uint8_t SF, uint32_t channel, uint16_t TimeOutMs );
    void       RxFsk   ( uint32_t TimetoRadioPlaner , uint32_t channel, uint16_t timeout );
    IrqFlags_t GetStatusPlaner ( uint32_t * IrqTimestampMs, ePlanerStatus *PlanerStatus );
    void       FetchPayloadLora( uint8_t *payloadSize, uint8_t payload[255], int16_t *snr, int16_t *signalRssi);
    void       FetchPayloadFsk ( uint8_t *payloadSize, uint8_t payload[255], int16_t *snr, int16_t *signalRssi);
    
    
      
   

private :

    void (* AttachCallBackHook0) (void * ) ;
    void * objHook0;
/*     isr  Timer Parameters */
           
  eBandWidth        NextBW        [ NB_HOOK ];
  uint8_t           NextSF        [ NB_HOOK ];
  uint32_t          NextChannel   [ NB_HOOK ];
  uint16_t          NextTimeOutMs [ NB_HOOK ];
  eRadioPlanerTask  NextTask      [ NB_HOOK ];
  uint8_t           CurrentHookToExecute;
  
  void              SetAlarm                    ( uint32_t alarmInMs ); 
  void              IsrTimerRadioPlaner         ( void );
  static void       CallbackIsrTimerRadioPlaner ( void * obj ) { ( reinterpret_cast<RadioPLaner<R>*>(obj) )->IsrTimerRadioPlaner(); };


/*     isr Radio Parameter   */
  uint32_t          IrqTimeStampMs;
  eRadioState       CurrentRadioState;
  void IsrRadioPlaner                ( void ); // Isr routine implemented in IsrRoutine.cpp file
  static void CallbackIsrRadioPlaner (void * obj){(reinterpret_cast<RadioPLaner< R >*>(obj))->IsrRadioPlaner();} ; 

  
  void CallBackHook0 (void) { AttachCallBackHook0 ( objHook0 ); };   

    R* Radio;
};

#endif
