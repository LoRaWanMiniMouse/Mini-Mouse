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
#include "Fragmentation.h"
#include "FragmentationDecode.h"

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
uint8_t LoRaMacNwkSKeyInit[] = { 0x23, 0x33, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0xFF, 0x23, 0x03, 0x4D, 0xE4, 0x11, 0x11, 0x11};
uint8_t LoRaMacAppSKeyInit[] = { 0x12, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0xE4, 0xBD, 0x29};
uint8_t LoRaMacAppKeyInit[]  = { 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0xBB};
uint8_t AppEuiInit[]         = { 0x70, 0xb3, 0xd5, 0x7e, 0xd0, 0x00, 0xff, 0x50 };
//uint8_t AppEuiInit[]         = { 0x11, 0x22, 0x33, 0x44, 0x44, 0x33, 0x22, 0x22 };
uint8_t DevEuiInit[]         = { 0x11, 0x22, 0x33, 0x44, 0x44, 0x33, 0xcc, 0xbb };    
uint32_t LoRaDevAddrInit     = 0x26011920;
   

SX1276  RadioUser( LORA_CS, LORA_RESET, TX_RX_IT, RX_TIMEOUT_IT);
/* User Radio ISR routine */
#define NBENCODEDFRAME 52
void UserRadioIsrFuota ( void ) {
	  int16_t snr,rssi;
	  uint32_t crcL, crcH;
	  uint8_t PayloadSize;
	  uint8_t FuotaPayload[255];
	  uint16_t NbFrame;
	  uint8_t  FrameLength;
	  static uint32_t endOfTest = 0xFFFFFFFF;
	  static uint8_t FirstTime = 0;
	  uint8_t tempvec[255];
    mcu.WatchDogRelease ( ); 
	  int IrqFlag = RadioUser.GetIrqFlagsLora( );
		RadioUser.ClearIrqFlagsLora( ); 
    if ( IrqFlag == RECEIVE_PACKET_IRQ_FLAG ) {
        RadioUser.FetchPayloadLora(&PayloadSize, FuotaPayload, &snr, &rssi );
		  	RadioUser.Sleep ( false );
			  if ((FuotaPayload[1] == 0xF) && (FuotaPayload[2] == 0xA) && (FuotaPayload[3] == 0xB) && (FuotaPayload[4] == 0x4)){
						NbFrame = (FuotaPayload[7]<<8) + FuotaPayload[8];
						FrameLength = FuotaPayload[9];
						if (FirstTime == 0){
								FirstTime =1;
								FotaParameterInit( NbFrame-NBENCODEDFRAME,NBENCODEDFRAME, FrameLength+2);
								DEBUG_MSG("Start FW upgrade \n");
						}
						for (int jj = 0 ; jj < FrameLength ; jj++){
								tempvec[jj+2] =  FuotaPayload[jj+12];
						}
						Crc64(&tempvec[2], FrameLength,&crcL, &crcH );
						tempvec[0] = FuotaPayload[5];
						tempvec[1] = FuotaPayload[6];
						if (endOfTest == 0xFFFFFFFF) {
								if (( FuotaPayload[10]  ==  ( crcL & 0xff )) && (FuotaPayload[11]  ==  ( ( crcL >> 8) & 0xff ))){  // check crc
										endOfTest = FragmentationDecodeCore( &tempvec[0], 0);
								}
						} else {
								DEBUG_MSG("end of defrag\n");
								FLASH_If_BankSwitch();
						}
        }
	  RadioUser.RxLora  (BW125, 7, 868100000 , 10000);			
    } else {
		    RadioUser.RxLora  (BW125, 7, 868100000 , 10000);
		}
}

