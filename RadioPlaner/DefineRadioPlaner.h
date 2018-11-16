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


#define NB_HOOK 4
typedef enum {
    TASK_IDLE, 
    TASK_TX_LORA,
    TASK_TX_FSK,
    TASK_RX_LORA,
    TASK_RX_FSK,
    TASK_CAD,
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

typedef enum { 
    TIMER_IDLE,
    TIMER_BUSY 
}ePlanerTimerState;


typedef enum { 
    RADIO_IDLE,
    RADIO_TX,
    RADIO_RX,
    RADIO_CAD 
}ePlanerRadioState;

#endif