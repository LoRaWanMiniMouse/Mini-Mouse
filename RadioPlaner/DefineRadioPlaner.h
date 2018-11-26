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
#ifndef DEFINE_RADIOPLANER_H
#define DEFINE_RADIOPLANER_H
#include "Define.h"
#define NB_HOOK 4
#define RadioPlanerTimeOut 20000 // A task couldn't be stay inside the Radioplaner more than 20 second except the background tasks . 
struct SRadioParam {
    uint32_t             Frequency;
    eBandWidth           Bw;
    uint8_t              Sf;
    uint8_t              Power;
    eCrcMode             CrcMode;
    eIqMode              IqMode;
    eHeaderMode          HeaderMode;
    uint8_t              PreambuleLength;
    eModulationType      Modulation;
    uint32_t             TimeOutMs;
    int16_t *            Snr;
    int16_t *            Rssi;
};


typedef enum {
    RX_LORA,
    RX_FSK, 
    TX_LORA,
    TX_FSK,
    CAD,
    NONE,
}eRadioPlanerTask;

typedef enum {
    TASK_SCHEDULE,
    TASK_ASAP,
    TASK_RUNNING,
    TASK_ABORTED,
    TASK_FINISHED,
}PlanerState;

struct STask {
    uint8_t           HookId ;
    uint32_t          StartTime ; // absolute Ms
    uint32_t          TaskDuration  ;  
    eRadioPlanerTask  TaskType  ;
    PlanerState       State;
    uint8_t           Priority; 
    uint8_t           TokenDebug;
};

typedef enum { 
    PLANER_RX_CANCELED, 
    PLANER_TX_CANCELED, 
    PLANER_RX_CRC_ERROR,
    PLANER_CAD_POSITIVE,
    PLANER_CAD_NEGATIVE,
    PLANER_TX_DONE,
    PLANER_RX_PACKET,
    PLANER_RX_TIMEOUT 
} ePlanerStatus;

typedef enum { 
    HOOK_OK,
    HOOK_ID_ERROR,
    TASK_ALREADY_RUNNING, 
}eHookStatus;

typedef enum { 
    TIMER_IDLE,
    TIMER_BUSY 
}ePlanerTimerState;


#define NO_MORE_TASK      0
#define SCHEDULED_TASK    1
#define MARGIN_DELAY      3  // for 3 ms
#define MARGIN_DELAY_NEG -500  // for 500 ms
#endif