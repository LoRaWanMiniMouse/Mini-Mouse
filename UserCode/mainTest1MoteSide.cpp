/*!
 * \file      MainTest1.c
 *
 * \brief     Description : Set 2 hooks : MiniMouse Lorawan class A hook + Continuous RX hook
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
  __  __ _       _                                 
 |  \/  (_)     (_)                                
 | \  / |_ _ __  _ _ __ ___   ___  _   _ ___  ___  
 | |\/| | | '_ \| | '_ ` _ \ / _ \| | | / __|/ _ \
 | |  | | | | | | | | | | | | (_) | |_| \__ \  __/ 
 |_|  |_|_|_| |_|_|_| |_| |_|\___/ \__,_|___/\___| 
                                                   
                                                   
 * \endcode

Maintainer        : Fabien Holin (SEMTECH)
*/

#include "LoraMacDataStoreInFlash.h"
#include "LoraWanProcess.h"
#include "Define.h"
#include "utilities.h"
#include "UserDefine.h"
#include "appli.h"
#include "SX126x.h"
#include "ApiMcu.h"
#include "utilities.h"
#include "main.h"
#include "UserDefine.h"
#include "ApiMcu.h"
#include "RadioPlaner.h"

#define FileId 4
#define SG_DELAY 1000
SRadioParam  sRadioParamTXC;
STask staskRCMote ;
uint8_t UserTxPayload [125];
uint8_t UserTxPayloadSize;



void CallBackTxFoTestMote ( void * RadioPlanerIn) {
    #ifdef SX126x_BOARD
        RadioPLaner< SX126x > * RpTx;
        RpTx = reinterpret_cast< RadioPLaner< SX126x > * > (RadioPlanerIn);
    #endif
    #ifdef SX1276_BOARD
        RadioPLaner< SX1276 > * RpTx;
        RpTx = reinterpret_cast< RadioPLaner< SX1276 > * > (RadioPlanerIn);
    #endif
    #ifdef SX1272_BOARD
        RadioPLaner< SX1272 > * RpTx;
        RpTx = reinterpret_cast< RadioPLaner< SX1272 > * > (RadioPlanerIn);
    #endif
    DEBUG_MSG ( "  Tx For Test Done  Frequency = 864.525 Mhz, Sf7 , BW125 , NO_CRC, IQ NORMAL, Preambule 8, Lora Payload 20 \n ");
    uint32_t tCurrentMillisec;
    ePlanerStatus  PlanerStatusRxc;
    RpTx->GetStatusPlaner ( 0, tCurrentMillisec, PlanerStatusRxc );
    staskRCMote.StartTime      = mcu.RtcGetTimeMs ( ) + SG_DELAY;
    RpTx->EnqueueTask (staskRCMote, UserTxPayload, &UserTxPayloadSize, sRadioParamTXC ); //@tbd RadioPlaner  timeonair
}


int mainTest1Mote( ) {
    
    int i;
    mcu.InitMcu ( );
    #ifdef SX126x_BOARD
    #define FW_VERSION     0x18
        SX126x  RadioUser( LORA_BUSY, LORA_CS, LORA_RESET,TX_RX_IT );
        RadioPLaner < SX126x > RP( &RadioUser );
    #endif
    #ifdef SX1276_BOARD
    #define FW_VERSION     0x17
        SX1276  RadioUser( LORA_CS, LORA_RESET, TX_RX_IT, RX_TIMEOUT_IT);
        RadioPLaner < SX1276 > RP( &RadioUser );
    #endif
    #ifdef SX1272_BOARD
    #define FW_VERSION     0x13
        SX1272  RadioUser( LORA_CS, LORA_RESET, TX_RX_IT, RX_TIMEOUT_IT);
        RadioPLaner < SX1272 > RP( &RadioUser );
    #endif
        
        RP.InitHook ( 0 , &CallBackTxFoTestMote, reinterpret_cast <void * > (&RP) );

/*Launch Hook 1 in continuous reception */
        sRadioParamTXC.Frequency       = 864525000;
        sRadioParamTXC.Sf              = 7;
        sRadioParamTXC.Bw              = BW125;
        sRadioParamTXC.CrcMode         = CRC_NO;
        sRadioParamTXC.IqMode          = IQ_NORMAL;
        sRadioParamTXC.HeaderMode      = EXPLICIT_HEADER;
        sRadioParamTXC.PreambuleLength = 8;
        sRadioParamTXC.Modulation      = LORA;
        sRadioParamTXC.TimeOutMs       = 0;
        UserTxPayloadSize      = 20;
        for (int i = 0 ; i < UserTxPayloadSize ; i ++ ){
            UserTxPayload [ i ] = i;    
        }
        staskRCMote.HookId         = 0;
        staskRCMote.TaskDuration   = 1;
        staskRCMote.State          = TASK_SCHEDULE;
        staskRCMote.TaskType       = TX_LORA; 
        staskRCMote.StartTime      = mcu.RtcGetTimeMs ( ) + SG_DELAY;
        RP.EnqueueTask (staskRCMote, UserTxPayload, &UserTxPayloadSize, sRadioParamTXC ); 

/*Launch Hook 0 minimouse class a */
    while (1) {
    }
}