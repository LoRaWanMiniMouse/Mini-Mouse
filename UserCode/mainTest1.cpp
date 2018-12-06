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
            DEBUG_PRINTF ( " Recive Packet for Hook Rxc Continous Rssi = %d: { ",*sRadioParamRXC.Rssi );
            for ( int i = 0 ; i < UserRxPayloadSize ; i ++ ) {
                DEBUG_PRINTF (" %x ", UserRxPayload [ i ] );
            }
            DEBUG_MSG ( " } \n ");
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



int mainTest1( ) {
    uint8_t LoRaMacNwkSKeyInit[]      = { 0x22, 0x33, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11};
    uint8_t LoRaMacAppSKeyInit[]      = { 0x11, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22};
    uint8_t LoRaMacAppKeyInit[]       = { 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0xBB};
    uint8_t AppEuiInit[]              = { 0x70, 0xb3, 0xd5, 0x7e, 0xd0, 0x00, 0xff, 0x50 };
    uint8_t DevEuiInit[]              = { 0x38, 0x35, 0x31, 0x31, 0x18, 0x47, 0x37, 0x56 };    
    uint32_t LoRaDevAddrInit          = 0x26011920;
    int i;
    uint8_t UserFport ;
    uint8_t UserRxFport ; 
    uint8_t AppTimeSleeping = 10;
   
    sLoRaWanKeys  LoraWanKeys  = { LoRaMacNwkSKeyInit, LoRaMacAppSKeyInit, LoRaMacAppKeyInit, AppEuiInit, DevEuiInit, LoRaDevAddrInit,OTA_DEVICE };
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
        mcu.WatchDogStart ( );
        //uint8_t uid[8];
        //mcu.GetUniqueId (uid); 
        //memcpy(&LoraWanKeys.DevEui[0], uid , 8);
        /*!
        * \brief   Lp<LoraRegionsEU>: A LoRaWan Object with Eu region's rules. 
        * \remark  The Current implementation  support radio SX1276 and sx1261
        */
    #ifdef SX126x_BOARD
        LoraWanObject<LoraRegionsEU,SX126x> Lp ( LoraWanKeys,&RP,USERFLASHADRESS); 
    #endif
    #ifdef SX1276_BOARD
        LoraWanObject<LoraRegionsEU,SX1276> Lp ( LoraWanKeys,&RP,USERFLASHADRESS); 
    #endif
    #ifdef SX1272_BOARD
        LoraWanObject<LoraRegionsEU,SX1272> Lp ( LoraWanKeys,&RP,USERFLASHADRESS); 
    #endif
        //SX126x  RadioUser( LORA_BUSY, LORA_CS, LORA_RESET,TX_RX_IT );
        //LoraWanObject<LoraRegionsEU,SX126x> Lp( LoraWanKeys,&RadioUser,USERFLASHADRESS); 
        uint8_t AvailableRxPacket  = NO_LORA_RXPACKET_AVAILABLE ;
        eLoraWan_Process_States LpState = LWPSTATE_IDLE;  
        /*!
        * \brief Restore the LoraWan Context
        */
        //DEBUG_PRINTF("MM is starting ...{ %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x } \n",uid[0],uid[1],uid[2],uid[3],uid[4],uid[5],uid[6],uid[7]);
        RP.InitHook ( 0 , &(Lp.packet.Phy.CallbackIsrRadio), &(Lp.packet.Phy) );
        RP.InitHook ( 1 , &CallBackRxContinuous, reinterpret_cast <void * > (&RP) );
        RP.InitHook ( 2 , &CallBackTxPeriodic, reinterpret_cast <void * > (&RP) );
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
        staskRC.HookId         = 1;
        staskRC.TaskDuration   = 100;
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
        staskTxPeriodic.HookId         = 2;
        staskTxPeriodic.TaskDuration   = 300;
        staskTxPeriodic.State          = TASK_SCHEDULE;
        staskTxPeriodic.TaskType       = TX_LORA;
        PeriodicSchedule                = mcu.RtcGetTimeMs ( ) + 1000;
        staskTxPeriodic.StartTime      = PeriodicSchedule ;
        RP.EnqueueTask (staskTxPeriodic, UserTxPeriodicPayload, &UserTxPeriodicPayloadSize, sRadioParamTXP ); 

/*Launch Hook 0 minimouse class a */
        Lp.RestoreContext  ( );
        Lp.SetDataRateStrategy ( STATIC_ADR_MODE );
        UserFport       = 3;
        uint8_t UserPayloadSizeClassA = 14;
        uint8_t UserPayloadClassA [ 30 ];
        for (int i = 0 ; i < UserPayloadSizeClassA ; i++ ) {
            UserPayloadClassA [i]= i;
        }
        UserPayloadClassA [ 0 ]  = FW_VERSION ;
        uint8_t MsgTypeClassA = CONF_DATA_UP;
        Lp.NewJoin();
        while(1) {
            /*!
            * \brief  For this example : send an un confirmed message on port 3 . The user payload is a ramp from 0 to 13 (14 bytes) + FW version. 
            */
            
            if ( ( Lp.IsJoined ( ) == NOT_JOINED ) && ( Lp.GetIsOtaDevice ( ) == OTA_DEVICE) ) {       
                LpState  = Lp.Join( );
            } else {
                LpState  = Lp.SendPayload( UserFport, UserPayloadClassA, UserPayloadSizeClassA, MsgTypeClassA );
                sStatisticTest.TxClassACpt++;

            }

    /*!
    * \brief 
    *        This function manages the state of the MAC and performs all the computation intensive (crypto) tasks of the MAC.
    *        This function is called periodically by the user�s application whenever the state of the MAC is not idle.
    *        The function is not timing critical, can be called at any time and can be interrupted by any IRQ (including the user�s IRQ).
    *        The only requirement is that this function must be called at least once between the end of the transmission and the beginning of the RX1 slot.
    *        Therefore when the stack is active a call periodicity of roughly 300mSec is recommended.
    */ 
            DEBUG_MSG (" new packet \n");
            while ( ( LpState != LWPSTATE_IDLE ) && ( LpState != LWPSTATE_ERROR ) && ( LpState != LWPSTATE_INVALID ) ) {
                LpState = Lp.LoraWanProcess( &AvailableRxPacket );
                mcu.GotoSleepMSecond ( 100 );
                mcu.WatchDogRelease  ( );
            }
            RP.GetStatistic ( );
            if ( LpState == LWPSTATE_ERROR )  {
                InsertTrace ( __COUNTER__, FileId );
                // user application have to save all the need
                NVIC_SystemReset();
            }
            if ( AvailableRxPacket != NO_LORA_RXPACKET_AVAILABLE ) { 
                AvailableRxPacket  = NO_LORA_RXPACKET_AVAILABLE ;
                sStatisticTest.ClassARxCpt ++;
                InsertTrace ( __COUNTER__, FileId );
                Lp.ReceivePayload( &UserRxFport, UserRxPayload, &UserRxPayloadSize );
                DEBUG_PRINTF("Receive on port %d  an Applicative Downlink \n DATA[%d] = [ ",UserRxFport,UserRxPayloadSize);
                for ( i = 0 ; i < UserRxPayloadSize ; i++){
                    DEBUG_PRINTF( "0x%.2x ",UserRxPayload[i]);
                }
            }
            sStatisticTest.PrintStatisticTest();
        
    /*
    * \brief Send a �Packet every 120 seconds in case of join 
    *        Send a packet every AppTimeSleeping seconds in normal mode
    */
            if ( ( Lp.IsJoined ( ) == NOT_JOINED ) && ( Lp.GetIsOtaDevice ( ) == OTA_DEVICE) && ( LpState != LWPSTATE_INVALID)){
                InsertTrace ( __COUNTER__, FileId );
                mcu.GotoSleepSecond ( 5 );
            } else {
                InsertTrace ( __COUNTER__, FileId );
                mcu.GotoSleepMSecond ( AppTimeSleeping * 1000 );
                InsertTrace ( __COUNTER__, FileId );
            }
        }
}