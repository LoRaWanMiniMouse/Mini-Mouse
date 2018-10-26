/*!
 * \file      Main4certification.c
 *
 * \brief     Description : Lorawan Stack example
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

InterruptIn button(GPIO_SENSOR);
int Flag_Abort = 0;
uint32_t TaskTimeToSend[NUM_OF_TASK];

void flip() {
    Flag_Abort = 1;
    TaskTimeToSend[TASK_GPIO_SENSOR] = mcu.RtcGetTimeSecond () - 10;
    DEBUG_MSG("Push BUtton\n");
}
 


#define FileId 4
/*!
 * \brief   BackUpFlash The LoraWan structure parameters save into the flash memory for failsafe restauration.
 */
struct sBackUpFlash BackUpFlash;

/*!
 * \brief   Radio Interrupt Pin declarations
 */

McuXX<McuSTM32L4> mcu ( LORA_SPI_MOSI, LORA_SPI_MISO, LORA_SPI_SCLK ) ;

/*!
 * \brief   Parameters of the LoraWanKeys structure. 
 * \remark  For APB Devices only NwkSkey, AppSKey and devaddr are mandatory
 * \remark  For OTA Devices only DevEUI, AppEUI and AppKey are mandatory
 */


uint8_t LoRaMacNwkSKeyInit[]      = { 0x22, 0x33, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11};
uint8_t LoRaMacAppSKeyInit[]      = { 0x11, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22};
uint8_t LoRaMacAppKeyInit[]       = { 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0xBB};
//uint8_t LoRaMacAppKeyInit[]       = { 0xBB, 0xBB, 0xF6, 0x22, 0x16, 0x67, 0xDB, 0x7E, 0xA9, 0xF2, 0x99, 0x71, 0x29, 0x12, 0x1c, 0x39};
//uint8_t AppEuiInit[]              = { 0x11 ,0x22, 0x33, 0x44, 0x44, 0x33, 0x22, 0x11 };
uint8_t AppEuiInit[]              = { 0x70 ,0xb3, 0xd5, 0x7E, 0xD0, 0x00, 0xFF, 0x50 };
uint8_t DevEuiInit[]              = { 0x11, 0x22, 0x33, 0x44, 0x44, 0x33, 0xcc, 0xbb };    

uint32_t LoRaDevAddrInit          = 0x26011920;


#ifdef SX126x_BOARD
#define FW_VERSION     0x18
    SX126x  RadioUser( LORA_BUSY, LORA_CS, LORA_RESET,TX_RX_IT );

#endif
#ifdef SX1276_BOARD
#define FW_VERSION     0x17
    SX1276  RadioUser( LORA_CS, LORA_RESET, TX_RX_IT, RX_TIMEOUT_IT);

#endif
#ifdef SX1272_BOARD
#define FW_VERSION     0x13
    SX1272  RadioUser( LORA_CS, LORA_RESET, TX_RX_IT, RX_TIMEOUT_IT);
#endif
/* User Radio ISR routine */
#define NBENCODEDFRAME 52

