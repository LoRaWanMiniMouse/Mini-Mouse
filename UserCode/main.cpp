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
uint8_t LoRaMacNwkSKeyInit[] = { 0x22, 0x33, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11};
uint8_t LoRaMacAppSKeyInit[] = { 0x11, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22};
uint8_t LoRaMacAppKeyInit[]  = { 0xBB, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11};
uint8_t AppEuiInit[]         = { 0x70, 0xb3, 0xd5, 0x7e, 0xf0, 0x00, 0x36, 0x12 };
uint8_t DevEuiInit[]         = { 0x11, 0x22, 0x33, 0x44, 0x44, 0x33, 0x22, 0xbb };    
uint32_t LoRaDevAddrInit     = 0x26011918;

int main( ) {
    int i;
    uint8_t UserPayloadSize ;
    uint8_t UserPayload [14];
    uint8_t UserRxPayloadSize;
    uint8_t UserRxPayload [25];
    uint8_t UserFport ;
    uint8_t UserRxFport ;
    uint8_t MsgType ;
    eDataRateStrategy AppDataRate = STATIC_ADR_MODE; // adr mode manage by network
    uint8_t AppTimeSleeping = 6 ;

    /*!
    * \brief  RtcInit , WakeUpInit, LowPowerTimerLoRaInit() are Mcu dependant . 
    */
    mcu.InitMcu ( );
    mcu.RtcInit ( );
    mcu.WatchDogStart ( );
    mcu.LowPowerTimerLoRaInit();
	  mcu.UartInit ( );
    sLoRaWanKeys  LoraWanKeys ={LoRaMacNwkSKeyInit, LoRaMacAppSKeyInit, LoRaMacAppKeyInit, AppEuiInit, DevEuiInit, LoRaDevAddrInit,OTA_DEVICE};

    /*!
    * \brief   Lp<LoraRegionsEU>: A LoRaWan Object with Eu region's rules. 
    * \remark  The Current implementation  support radio SX1276 and sx1261
    */
    SX1276  RadioUser( LORA_CS, LORA_RESET, TX_RX_IT, RX_TIMEOUT_IT);
    LoraWanObject<LoraRegionsEU,SX1276> Lp( LoraWanKeys,&RadioUser,USERFLASHADRESS); 
    //SX126x  RadioUser( LORA_BUSY, LORA_CS, LORA_RESET,TX_RX_IT );
    //LoraWanObject<LoraRegionsEU,SX126x> Lp( LoraWanKeys,&RadioUser,USERFLASHADRESS); 


    uint8_t AvailableRxPacket = NO_LORA_RXPACKET_AVAILABLE ;
    eLoraWan_Process_States LpState = LWPSTATE_IDLE;    

    /*!
    * \brief Restore the LoraWan Context
    */
		DEBUG_MSG("MM is starting ...\n\n");
    mcu.mwait(2);
    Lp.RestoreContext  ( );

    while(1) {
        
        /*!
         * \brief  For this example : send an un confirmed message on port 3 . The user payload is a ramp from 0 to 13 (14 bytes). 
        */
        UserFport       = 3;
        UserPayloadSize = 9;
        MsgType = UNCONF_DATA_UP;
        PrepareFrame ( UserPayload );
        UserPayload[8]  = Lp.GetNbOfReset(); // in this example adding number of reset inside the applicatif payload
        /*!
         * \brief  Configure a new DataRate Strategy 
        */
        Lp.SetDataRateStrategy( AppDataRate );
        if ( Lp.IsJoined ( ) == NOT_JOINED ) {            
            LpState = Lp.Join( );
        } else {
            LpState = Lp.SendPayload( UserFport, UserPayload, UserPayloadSize, MsgType );
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
        while ( LpState != LWPSTATE_IDLE ){
            LpState = Lp.LoraWanProcess( &AvailableRxPacket );
            mcu.GotoSleepMSecond ( 300 );
            mcu.WatchDogRelease ( );
        }
        if ( LpState == LWPSTATE_ERROR ) {
        // user application have to save all the need
            NVIC_SystemReset();
        }
        if ( AvailableRxPacket == LORA_RX_PACKET_AVAILABLE ) { 
            Lp.ReceivePayload( &UserRxFport, UserRxPayload, &UserRxPayloadSize );
            DEBUG_PRINTF("Receive on port %d  an Applicative Downlink \n DATA[%d] = [ ",UserRxFport,UserRxPayloadSize);
            for ( i = 0 ; i < UserRxPayloadSize ; i++){
                DEBUG_PRINTF( "0x%.2x ",UserRxPayload[i]);
            }
            DEBUG_MSG("]\n\n\n");
            // call the application layer to manage the application downlink
        } 
        /*!
         * \brief Send a ¨Packet every 120 seconds in case of join 
         *        Send a packet every AppTimeSleeping seconds in normal mode
         */

        if ( Lp.IsJoined ( ) == NOT_JOINED ) {
            mcu.GotoSleepSecond(5);
        } else {
            mcu.GotoSleepSecond ( AppTimeSleeping );
        }
        
    }
}

