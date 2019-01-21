#include "ApiMcu.h"
#include "Define.h"
#include "PointToPointReceiver.h"
#include "PointToPointTransmitter.h"
#include "RadioPlaner.h"
#include "UserDefine.h"
#include "appli.h"
#include "main.h"
#include "sx1276.h"
#include "utilities.h"
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

#define MAX_PAYLOAD_RECEIVED 255
#define TX_ON_RX3_ID              0
#define LP_HOOK_ID                1
#define POINT_TO_POINT_TX_HOOK_ID 2
#define POINT_TO_POINT_RX_HOOK_ID 3

#define FW_VERSION 18

Relay relay;

#define FileId 4
int16_t RxcSnr ;
int16_t RxcRssi ;
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



void PrintPtpStatistics(const StatisticCounters_t* stats,
                        const uint16_t counter);


void CallBackTxOnRx3 ( void * RadioPlanerIn) {

}


int mainPtPRxTx( void ) {
uint8_t LoRaMacNwkSKeyInit[]      = { 0x22, 0x33, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11};
uint8_t LoRaMacAppSKeyInit[]      = { 0x11, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22};
uint8_t LoRaMacAppKeyInit[]       = { 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0xBB};
uint8_t AppEuiInit[]              = { 0x70, 0xb3, 0xd5, 0x7e, 0xd0, 0x00, 0xff, 0x50 };
uint8_t DevEuiInit[]              = { 0x38, 0x35, 0x31, 0x31, 0x18, 0x47, 0x37, 0x57 };    
#if BLOC
uint32_t LoRaDevAddrInit            = 0x26011632;
#else
uint32_t LoRaDevAddrInit          = 0x26011D16;
//uint32_t LoRaDevAddrInit          = 0x26011B67;

#endif
int i;
uint8_t UserFport ;
uint8_t UserRxFport ; 
uint8_t AppTimeSleeping = 10;
uint8_t payload_received[MAX_PAYLOAD_RECEIVED] = { 0x00 };
uint8_t payload_received_size = 0;
uint8_t payload_send[MAX_PAYLOAD_RECEIVED] = { 0x00 };
uint8_t payload_send_size = 15;
// uint32_t wait_time_ms = 0;
#ifdef BLOC
    sLoRaWanKeys  LoraWanKeys  = { LoRaMacNwkSKeyInit, LoRaMacAppSKeyInit, LoRaMacAppKeyInit, AppEuiInit, DevEuiInit, LoRaDevAddrInit,OTA_DEVICE };
#else
    uint8_t DevEuiInit2[]              = { 0x38, 0x35, 0x31, 0x31, 0x18, 0x47, 0x37, 0x56 };    
    sLoRaWanKeys  LoraWanKeys  = { LoRaMacNwkSKeyInit, LoRaMacAppSKeyInit, LoRaMacAppKeyInit, AppEuiInit, DevEuiInit2, LoRaDevAddrInit,APB_DEVICE };
#endif

mcu.InitMcu();
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

#ifdef SX126x_BOARD
    LoraWanObject<LoraRegionsEU,SX126x> Lp ( LoraWanKeys,&RP,USERFLASHADRESS); 
#endif
#ifdef SX1276_BOARD
    LoraWanObject<LoraRegionsEU,SX1276> Lp ( LoraWanKeys,&RP,USERFLASHADRESS); 
#endif
#ifdef SX1272_BOARD
    LoraWanObject<LoraRegionsEU,SX1272> Lp ( LoraWanKeys,&RP,USERFLASHADRESS); 
#endif

mcu.mwait_ms(2);
PointToPointReceiver    ptpRx(&RP, POINT_TO_POINT_RX_HOOK_ID);
PointToPointTransmitter ptpTx(&RP, POINT_TO_POINT_TX_HOOK_ID);
// mcu.Init_Irq(PA_1);
//mcu.InitGpioIn(USER_BUTTON);

RP.InitHook ( LP_HOOK_ID , &(Lp.packet.Phy.CallbackIsrRadio), &(Lp.packet.Phy) );
RP.InitHook ( POINT_TO_POINT_RX_HOOK_ID, &(PointToPointReceiver::Callback ),reinterpret_cast<void*>(&ptpRx));
RP.InitHook ( POINT_TO_POINT_TX_HOOK_ID, &(PointToPointTransmitter::Callback ),reinterpret_cast<void*>(&ptpTx));
RP.InitHook ( TX_ON_RX3_ID, &CallBackTxOnRx3, reinterpret_cast <void * > (&RP) );
STask       Tx4Rx3Task;
Tx4Rx3Task.HookId       = TX_ON_RX3_ID;
Tx4Rx3Task.TaskDuration = 200; // tbupdated with Timeonair
Tx4Rx3Task.State        = TASK_SCHEDULE;
Tx4Rx3Task.TaskType     = TX_LORA;

DEBUG_MSG("Init  Done\n");


uint8_t AvailableRxPacket  = NO_LORA_RXPACKET_AVAILABLE ;
eLoraWan_Process_States LpState = LWPSTATE_IDLE;  
RadioUser.Reset();
mcu.GotoSleepMSecond ( 300 );
uint32_t count_start = 0;
const uint32_t wait_before_next_start_min = 1000;  // 1000
const uint32_t wait_before_next_start_max = 17000; // 10000
uint16_t wait_before_next_start = wait_before_next_start_min;
StatisticCounters_t ptp_stats;
bool tx_loop_running = false;
bool start_tx = false;

mcu.MMClearDebugBufferRadioPlaner ( );

//Lp.RestoreContext  ( );
#if BLOC
    Lp.SetDataRateStrategy ( USER_DR_DISTRIBUTION );
    
#else 
    Lp.SetDataRateStrategy ( USER_DR_DISTRIBUTION );
    Lp.ActivateRX3 (); 
#endif
UserFport       = 3;
uint8_t UserPayloadSizeClassA = 14;
uint8_t UserPayloadClassA [ 250 ];

for (int i = 0 ; i < UserPayloadSizeClassA ; i++ ) {
    UserPayloadClassA [i]= i;
}
UserPayloadClassA [ 0 ]  = FW_VERSION ;
uint8_t MsgTypeClassA = UNCONF_DATA_UP;
Lp.NewJoin();

uint32_t next_start = mcu.RtcGetTimeMs();
uint32_t CptDemo = 0;
uint32_t RxAppTime = 0;

//relay.AddDevaddrInWhiteList(0x26011D16);
mcu.MMClearDebugBufferRadioPlaner ( );
//ptpRx.Start(payload_received, &payload_received_size);
next_start = mcu.RtcGetTimeMs();
start_tx   = true;
bool T3ACtivated = false;
int cpt = 200;
#ifdef BLOC
    Lp.RestoreContext  ( );

    while ( ( Lp.IsJoined ( ) == NOT_JOINED ) && ( Lp.GetIsOtaDevice ( ) == OTA_DEVICE) ) {       
        LpState  = Lp.Join( next_start + 200 );
        while ( ( LpState != LWPSTATE_IDLE ) && ( LpState != LWPSTATE_ERROR ) && ( LpState != LWPSTATE_INVALID ) ) {
            LpState = Lp.LoraWanProcess( &AvailableRxPacket );
            mcu.GotoSleepMSecond ( 200 );
            mcu.WatchDogRelease  ( );
        }
         mcu.GotoSleepMSecond ( 3000 );
    } 
    ptpRx.Start(payload_received, &payload_received_size);
    while(1){
        ptpRx.GetRxPayload ( UserPayloadClassA, &UserPayloadSizeClassA, &RxAppTime );
        if ( UserPayloadSizeClassA > 0 ) {
            uint32_t ReceiveDevaddr = relay.RelayExtractDevaddr(UserPayloadClassA);
            
            if ( relay.IsWhiteListedDevaddr(ReceiveDevaddr) == YES ) { 
                DEBUG_PRINTF ( "devaddr = %x\n", ReceiveDevaddr);
                LpState  = Lp.SendPayload( UserFport, UserPayloadClassA, UserPayloadSizeClassA, MsgTypeClassA , mcu.RtcGetTimeMs () + 200 );
                relay.SetTxTimeForRx3 ( RxAppTime , ReceiveDevaddr );
            }
        }
        if ( (cpt > 100 ) && (T3ACtivated == false) ){
            cpt = 0 ;
            relay.builStatus( UserPayloadClassA, &UserPayloadSizeClassA );
            LpState  = Lp.SendPayload( UserFport+1, UserPayloadClassA, UserPayloadSizeClassA, MsgTypeClassA , mcu.RtcGetTimeMs () + 200 );
        }
        while ( ( LpState != LWPSTATE_IDLE ) && ( LpState != LWPSTATE_ERROR ) && ( LpState != LWPSTATE_INVALID ) ) {
            LpState = Lp.LoraWanProcess( &AvailableRxPacket );
            mcu.GotoSleepMSecond ( 200 );
            mcu.WatchDogRelease  ( );
        }
        if ( AvailableRxPacket != NO_LORA_RXPACKET_AVAILABLE ) { 
            AvailableRxPacket  = NO_LORA_RXPACKET_AVAILABLE ;
            sStatisticTest.ClassARxCpt ++;
            InsertTrace ( __COUNTER__, FileId );
            DEBUG_PRINTF("Receive on port %d  an Applicative Downlink \n DATA[%d] = [ ",UserRxFport,UserRxPayloadSize);
                    for ( i = 0 ; i < UserRxPayloadSize ; i++){
                        DEBUG_PRINTF( "0x%.2x ",UserRxPayload[i]);
                    }
            Lp.ReceivePayload( &UserRxFport, UserRxPayload, &UserRxPayloadSize );
            if ( UserRxPayloadSize > 0) {
                if ( ( UserRxPayloadSize == 4 ) && (UserRxFport == 1)  ) {
                    relay.AddDevaddrInWhiteList ( UserRxPayload [3] + (UserRxPayload [2] << 8) + (UserRxPayload [1] << 16) + (UserRxPayload [0] << 24) );
                    cpt = 0 ;
                    relay.builStatus( UserPayloadClassA, &UserPayloadSizeClassA );
                    LpState  = Lp.SendPayload( UserFport+1, UserPayloadClassA, UserPayloadSizeClassA, MsgTypeClassA , mcu.RtcGetTimeMs () + 200 );

                }
                if ( ( UserRxPayloadSize == 4 ) && (UserRxFport == 2) ) {
                    uint32_t Removedevaddr =  UserRxPayload [3] + (UserRxPayload [2] << 8) + (UserRxPayload [1] << 16) + (UserRxPayload [0] << 24);
                    relay.RemoveDevaddrInWhiteList ( Removedevaddr );
                    cpt = 0 ;
                    relay.builStatus( UserPayloadClassA, &UserPayloadSizeClassA );
                    LpState  = Lp.SendPayload( UserFport+1, UserPayloadClassA, UserPayloadSizeClassA, MsgTypeClassA , mcu.RtcGetTimeMs () + 200 );
                    DEBUG_PRINTF ("remove devadrr =%x from relay\n", Removedevaddr);
                }
                if ( UserRxPayloadSize > 9 )   {
                    uint32_t ReceiveDevaddr = relay.ExtractDevaddrDownLink(UserRxPayload);
                    DEBUG_PRINTF ( "ReceiveDevaddr = 0x%x\n", ReceiveDevaddr);
                    uint32_t TargetTime ;
                    if ( ( relay.IsRx3Activated ( ReceiveDevaddr ) == YES ) && ( relay.GetTxTimeForRx3 ( &TargetTime, ReceiveDevaddr ) == OK ) ) {
                        DEBUG_PRINTF("Receive on port %d  an Applicative Downlink \n DATA[%d] = [ ",UserRxFport,UserRxPayloadSize);
                        for ( i = 0 ; i < UserRxPayloadSize ; i++){
                            DEBUG_PRINTF( "0x%.2x ",UserRxPayload[i]);
                        }
                        DEBUG_MSG ( "]\n");
                        Tx4Rx3Task.StartTime = TargetTime;
                        RP.EnqueueTask (Tx4Rx3Task, UserRxPayload, &UserRxPayloadSize, ptpRx.Tx4Rx3Param ); //@tbd RadioPlaner  timeonair 
                    }
                }
            } else {
                DEBUG_MSG ("Receive Ack \n");
            }
        }
        cpt++;
        mcu.GotoSleepMSecond ( 2000 );
    }
#else 
#endif;
    Lp.Init ();
    //uint32_t FrequencyPlan[3] = {863100000,863300000,863500000};
    //Lp.SetDefaultChannel (FrequencyPlan , 3);
    while (1) {
        if ( start_tx ) {
            start_tx = false;
            count_start++;
            ptpTx.SetChannelDr (  Lp.GetNextFrequency ( ), Lp.GetNextDataRate  ( ) );
            uint32_t NextSendSlot = ptpTx.Start(payload_send, payload_send_size);
            if ( ( Lp.IsJoined ( ) == NOT_JOINED ) && ( Lp.GetIsOtaDevice ( ) == OTA_DEVICE) ) {       
                LpState  = Lp.Join( NextSendSlot );
            } else {
                if ( AvailableRxPacket != NO_LORA_RXPACKET_AVAILABLE ) { 
                    AvailableRxPacket  = NO_LORA_RXPACKET_AVAILABLE ;
                    sStatisticTest.ClassARxCpt ++;
                    InsertTrace ( __COUNTER__, FileId );
                    Lp.ReceivePayload( &UserRxFport, UserRxPayload, &UserRxPayloadSize );
                    if ( UserRxPayloadSize > 0) {
                        DEBUG_PRINTF("Receive on port %d  an Applicative Downlink \n DATA[%d] = [ ",UserRxFport,UserRxPayloadSize);
                        for ( i = 0 ; i < UserRxPayloadSize ; i++){
                            DEBUG_PRINTF( "0x%.2x ",UserRxPayload[i]);
                        }
                    } else {
                        DEBUG_MSG ("Receive Ack \n");
                    }
                    LpState  = Lp.SendPayload( UserFport, UserRxPayload, UserRxPayloadSize, UNCONF_DATA_UP,NextSendSlot );
                } else {   
                    LpState  = Lp.SendPayload( UserFport, UserPayloadClassA, UserPayloadSizeClassA, UNCONF_DATA_UP,NextSendSlot );
                }
            }
        }
        if((int32_t)(next_start - mcu.RtcGetTimeMs() ) <= 0){
            start_tx = true;
            next_start = mcu.RtcGetTimeMs() + 10000;
            ptpTx.GetStatistics(&ptp_stats);
            //PrintPtpStatistics(&ptp_stats, count_start);
        }
        while ( ( LpState != LWPSTATE_IDLE ) && ( LpState != LWPSTATE_ERROR ) && ( LpState != LWPSTATE_INVALID ) ) {
            LpState = Lp.LoraWanProcess( &AvailableRxPacket );
            mcu.GotoSleepMSecond ( 200 );
            mcu.WatchDogRelease  ( );
        }
         
    mcu.GotoSleepMSecond ( 200 );
    }
}

