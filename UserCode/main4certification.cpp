///*!
// * \file      Main.c
// *
// * \brief     Description : Example main of LoRaWan MiniMouse stack
// *
// * \copyright Revised BSD License, see section \ref LICENSE.
// *
// * \code
//  __  __ _       _                                 
// |  \/  (_)     (_)                                
// | \  / |_ _ __  _ _ __ ___   ___  _   _ ___  ___  
// | |\/| | | '_ \| | '_ ` _ \ / _ \| | | / __|/ _ \
// | |  | | | | | | | | | | | | (_) | |_| \__ \  __/ 
// |_|  |_|_|_| |_|_|_| |_| |_|\___/ \__,_|___/\___| 
//                                                   
//                                                   
// * \endcode

//Maintainer        : Fabien Holin (SEMTECH)
//*/
//#include "mbed.h"
//#include "ApiFlash.h"
//#include "LoraMacDataStoreInFlash.h"
//#include "LoraWanProcess.h"
//#include "Define.h"
//#include "rtc_api.h"
//#include "ApiTimers.h"
//#include "utilities.h"
//#include "UserDefine.h"
///*!
// * \brief   sBackUpFlash BackUpFlash The LoraWan parameters save into the flash memory for failsafe restauration.
// */
//struct sBackUpFlash BackUpFlash;
//InterruptIn RadioGlobalIt        ( TX_RX_IT ) ;
//InterruptIn RadioTimeOutGlobalIt ( RX_TIMEOUT_IT ); 


///*!
// * \brief   Parameters of the LoraWanKeys structure. 
// * \remark  For APB Devices only NwkSkey, AppSKey and devaddr are mandatory
// * \remark  For OTA Devices only DevEUI, AppEUI and AppKey are mandatory
// */
//uint8_t LoRaMacNwkSKeyInit[] = { 0x22, 0x33, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11};
//uint8_t LoRaMacAppSKeyInit[] = { 0x11, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22};
//uint8_t LoRaMacAppKeyInit[]  = { 0x33, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11};
//uint8_t AppEuiInit[]         = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
//uint8_t DevEuiInit[]         = { 0x11, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x33 };    
//uint32_t LoRaDevAddrInit     = 0x26011918;

//sLoRaWanKeys  LoraWanKeys ={LoRaMacNwkSKeyInit, LoRaMacAppSKeyInit, LoRaMacAppKeyInit, AppEuiInit, DevEuiInit, LoRaDevAddrInit,APB_DEVICE};
//SX1276  RadioUser( LORA_SPI_MOSI, LORA_SPI_MISO, LORA_SPI_SCLK, LORA_CS, LORA_RESET );
//int main( ) {
//    int i;
//    int StatusCertification = 0;
//    uint8_t UserPayloadSize = 14;
//    uint8_t UserPayload[255];
//    uint8_t UserRxPayloadSize;
//    uint8_t UserRxPayload [255];
//    uint8_t UserFport = 3;
//    uint8_t UserRxFport ;
//    uint8_t MsgType ;
//    
//    /*!
//    * \brief   Lp<LoraRegionsEU>: A LoRaWan Object with Eu region's rules. 
//    * \remark  The Current implementation doesn't yet support different radio (only SX1276) 
//    * \remark  On the future dev , the Radio Type will be a parameter of the LoraWan Objects
//    */
//     LoraWanObject<LoraRegionsEU,SX1276> Lp( LoraWanKeys,&RadioUser,USERFLASHADRESS); 

//   

//    /*!
//    * \brief  RtcInit , WakeUpInit, LowPowerTimerLoRaInit() are Mcu dependant . 
//    */
//    RtcInit ( );
//    WakeUpInit ( );
//    WatchDogStart ( );
//    LowPowerTimerLora.LowPowerTimerLoRaInit();
//#if DEBUG_TRACE == 1
//    pcf.baud( 115200 );
//#endif    

//    /*!
//    * \brief  For this example : send an un confirmed message on port 3 . The user payload is a ramp from 0 to 13 (14 bytes). 
//    */
//    UserFport       = 3;
//    UserPayloadSize = 14;
//    for (uint8_t i = 0; i < 14 ; i ++) {
//        UserPayload[i]  = i;
//    }
//    MsgType = UNCONF_DATA_UP;
//    uint8_t AvailableRxPacket = NO_LORA_RXPACKET_AVAILABLE ;
//    eLoraWan_Process_States LpState = LWPSTATE_IDLE;    
//    /*!
//    * \brief  Configure the DataRate Strategy 
//    */
//    Lp.SetDataRateStrategy( MOBILE_LONGRANGE_DR_DISTRIBUTION );

//    
//    /*!
//    * \brief Restore the LoraWan Context
//    */
//    wait(2);
//    //Lp.RestoreContext ( );



//    while(1) {
//        DEBUG_MSG("\n\n\n\n ");

//        if ( Lp.IsJoined ( ) == JOINED ) {            
//            LpState = Lp.SendPayload( UserFport, UserPayload, UserPayloadSize, MsgType );
//        } else {
//            LpState = Lp.Join( );
//        }
//        /*!
//         * \brief 
//         *        This function manages the state of the MAC and performs all the computation intensive (crypto) tasks of the MAC.
//         *        This function is called periodically by the user’s application whenever the state of the MAC is not idle.
//         *        The function is not timing critical, can be called at any time and can be interrupted by any IRQ (including the user’s IRQ).
//         *        The only requirement is that this function must be called at least once between the end of the transmission and the beginning of the RX1 slot.
//         *        Therefore when the stack is active a call periodicity of roughly 300mSec is recommended.
//         */ 

//        while ( LpState != LWPSTATE_IDLE ){
//            LpState = Lp.LoraWanProcess( &AvailableRxPacket );
//            wait_ms(100);
//           // GotoSleepMSecond ( 100 );
//            WatchDogRelease ( );
//        }

//        if ( AvailableRxPacket == LORA_RX_PACKET_AVAILABLE ) { 
//            Lp.ReceivePayload( &UserRxFport, UserRxPayload, &UserRxPayloadSize );
//            DEBUG_PRINTF("Receive on port %d  an Applicative Downlink \n DATA[%d] = [ ",UserRxFport,UserRxPayloadSize);
//            for ( i = 0 ; i < UserRxPayloadSize ; i++){
//                DEBUG_PRINTF( "0x%.2x ",UserRxPayload[i]);
//            }
//            DEBUG_MSG("]\n");
//            if ( ( UserRxFport == 224 ) || ( UserRxPayloadSize == 0 ) ) {
//               DEBUG_MSG("Receive Certification Payload \n"); 
//               StatusCertification = Certification (true , &UserFport , &UserPayloadSize, &UserRxPayloadSize, &MsgType, UserRxPayload, UserPayload, &Lp) ;
//            } 
//        } else {
//            if ( StatusCertification > 0 ){
//                Certification ( false ,  &UserFport , &UserPayloadSize, &UserRxPayloadSize, &MsgType, UserRxPayload, UserPayload, &Lp) ;
//            }
//        }
//        /*!
//         * \brief Send a ¨Packet every 5 seconds and goto to sleep
//        */
//        if ( Lp.IsJoined ( ) == JOINED ) {
//            //GotoSleepSecond ( AppTimeSleeping *100 );
//            wait(5);
//        } else {
//            wait(5);
//            //GotoSleepSecond ( 5 );
//        }
//    }
//}

