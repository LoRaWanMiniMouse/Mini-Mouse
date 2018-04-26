///*!
// * \file      Main4certification.c
// *
// * \brief     Description : Replace  main of LoRaWan MiniMouse stack  by this file in case of certification tests
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

//#include "ApiMcu.h"
//#include "LoraMacDataStoreInFlash.h"
//#include "LoraWanProcess.h"
//#include "Define.h"
//#include "utilities.h"
//#include "UserDefine.h"
//#include "appli.h"
//#include "SX126x.h"
///*!
// * \brief   BackUpFlash The LoraWan structure parameters save into the flash memory for failsafe restauration.
// */
//struct sBackUpFlash BackUpFlash;

///*!
// * \brief   Radio Interrupt Pin declarations
// */

//McuXX<McuSTM32L4> mcu ( LORA_SPI_MOSI, LORA_SPI_MISO, LORA_SPI_SCLK ) ;

///*!
// * \brief   Parameters of the LoraWanKeys structure. 
// * \remark  For APB Devices only NwkSkey, AppSKey and devaddr are mandatory
// * \remark  For OTA Devices only DevEUI, AppEUI and AppKey are mandatory
// */
//uint8_t LoRaMacNwkSKeyInit[] = { 0x22, 0x33, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11};
//uint8_t LoRaMacAppSKeyInit[] = { 0x11, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22};
//uint8_t LoRaMacAppKeyInit[]  = { 0xBB, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11};
//uint8_t AppEuiInit[]         = { 0x11, 0x22, 0x33, 0x44, 0x44, 0x33, 0x22, 0x22 };
//uint8_t DevEuiInit[]         = { 0x11, 0x22, 0x33, 0x44, 0x44, 0x33, 0xcc, 0xbb };    
//uint32_t LoRaDevAddrInit     = 0x26011918;


//SX1276  RadioUser( LORA_CS, LORA_RESET );
///* ISR routine specific of this example */
//void UserIsr ( void ) {
//    DEBUG_MSG( "\n\n\n Demo : use radio both in lorawan mode and in specific user mode \n receive isr radio \n\n\n" );
//}

//int main( ) {
//    int i;
//    uint8_t UserPayloadSize ;
//    uint8_t UserPayload [14];
//    uint8_t UserRxPayloadSize;
//    uint8_t UserRxPayload [25];
//    uint8_t UserFport ;
//    uint8_t UserRxFport ;
//    uint8_t MsgType ;
//    eDataRateStrategy AppDataRate = STATIC_ADR_MODE; // adr mode manage by network
//    uint8_t AppTimeSleeping = 60 ;

//    /*!
//    * \brief  RtcInit , WakeUpInit, LowPowerTimerLoRaInit() are Mcu dependant . 
//    */
//    mcu.InitMcu ( );
//    mcu.RtcInit ( );
//    mcu.WatchDogStart ( );
//    mcu.LowPowerTimerLoRaInit();
//#if DEBUG_TRACE == 1
//    pcf.baud( 115200 );
//#endif    
//    //SetDevEui ( DevEuiInit );
//    sLoRaWanKeys  LoraWanKeys ={LoRaMacNwkSKeyInit, LoRaMacAppSKeyInit, LoRaMacAppKeyInit, AppEuiInit, DevEuiInit, LoRaDevAddrInit,OTA_DEVICE};
//    sLoRaWanKeys  LoraWanKeys2 ={LoRaMacNwkSKeyInit, LoRaMacAppSKeyInit, LoRaMacAppKeyInit, AppEuiInit, DevEuiInit, LoRaDevAddrInit,APB_DEVICE};
//    /*!
//    * \brief   Lp<LoraRegionsEU>: A LoRaWan Object with Eu region's rules. 
//    * \remark  The Current implementation doesn't yet support different radio (only SX1276) 
//    * \remark  On the future dev , the Radio Type will be a parameter of the LoraWan Objects
//    */

//    LoraWanObject<LoraRegionsEU,SX1276> Lp( LoraWanKeys,&RadioUser,USERFLASHADRESS); 
//    LoraWanObject<LoraRegionsEU,SX1276> Lp2( LoraWanKeys2,&RadioUser,USERFLASHADRESS-4096); 
//    /*!
//    * \brief  For this example : send an un confirmed message on port 3 . The user payload is a ramp from 0 to 13 (14 bytes). 
//    */
//    UserFport       = 3;
//    UserPayloadSize = 14;
//    MsgType = UNCONF_DATA_UP;
//    uint8_t AvailableRxPacket = NO_LORA_RXPACKET_AVAILABLE ;
//    eLoraWan_Process_States LpState = LWPSTATE_IDLE;    
//    eLoraWan_Process_States Lp2State = LWPSTATE_IDLE;    
//    /*!
//    * \brief Restore the LoraWan Context
//    */
//    wait(2);
//    //Lp.FactoryReset ();
//    //Lp2.FactoryReset ();
//    Lp.RestoreContext  ( );
//    Lp2.RestoreContext ( );