/*
 while (true) {
  if ( ( Lp.IsJoined ( ) == NOT_JOINED ) && ( Lp.GetIsOtaDevice ( ) == OTA_DEVICE) ) {       
     LpState  = Lp.Join( 0);
  } else {
    if ( CptDemo == 0 ) {
      LpState  = Lp.SendPayload( UserFport, UserPayloadClassA, UserPayloadSizeClassA, MsgTypeClassA, 0 );
    };
    (CptDemo == 100) ? CptDemo = 0 : CptDemo++ ;
    sStatisticTest.TxClassACpt++;
  }

  if (mcu.GetValueDigitalInPin(USER_BUTTON) == 0) {
    tx_loop_running = !tx_loop_running;
    next_start = mcu.RtcGetTimeMs();
    mcu.mwait_ms(200);
  }
  if (tx_loop_running) {
    if (start_tx){
      start_tx = false;
      count_start++;
     // ptpTx.Start(payload_send, payload_send_size);
    }
    if((int32_t)(next_start - mcu.RtcGetTimeMs() ) <= 0){
      start_tx = true;
      next_start = mcu.RtcGetTimeMs() + randr(2000, 10000);
      ptpTx.GetStatistics(&ptp_stats);
      PrintPtpStatistics(&ptp_stats, count_start);
    }
  }
*  
  while ( ( LpState != LWPSTATE_IDLE ) && ( LpState != LWPSTATE_ERROR ) && ( LpState != LWPSTATE_INVALID ) ) {
      LpState = Lp.LoraWanProcess( &AvailableRxPacket );
      mcu.GotoSleepMSecond ( 400 );
      mcu.WatchDogRelease  ( );
  }
  //mcu.MMPrintBuffer ( ) ;
  //RP.GetStatistic ( );
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
      if ( UserRxPayloadSize > 0) {
          DEBUG_PRINTF("Receive on port %d  an Applicative Downlink \n DATA[%d] = [ ",UserRxFport,UserRxPayloadSize);
          for ( i = 0 ; i < UserRxPayloadSize ; i++){
              DEBUG_PRINTF( "0x%.2x ",UserRxPayload[i]);
          }
      } else {
          DEBUG_MSG ("Receive Ack \n");
      }
     
  }

   mcu.GotoSleepSecond ( 30 );
  mcu.MMPrintBuffer();
  
  sStatisticTest.PrintStatisticTest(); 
}
}

PintPtpStatistics(const StatisticCounters_t* stats, const uint16_t counter)
{DEBUG_PRINTF("......... %3i .........\n", counter);
DEBUG_PRINTF("Wake up fragment tx attempts: %5i\n", stats->wuf_tx_attempt);
DEBUG_PRINTF("Wake up sequence tx attempts: %5i\n", stats->wus_tx_attempt);
DEBUG_PRINTF("Data tx attempts:             %5i\n", stats->data_tx_attempt);
DEBUG_PRINTF("Acknowledge rx attempts:      %5i\n", stats->ack_rx_attempt);
DEBUG_PRINTF("Acknowledge rx successes:     %5i\n", stats->ack_rx_success);
DEBUG_MSG("==============================\n");
}
*/
