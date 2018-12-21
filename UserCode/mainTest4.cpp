/*!
 * \file      MainTest4.c
 *
 * \brief     Description : Set 2 hooks : txperiodic 3 seconds + Continuous RX hook
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
int16_t RxcSnr ;
int16_t RxcRssi ;
SRadioParam  sRadioParamRXC;
STask staskRC ;
SRadioParam  sRadioParamTXP;
STask staskTxPeriodic ;
uint32_t PeriodicSchedule ; 
uint8_t UserTxPeriodicPayload [125];
uint8_t UserTxPeriodicPayloadSize;

uint8_t UserRxPayload [125];
uint8_t UserRxPayloadSize;



struct StatisticTest {
    uint32_t RxcCpt ;
    uint32_t TxcCpt ;
    uint32_t TxpCpt;
    uint32_t RxcCrcErrorCpt ;
    uint32_t RxcTimeOut ;
    uint32_t RxcAbortedCpt;
    uint32_t TxpAbortedCpt;
    uint32_t ClassARxCpt;
    uint32_t TestStartTimeSec;
    uint32_t TxClassACpt ;
    void PrintStatisticTest ( void ) {
        DEBUG_MSG    ("\n\n");
        DEBUG_MSG    ("*************************************************************************************************************\n");
        DEBUG_MSG    ("*********************************************Test Statistics ************************************************\n");
        DEBUG_MSG    ("*************************************************************************************************************\n");
        DEBUG_PRINTF ( "                                        Test Duration (Seconds)      = %d \n", mcu.RtcGetTimeSecond ( ) - TestStartTimeSec );
        DEBUG_PRINTF ( "                                        StatisticTest.RxcCpt         = %d/%d \n", RxcCpt,TxcCpt );
        DEBUG_PRINTF ( "                                        StatisticTest.ClassARxCpt    = %d/%d \n", ClassARxCpt,TxClassACpt );
        DEBUG_PRINTF ( "                                        StatisticTest.RxcCrcErrorCpt = %d \n", RxcCrcErrorCpt);
        DEBUG_PRINTF ( "                                        StatisticTest.RxcAbortedCpt  = %d \n", RxcAbortedCpt );
        DEBUG_PRINTF ( "                                        StatisticTest.TxpAbortedCpt  = %d \n", TxpAbortedCpt );
        DEBUG_PRINTF ( "                                        StatisticTest.TxpCpt         = %d \n", TxpCpt );
        DEBUG_PRINTF ( "                                        StatisticTest.RxcTimeOut     = %d \n", RxcTimeOut  );

        DEBUG_MSG    ("\n\n");
    }
} ;
StatisticTest sStatisticTest ;
void CallBackRxContinuous ( void * RadioPlanerIn) {
    RadioPLaner< SX1276 > * RpRxc;
    RpRxc = reinterpret_cast< RadioPLaner< SX1276 > * > (RadioPlanerIn);
    //DEBUG_MSG ( " call CallBackRxContinuous\n ");
    uint32_t tCurrentMillisec;
    ePlanerStatus  PlanerStatusRxc;
    RpRxc->GetStatusPlaner ( staskRC.HookId, tCurrentMillisec, PlanerStatusRxc );
    switch ( PlanerStatusRxc ) {
        case PLANER_RX_PACKET : 
           /*  DEBUG_PRINTF ( " Recive Packet for Hook Rxc Continous Rssi = %d: { ",*sRadioParamRXC.Rssi );
            for ( int i = 0 ; i < UserRxPayloadSize ; i ++ ) {
                DEBUG_PRINTF (" %x ", UserRxPayload [ i ] );
            }
            DEBUG_MSG ( " } \n "); */
            sStatisticTest.RxcCpt ++ ;
            sStatisticTest.TxcCpt = UserRxPayload [ 0 ] + ( UserRxPayload [ 1 ] << 8 )  + ( UserRxPayload [ 2 ] << 16 )  + ( UserRxPayload [ 3 ] << 24 ) ; 
            break;
        case PLANER_RX_TIMEOUT : 
            //DEBUG_MSG ( " Receive timeOut on thread rx continuous " ) ;
            sStatisticTest.RxcTimeOut ++;
            break;
        case PLANER_RX_CRC_ERROR :    
         //  DEBUG_MSG ( " Receive A packet with CRC ERROR on thread rx continuous " ) ;
            sStatisticTest.RxcCrcErrorCpt ++ ;
            break;
        case PLANER_TASK_ABORTED :
            sStatisticTest.RxcAbortedCpt ++;
          //  DEBUG_PRINTF ( " Task with  HOOK ID = %d  ABORTED \n",staskRC.HookId);
           // DEBUG_PRINTF ( " Planer status for task 1 = %d\n",PlanerStatusRxc ) ;
            break;
        default :
            break;
    }
    staskRC.StartTime      = mcu.RtcGetTimeMs ( );
    RpRxc->EnqueueTask (staskRC, UserRxPayload, &UserRxPayloadSize, sRadioParamRXC ); //@tbd RadioPlaner  timeonair
}

