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
};
typedef enum {
    TASK_AT_TIME,
    TASK_NOW,
    TASK_ASSAP, 
}eTimingTypeTask;

struct STask {
    uint8_t  HookId ;
    uint32_t StartTime ; // absolute Ms
    uint32_t TaskDuration  ;  
    eTimingTypeTask TaskTimingType ;
};


#define NB_HOOK 4
/* to be discuss the following enum order is important because it define the priority order Min Num = hiest priority) */
typedef enum {
    TASK_RX_LORA,
    TASK_RX_FSK, 
    TASK_TX_LORA,
    TASK_TX_FSK,
    TASK_CAD,
    TASK_IDLE,
}eRadioPlanerTask;


typedef enum { 
    RADIO_IN_TX_LORA,
    RADIO_IN_TX_FSK,
    RADIO_IN_RX_LORA,
    RADIO_IN_RX_FSK,
    RADIO_IN_CAD,
    RADIO_IN_IDLE,
}eRadioState;

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
    INIT_HOOK_OK,
    INIT_HOOK_ERROR 
}ePlanerInitHookStatus;

typedef enum { 
    TIMER_IDLE,
    TIMER_BUSY 
}ePlanerTimerState;



enum { 
    HOOK_0,
    HOOK_1,
    HOOK_2,
    HOOK_3
};
#endif