int main( ) {
    int i;
    uint8_t UserPayloadSize ;
    uint8_t UserPayload [255];
    uint8_t UserRxPayloadSize;
    uint8_t UserRxPayload [125];
    uint8_t UserFport ;
    uint8_t UserRxFport ;
    uint8_t MsgType ;
    eDataRateStrategy AppDataRate = STATIC_ADR_MODE; // adr mode manage by network
    uint8_t AppTimeSleeping = 6 ;
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

    LoraWanObject<LoraRegionsEU,SX1276> Lp( LoraWanKeys,&RadioUser,USERFLASHADRESS); 
	  //SX126x  RadioUser( LORA_BUSY, LORA_CS, LORA_RESET,TX_RX_IT );
  	//LoraWanObject<LoraRegionsEU,SX126x> Lp( LoraWanKeys,&RadioUser,USERFLASHADRESS); 


    uint8_t AvailableRxPacket = NO_LORA_RXPACKET_AVAILABLE ;
    eLoraWan_Process_States LpState = LWPSTATE_IDLE;    

    /*!
    * \brief Restore the LoraWan Context
    */
		DEBUG_PRINTF("MM is starting ...{ %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x }",uid[0],uid[1],uid[2],uid[3],uid[4],uid[5],uid[6],uid[7]);

		DEBUG_MSG("********************Debug Trace In flash****************** \n\n");
    ReadTraceInFlash ( USERFLASHADRESS + 4096 );
		
		DEBUG_MSG("******************** Current Debug Trace****************** \n\n");
		ReadTrace ( ExtDebugTrace );
		uint8_t TPointer ;
	  TPointer = ExtDebugTrace[ TRACE_SIZE - 1]& 0xff;
    
		for (int i  = 0; i < 200; i++){ 
		    UserPayload [i] = (ExtDebugTrace[(uint8_t)(TPointer - i)] & 0xFF);
		}
	  StoreTraceInFlash( USERFLASHADRESS + 4096 );
    mcu.mwait(2);
		InsertTrace ( __COUNTER__, FileId );
    Lp.RestoreContext  ( );

    uint32_t FrameCounterUpTest  = 1;
		uint32_t FrameCounterDwnTest = 0;
    while(1) {
        
        /*!
         * \brief  For this example : send an un confirmed message on port 3 . The user payload is a ramp from 0 to 13 (14 bytes). 
        */
			  if ( FrameCounterUpTest > 10 ) { 
						UserFport       = 3;
						UserPayloadSize = randr( 9, 40 );
						memset( UserPayload, 0, UserPayloadSize);
						MsgType = UNCONF_DATA_UP;
						//PrepareFrame ( UserPayload );
						UserPayload[ 0 ]  =  FrameCounterUpTest >> 24;
						UserPayload[ 1 ]  = (FrameCounterUpTest >> 16) & 0xFF;
						UserPayload[ 2 ]  = (FrameCounterUpTest >> 8) & 0xFF;
						UserPayload[ 3 ]  =  FrameCounterUpTest & 0xFF;
						UserPayload[ 4 ]  =  FrameCounterDwnTest >> 24;
						UserPayload[ 5 ]  = (FrameCounterDwnTest >> 16) & 0xFF;
						UserPayload[ 6 ]  = (FrameCounterDwnTest >> 8) & 0xFF;
						UserPayload[ 7 ]  =  FrameCounterDwnTest & 0xFF;
						UserPayload[ 8 ]  = Lp.GetNbOfReset(); // in this example adding number of reset inside the applicatif payload
						Lp.SetDataRateStrategy( AppDataRate );
				} else {  // send Trace
					  UserFport       = 4;
						MsgType = UNCONF_DATA_UP;
						Lp.SetDataRateStrategy( MOBILE_LOWPER_DR_DISTRIBUTION );
					  UserPayloadSize = 210 ;
				}
					
					
        if ( ( Lp.IsJoined ( ) == NOT_JOINED ) && ( Lp.GetIsOtaDevice ( ) == OTA_DEVICE) ) {       
            InsertTrace ( __COUNTER__, FileId );					
            LpState = Lp.Join( );
        } else {
					  InsertTrace ( __COUNTER__, FileId );
            LpState = Lp.SendPayload( UserFport, UserPayload, UserPayloadSize, MsgType );
					  FrameCounterUpTest ++;
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
            mcu.GotoSleepMSecond ( 100 );
            mcu.WatchDogRelease ( );
        }
				
        if ( LpState == LWPSTATE_ERROR ) {
					  InsertTrace ( __COUNTER__, FileId );
        // user application have to save all the need
            NVIC_SystemReset();
        }
        if ( AvailableRxPacket == LORA_RX_PACKET_AVAILABLE ) { 
					  InsertTrace ( __COUNTER__, FileId );
            Lp.ReceivePayload( &UserRxFport, UserRxPayload, &UserRxPayloadSize );
					  FrameCounterDwnTest ++;
            DEBUG_PRINTF("Receive on port %d  an Applicative Downlink \n DATA[%d] = [ ",UserRxFport,UserRxPayloadSize);
            for ( i = 0 ; i < UserRxPayloadSize ; i++){
                DEBUG_PRINTF( "0x%.2x ",UserRxPayload[i]);
            }
            DEBUG_MSG("]\n\n\n");
						if ( ( UserRxFport == 0x69 )&& ( UserRxPayloadSize == 5 ) ) {
								int GoInFwUpgrade = 0;
								for (int i = 0; i < 5 ; i++) {
									GoInFwUpgrade = (UserRxPayload[i]-i == 0) ? GoInFwUpgrade : 1;
								}
								if ( GoInFwUpgrade == 0 ){
										Lp.SetDataRateStrategy(USER_DR_DISTRIBUTION);
										LpState = Lp.SendPayload( 0x69, UserPayload, UserPayloadSize, MsgType );
										while ( ( LpState != LWPSTATE_IDLE ) && ( LpState != LWPSTATE_ERROR ) ){
												LpState = Lp.LoraWanProcess( &AvailableRxPacket );
												mcu.GotoSleepMSecond ( 100 );
												mcu.WatchDogRelease ( );
										}
										mcu.AttachInterruptIn( &UserRadioIsrFuota ); // attach ISR
										mcu.mwait(1);
										RadioUser.RxLora  (BW125, 7, 868100000 , 10000);
										uint32_t BeginFWUpgrade = mcu.RtcGetTimeSecond( );
										while(1) {
												if ( ( mcu.RtcGetTimeSecond( ) - BeginFWUpgrade ) > 3600 ) {
														 DEBUG_MSG ( "ERROR : TOO LATE FOR FW UPGRADE  \n");
														 NVIC_SystemReset() ;
												}
												 mcu.GotoSleepSecond ( AppTimeSleeping);
										}
						    }
				    }
				}
        /*!
         * \brief Send a ¨Packet every 120 seconds in case of join 
         *        Send a packet every AppTimeSleeping seconds in normal mode
         */
				
        if ( ( Lp.IsJoined ( ) == NOT_JOINED ) && ( Lp.GetIsOtaDevice ( ) == OTA_DEVICE) && ( LpState != LWPSTATE_INVALID)){
					  InsertTrace ( __COUNTER__, FileId ); 
            mcu.GotoSleepSecond(5);
        } else {
					  InsertTrace ( __COUNTER__, FileId );
            mcu.GotoSleepSecond ( AppTimeSleeping );
					  InsertTrace ( __COUNTER__, FileId );
        }
    }
}