//    while(1) {
//        
//        DEBUG_MSG("\n\n\n\n Lp1 OBJECT\n");
//        UserFport       = 3;
//        UserPayloadSize = 9;
//        PrepareFrame ( UserPayload );
//        UserPayload[8]  = Lp.GetNbOfReset(); // in this example adding number of reset inside the applicatif payload
//        /*!
//         * \brief  Configure a new DataRate Strategy 
//        */
//        Lp.SetDataRateStrategy( AppDataRate );
//        if ( Lp.IsJoined ( ) == NOT_JOINED ) {            
//            LpState = Lp.Join( );
//        } else {
//            LpState = Lp.SendPayload( UserFport, UserPayload, UserPayloadSize, MsgType );
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
//            mcu.GotoSleepMSecond ( 100 );
//            mcu.WatchDogRelease ( );
//        }
//        if ( LpState == LWPSTATE_ERROR ) {
//        // user application have to save all the need
//            NVIC_SystemReset();
//        }
//        if ( AvailableRxPacket == LORA_RX_PACKET_AVAILABLE ) { 
//            Lp.ReceivePayload( &UserRxFport, UserRxPayload, &UserRxPayloadSize );
//            DEBUG_PRINTF("Receive on port %d  an Applicative Downlink \n DATA[%d] = [ ",UserRxFport,UserRxPayloadSize);
//            for ( i = 0 ; i < UserRxPayloadSize ; i++){
//                DEBUG_PRINTF( "0x%.2x ",UserRxPayload[i]);
//            }
//            DEBUG_MSG("]\n");
//            // call the application layer to manage the application downlink
//        } 
//        /*!
//         * \brief Send a ¨Packet every 5 seconds in case of join 
//         *        Send a packet every AppTimeSleeping seconds in normal mode
//         */
//            mcu.GotoSleepSecond ( 1 );

///*!
//* \brief 
//*    In this example radio is also use to send packet no lorawan (or to do anything else)           
//*    It is very mandatory to attach / detach ISR radio in this case 
//*    to attach callback (void) UserIsr (void) on It TX done set : call RadioGlobalIt.rise( &UserIsr ); // attach ISR
//*    Don't forget to "detach" it . On this example it is done inside the function UserIsr with : RadioGlobalIt.rise( NULL );
//*/        
//        mcu.AttachInterruptIn( &UserIsr ); // attach ISR
//        RadioUser.SendLora( UserPayload, 13, 10, BW500, 868000000, 11 ); // send a buffer of 13 bytes , SF = 10, BW = 500 , freq = 868 MHz, Power = 11 dbm
//        mcu.GotoSleepSecond ( 1 );
///*!
//* \brief 
//*  In this example the payload is sent over two different lorawan networks
//*  
//*/        
//        DEBUG_MSG("\n\n\n\n Lp2 OBJECT\n");
//        UserPayload[8]  = Lp2.GetNbOfReset(); // in this example adding number of reset inside the applicatif payload
//        /*!
//         * \brief  Configure the DataRate Strategy 
//        */
//        Lp2.SetDataRateStrategy( MOBILE_LONGRANGE_DR_DISTRIBUTION );
//        if ( Lp2.IsJoined ( ) == NOT_JOINED ) {       // not applicable because APB_device but manage inside the lorawan so user haven't any issue     
//            Lp2State = Lp2.Join( );
//        } else {
//            Lp2State = Lp2.SendPayload( UserFport, UserPayload, UserPayloadSize, MsgType );
//        }
//        /*!
//         * \brief 
//         *        This function manages the state of the MAC and performs all the computation intensive (crypto) tasks of the MAC.
//         *        This function is called periodically by the user’s application whenever the state of the MAC is not idle.
//         *        The function is not timing critical, can be called at any time and can be interrupted by any IRQ (including the user’s IRQ).
//         *        The only requirement is that this function must be called at least once between the end of the transmission and the beginning of the RX1 slot.
//         *        Therefore when the stack is active a call periodicity of roughly 300mSec is recommended.
//         */ 

//        while ( Lp2State != LWPSTATE_IDLE ){
//            Lp2State = Lp2.LoraWanProcess( &AvailableRxPacket );
//            mcu.GotoSleepMSecond ( 100 );
//            mcu.WatchDogRelease ( );
//        }
//        if ( LpState == LWPSTATE_ERROR ) {
//        // user application have to save all the need
//            NVIC_SystemReset();
//        }
//        if ( AvailableRxPacket == LORA_RX_PACKET_AVAILABLE ) { 
//            Lp2.ReceivePayload( &UserRxFport, UserRxPayload, &UserRxPayloadSize );
//            DEBUG_PRINTF("Receive on port %d  an Applicative Downlink \n DATA[%d] = [ ",UserRxFport,UserRxPayloadSize);
//            for ( i = 0 ; i < UserRxPayloadSize ; i++){
//                DEBUG_PRINTF( "0x%.2x ",UserRxPayload[i]);
//            }
//            DEBUG_MSG("]\n");
//            // call the application layer to manage the application downlink
//        } 
//        /*!
//         * \brief Send a ¨Packet every 5 seconds in case of join 
//         *        Send a packet every AppTimeSleeping seconds in normal mode
//         */
//        if ( Lp.IsJoined ( ) == NOT_JOINED ) {
//            mcu.GotoSleepSecond(5);
//        } else {
//            mcu.GotoSleepSecond ( AppTimeSleeping );
//        }
//        
//    }
//}