void CallBackTxPeriodic ( void * RadioPlanerIn) {
   
    RadioPLaner< SX1276 > * RpTxPeriodic;
    RpTxPeriodic = reinterpret_cast< RadioPLaner< SX1276 > * > (RadioPlanerIn);
    uint32_t tCurrentMillisect;
    ePlanerStatus  PlanerStatusTxp;
    RpTxPeriodic->GetStatusPlaner ( staskTxPeriodic.HookId, tCurrentMillisect, PlanerStatusTxp );
    if ( PlanerStatusTxp == PLANER_TASK_ABORTED ) {
      //  DEBUG_PRINTF ( "  \n Task with HOOK ID = %d  ABORTED \n",staskTxPeriodic.HookId);
        sStatisticTest.TxpAbortedCpt ++;
    } else {
        sStatisticTest.TxpCpt ++;
    }
    while (int (PeriodicSchedule - mcu.RtcGetTimeMs ( ) ) < 1000 ){
        PeriodicSchedule += 3000 ;
    }
    staskTxPeriodic.StartTime      = PeriodicSchedule;
    RpTxPeriodic->EnqueueTask (staskTxPeriodic, UserTxPeriodicPayload, &UserTxPeriodicPayloadSize, sRadioParamTXP ); //@tbd RadioPlaner  timeonair
}



int mainTest4( ) {
    
    uint8_t AppTimeSleeping = 10;
    mcu.InitMcu ( );
    sStatisticTest.TxcCpt         = 0;
    sStatisticTest.RxcCpt         = 0;
    sStatisticTest.RxcCrcErrorCpt = 0;
    sStatisticTest.RxcAbortedCpt  = 0;
    sStatisticTest.TxpAbortedCpt  = 0;
    sStatisticTest.TxpCpt         = 0;
    sStatisticTest.ClassARxCpt    = 0;
    sStatisticTest.RxcTimeOut     = 0;
    sStatisticTest.TxClassACpt    = 0;
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
          
        RP.InitHook ( 2 , &CallBackRxContinuous, reinterpret_cast <void * > (&RP) );
        RP.InitHook ( 1 , &CallBackTxPeriodic, reinterpret_cast <void * > (&RP) );
        RadioUser.Reset();
        sStatisticTest.TestStartTimeSec = mcu.RtcGetTimeSecond ();
    

/*Launch Hook 1 in continuous reception */
        sRadioParamRXC.Frequency       = 860525000;
        sRadioParamRXC.Sf              = 7;
        sRadioParamRXC.Bw              = BW125;
        sRadioParamRXC.CrcMode         = CRC_NO;
        sRadioParamRXC.IqMode          = IQ_NORMAL;
        sRadioParamRXC.HeaderMode      = EXPLICIT_HEADER;
        sRadioParamRXC.PreambuleLength = 8;
        sRadioParamRXC.Modulation      = LORA;
        sRadioParamRXC.TimeOutMs       = 10000;
        sRadioParamRXC.Snr             = &RxcSnr;
        sRadioParamRXC.Rssi            = &RxcRssi;
        staskRC.HookId         = 2;
        staskRC.TaskDuration   = 3;
        staskRC.State          = TASK_ASAP;
        staskRC.TaskType       = RX_LORA; 
        staskRC.StartTime      = mcu.RtcGetTimeMs ( );
        RP.EnqueueTask (staskRC, UserRxPayload, &UserRxPayloadSize, sRadioParamRXC ); 

/*Launch Hook 2 Tx at fix periodi period */

        sRadioParamTXP.Frequency       = 867000000;
        sRadioParamTXP.Sf              = 9;
        sRadioParamTXP.Bw              = BW125;
        sRadioParamTXP.CrcMode         = CRC_YES;
        sRadioParamTXP.IqMode          = IQ_NORMAL;
        sRadioParamTXP.HeaderMode      = EXPLICIT_HEADER;
        sRadioParamTXP.PreambuleLength = 8;
        sRadioParamTXP.Modulation      = LORA;
        sRadioParamTXP.TimeOutMs       = 0;
        UserTxPeriodicPayloadSize      = 20;
        for (int i = 0 ; i < UserTxPeriodicPayloadSize ; i ++ ){
            UserTxPeriodicPayload [ i ] = 0;    
        }
        staskTxPeriodic.HookId         = 1;
        staskTxPeriodic.TaskDuration   = 300;
        staskTxPeriodic.State          = TASK_SCHEDULE;
        staskTxPeriodic.TaskType       = TX_LORA;
        PeriodicSchedule                = mcu.RtcGetTimeMs ( ) + 1000;
        staskTxPeriodic.StartTime      = PeriodicSchedule ;
        RP.EnqueueTask (staskTxPeriodic, UserTxPeriodicPayload, &UserTxPeriodicPayloadSize, sRadioParamTXP ); 

/*Launch Hook 0 minimouse class a */
       static uint32_t time = mcu.RtcGetTimeSecond () ;
        mcu.MMClearDebugBufferRadioPlaner ( );
        while ( 1 ){

            if ( ( mcu.RtcGetTimeSecond () - time ) > 5 ) {
                time = mcu.RtcGetTimeSecond ();
                RP.GetStatistic ( );
                sStatisticTest.PrintStatisticTest();
            }
            mcu.GotoSleepMSecond ( AppTimeSleeping * 1000 );
        }
             
}