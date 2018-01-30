#include "mbed.h"
#include "ApiFlash.h"
#include "LoraMacDataStoreInFlash.h"
#include "LoraWanProcess.h"
#include "Define.h"
#include "rtc_api.h"
#include "ApiRtc.h"





struct sBackUpFlash BackUpFlash;
//static LoraWanObjet<LoraRegionsEU> Lp( TX_RX_IT );
//static LoraWanObjet Lp( TX_RX_IT );
//@note set to board definition
#define CHECKFCNTDOWN 1
int UserPayloadSize = 14;
uint8_t UserRxPayloadSize;
uint8_t UserRxPayload [255];
uint8_t UserPayload[255];
uint8_t UserFport = 3;
uint8_t UserRxFport ;
uint8_t MsgType ;
uint16_t FcntDwnCertif = 0;
uint32_t MsgTypePrevious = UNCONFDATAUP ;
LoraWanObjet<LoraRegionsEU> Lp( TX_RX_IT ); // shouldn't be glabal just easier for certification application
int  Certification ( bool NewCommand ){
    uint32_t temp ;
    int i ;
    UserFport       = 224;
    UserPayloadSize = 2;
    MsgType = UNCONFDATAUP ;
    if ( NewCommand == true) {
        switch ( UserRxPayload[0] ) {
            case 0 :  // end of test
                UserFport       = 3;
                UserPayloadSize = 14;
                for ( i = 0; i < 14 ; i ++) {
                    UserPayload[i]  = i;
                }
                break;
            case 1 :
                temp =  ( UserRxPayload[0] << 24 ) + ( UserRxPayload[1] << 16 ) + ( UserRxPayload[2] << 8 ) + ( UserRxPayload[3] );
                if ( temp == 0x01010101) {
                     Lp.SetDataRateStrategy( STATIC_ADR_MODE );
                     FcntDwnCertif   = 0;
                     UserPayload[0]  = FcntDwnCertif >> 8;
                     UserPayload[1]  = FcntDwnCertif & 0xFF;
                }
                break;            
            case 2 :  // Confirmed Uplink
                MsgType = CONFDATAUP ; 
                MsgTypePrevious = MsgType;
                UserPayload[0]  = FcntDwnCertif >> 8;
                UserPayload[1]  = FcntDwnCertif & 0xFF;
                break;
            case 3 :  // UnConfirmed Uplink
                MsgType = UNCONFDATAUP ;
                MsgTypePrevious = MsgType;   
                UserPayload[0]  = FcntDwnCertif >> 8;
                UserPayload[1]  = FcntDwnCertif & 0xFF;            
                break;
            case 4 :  //echo payload
                UserPayloadSize = UserRxPayloadSize;
                UserPayload[0] = 4;
                for ( i = 1 ; i < UserPayloadSize; i++ ) {
                    UserPayload[i]  = UserRxPayload [i] + 1;
                }
                break;
            case 5 :  // link check request 
              UserPayloadSize = 1;
              UserPayload[0]  = 2;
              UserFport       = 0;
            case 6 :  // rejoin 
               Lp.NewJoin( );
               break;
            default :
                break;
        }
        FcntDwnCertif++;
    } else { // for the case of echo cmd
         MsgType = MsgTypePrevious;
    }
    return ( UserRxPayload[0] );
}

int main( ) {
    int i;
    int StatusCertification = 0;
    int StatusCertificationp = 0;
    my_rtc_init ( );
    pcf.baud( 115200 );
    
    UserFport       = 3;
    UserPayloadSize = 14;
    for (int i = 0; i < 14 ; i ++) {
        UserPayload[i]  = i;
    }
    MsgType = UNCONFDATAUP;
    uint8_t AvailableRxPacket = NOLORARXPACKETAVAILABLE ;
    eLoraWan_Process_States LpState = LWPSTATE_IDLE;    
    
    /************************************************/
    /*          Configure Adr Mode                  */
    /************************************************/
    Lp.SetDataRateStrategy( MOBILE_LOWPER_DR_DISTRIBUTION );

    /************************************************/
    /*           Restore Context from Flash         */
    /* fcnt up is incemented by FLASH_UPDATE_PERIOD */
    /************************************************/
    //Lp.RestoreContext ( );
    //Lp.NewJoin( );
    

    while(1) {
        pcf.printf("\n\n\n\n ");

        if ( Lp.IsJoined ( ) == JOINED ) {            
            LpState = Lp.SendPayload( UserFport, UserPayload, UserPayloadSize, MsgType );
        } else {
            LpState = Lp.Join( );
        }
        
        
        while ( LpState != LWPSTATE_IDLE ){
            LpState = Lp.LoraWanProcess( &AvailableRxPacket );
            wait_ms( 100 );
        }

        if ( AvailableRxPacket == LORARXPACKETAVAILABLE ) { 
            Lp.ReceivePayload( &UserRxFport, UserRxPayload, &UserRxPayloadSize );
            pcf.printf("Receive on port %d  an Applicative Downlink \n DATA[%d] = [ ",UserRxFport,UserRxPayloadSize);
            for ( i = 0 ; i < UserRxPayloadSize ; i++){
                pcf.printf( "0x%.2x ",UserRxPayload[i]);
                
            }
            pcf.printf("]\n");
            if ( ( UserRxFport == 224 ) || ( UserRxPayloadSize == 0 ) ) {
               pcf.printf("Receive Certification Payload \n"); 
               StatusCertification = Certification ( true );
            } 
        } else {
            if ( StatusCertification > 0 ){
                Certification ( false );
            }
        }

        wait_s( 5 ); 
    }
}