int main( ) {
    int i;
    uint8_t UserPayloadSize ;
    uint8_t UserPayload [255];
    uint8_t UserRxPayloadSize;
    uint8_t UserRxPayload [125];
    uint8_t UserFport ;
    uint8_t UserRxFport ;
    uint8_t MsgType ;
    uint8_t AppTimeSleeping = 5;
    uint8_t uid[8];
    /*!
    * \brief  RtcInit , WakeUpInit, LowPowerTimerLoRaInit() are Mcu dependant . 
    */
    mcu.InitMcu ( );
    mcu.WatchDogStart ( );
    mcu.GetUniqueId (uid); 
    sLoRaWanKeys  LoraWanKeys ={LoRaMacNwkSKeyInit, LoRaMacAppSKeyInit, LoRaMacAppKeyInit, AppEuiInit, DevEuiInit, LoRaDevAddrInit,OTA_DEVICE};
    memcpy(&LoraWanKeys.DevEui[0], uid , 8);
    /*!
    * \brief   Lp<LoraRegionsEU>: A LoRaWan Object with Eu region's rules. 
    * \remark  The Current implementation  support radio SX1276 and sx1261
    */
#ifdef SX126x_BOARD
    LoraWanObject<LoraRegionsEU,SX126x> Lp( LoraWanKeys,&RadioUser,USERFLASHADRESS); 
#endif
#ifdef SX1276_BOARD
    LoraWanObject<LoraRegionsEU,SX1276> Lp( LoraWanKeys,&RadioUser,USERFLASHADRESS); 
#endif
#ifdef SX1272_BOARD
    LoraWanObject<LoraRegionsEU,SX1272> Lp( LoraWanKeys,&RadioUser,USERFLASHADRESS); 
#endif
    //SX126x  RadioUser( LORA_BUSY, LORA_CS, LORA_RESET,TX_RX_IT );
    //LoraWanObject<LoraRegionsEU,SX126x> Lp( LoraWanKeys,&RadioUser,USERFLASHADRESS); 
    uint8_t AvailableRxPacket = NO_LORA_RXPACKET_AVAILABLE ;
    eLoraWan_Process_States LpState = LWPSTATE_IDLE;    
    /*!
    * \brief Restore the LoraWan Context
    */
    DEBUG_PRINTF("MM is starting ...{ %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x } \n",uid[0],uid[1],uid[2],uid[3],uid[4],uid[5],uid[6],uid[7]);
    uint8_t TPointer ;
    TPointer = ExtDebugTrace[ TRACE_SIZE - 1]& 0xff;  
    for (int i  = 0; i < 200; i++){ 
        UserPayload [i] = (ExtDebugTrace[(uint8_t)(TPointer - i)] & 0xFF);
    }
    ReadTrace( ExtDebugTrace );
    mcu.mwait(2);
    Lp.RestoreContext  ( );
//    Lp.SetDataRateStrategy( STATIC_ADR_MODE );
//    UserFport       = 3;
//    UserPayloadSize = 14;
//    for (int i = 0 ; i < UserPayloadSize ; i++ ) {
//        UserPayload[i]= i;
//    }
//    UserPayload[ 0 ]  = FW_VERSION ;
//    MsgType = UNCONF_DATA_UP;
    /***********************INIT TIMING TASK *///////////////////
    Lp.SetDataRateStrategy( MOBILE_LONGRANGE_DR_DISTRIBUTION );

    TaskTimeToSend[TASK1] = mcu.RtcGetTimeSecond () + 20 ;
    TaskTimeToSend[TASK2] = mcu.RtcGetTimeSecond () + 50 ;
    TaskTimeToSend[TASK3] = mcu.RtcGetTimeSecond () + 28  ;
    TaskTimeToSend[TASK_GPIO_SENSOR] = mcu.RtcGetTimeSecond () + (3600*24*365*10)  ; // task delay 10 years later ! modify inside the Interrupt
    int NextTaskToCallTemp ;
    int TaskToken ;
    uint32_t NextSleepTime ;
    eTaskid NextTaskToCall;
    mcu.Init_Irq ( GPIO_SENSOR ) ;

    
    while(1) {
        if ( ( Lp.IsJoined ( ) == NOT_JOINED ) && ( Lp.GetIsOtaDevice ( ) == OTA_DEVICE) ) {       
            InsertTrace ( __COUNTER__, FileId );
            LpState = Lp.Join( );
        } else {

            /*!
            * \brief  For this example : send an un confirmed message on port 3 . The user payload is a ramp from 0 to 13 (14 bytes) + FW version. 
            */
             /************************TASK arbitrer**************************/
            NextTaskToCall = TASKIDLE ;
            NextTaskToCallTemp = 0 ;
            TaskToken = 0xFFFFFF;
            NextSleepTime = 0;
            for (int i = 0; i < NUM_OF_TASK ; i++) {
                int TaskTokenTemp = TaskTimeToSend[i] - mcu.RtcGetTimeSecond (); // compare current time and time target for the task
                if ( TaskTokenTemp < TaskToken ){
                     TaskToken = TaskTokenTemp;
                     NextTaskToCallTemp =  i ; 
                }
            }
            (TaskToken > 0 ) ? NextSleepTime = TaskToken : NextTaskToCall = (eTaskid) NextTaskToCallTemp ;
            if (NextTaskToCall == TASKIDLE) {
                 DEBUG_PRINTF("Next Task = TASK_IDLE and goto to sleep for %d Seconds \n",NextSleepTime );
            } else {
                 DEBUG_PRINTF("Next Task = TASK%.1d ,  \n",NextTaskToCall + 1  );
            }
            /******************************En of Task Arbitrer****************/

            switch (NextTaskToCall) {
                case TASKIDLE :
                    break;
                case TASK1 :
                    UserFport  = 1;
                    UserPayloadSize = 14;
                    MsgType = UNCONF_DATA_UP;
                    for (int i = 0 ; i < UserPayloadSize ; i++ ) {
                        UserPayload[i]= 22;
                    }
                    InsertTrace ( __COUNTER__, FileId );
                    LpState = Lp.SendPayload( UserFport, UserPayload, UserPayloadSize, MsgType );
                    break;
                case TASK2 :
                    UserFport  = 2;
                    UserPayloadSize = 1;
                    MsgType = UNCONF_DATA_UP;
                    for (int i = 0 ; i < UserPayloadSize ; i++ ) {
                        UserPayload[i]= FW_VERSION;
                    }
                    InsertTrace ( __COUNTER__, FileId );
                    LpState = Lp.SendPayload( UserFport, UserPayload, UserPayloadSize, MsgType );
                    break;
                case TASK3 :
                    UserFport  = 25;
                    UserPayloadSize = 18;
                    MsgType = UNCONF_DATA_UP;
                    for (int i = 0 ; i < UserPayloadSize ; i++ ) {
                        UserPayload[i]= i;
                    }
                    InsertTrace ( __COUNTER__, FileId );
                    LpState = Lp.SendPayload( UserFport, UserPayload, UserPayloadSize, MsgType );
                    break;
               case TASK_GPIO_SENSOR :
                    UserFport  = 22;
                    UserPayloadSize = 2;
                    MsgType = UNCONF_DATA_UP;
                    for (int i = 0 ; i < UserPayloadSize ; i++ ) {
                        UserPayload[i]= 66;
                    }
                    InsertTrace ( __COUNTER__, FileId );
                    LpState = Lp.SendPayload( UserFport, UserPayload, UserPayloadSize, MsgType );
                    break;
            }
        }
/*!
* \brief 
*        This function manages the state of the MAC and performs all the computation intensive (crypto) tasks of the MAC.
*        This function is called periodically by the user’s application whenever the state of the MAC is not idle.
*        The function is not timing critical, can be called at any time and can be interrupted by any IRQ (including the user’s IRQ).
*        The only requirement is that this function must be called at least once between the end of the transmission and the beginning of the RX1 slot.
*        Therefore when the stack is active a call periodicity of roughly 300mSec is recommended.
*/ 
        DEBUG_MSG("\n\n");

        while ( ( LpState != LWPSTATE_IDLE ) && ( LpState != LWPSTATE_ERROR ) && ( LpState != LWPSTATE_INVALID) ){
            LpState = Lp.LoraWanProcess( &AvailableRxPacket );
            mcu.GotoSleepMSecond ( 500 );
            mcu.WatchDogRelease ( );
        }

        if ( LpState == LWPSTATE_ERROR ) {
            InsertTrace ( __COUNTER__, FileId );
            // user application have to save all the need
            NVIC_SystemReset();
        }
        if ( AvailableRxPacket != NO_LORA_RXPACKET_AVAILABLE ) { 
            InsertTrace ( __COUNTER__, FileId );
            Lp.ReceivePayload( &UserRxFport, UserRxPayload, &UserRxPayloadSize );
            DEBUG_PRINTF("Receive on port %d  an Applicative Downlink \n DATA[%d] = [ ",UserRxFport,UserRxPayloadSize);
            for ( i = 0 ; i < UserRxPayloadSize ; i++){
                DEBUG_PRINTF( "0x%.2x ",UserRxPayload[i]);
            }
            DEBUG_MSG("]\n\n\n");
        }
        
        switch (NextTaskToCall) {
            case TASKIDLE :
                break;
            case TASK1 :
                TaskTimeToSend[TASK1] = mcu.RtcGetTimeSecond () + 20 ;
                break;
            case TASK2 :
                TaskTimeToSend[TASK2] = mcu.RtcGetTimeSecond () + 50 ;
                break;
            case TASK3 :
                TaskTimeToSend[TASK3] = mcu.RtcGetTimeSecond () + 32  ;
                break;
            case TASK_GPIO_SENSOR :
                TaskTimeToSend[TASK_GPIO_SENSOR] = mcu.RtcGetTimeSecond () + (3600*24*365*10)  ;
                break;
        }
           /************************Sleep arbitrer**************************/

        if ( ( Lp.IsJoined ( ) == NOT_JOINED ) && ( Lp.GetIsOtaDevice ( ) == OTA_DEVICE) && ( LpState != LWPSTATE_INVALID)){
            InsertTrace ( __COUNTER__, FileId ); 
            mcu.GotoSleepSecond(5);
        } else if (NextTaskToCall == TASKIDLE) {
            
            InsertTrace ( __COUNTER__, FileId );
            mcu.GotoSleepSecond ( NextSleepTime );
            InsertTrace ( __COUNTER__, FileId );
        }
    }
}

