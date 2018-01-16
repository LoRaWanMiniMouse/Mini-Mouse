#include "mbed.h"
#include "ApiFlash.h"
#include "LoraMacDataStoreInFlash.h"
#include "LoraWanProcess.h"
#include "Define.h"
#include "rtc_api.h"
#include "ApiRtc.h"


DigitalOut led( D13 );


struct sBackUpFlash BackUpFlash;
//static LoraWanObjet<LoraRegionsEU> Lp( TX_RX_IT );
//static LoraWanObjet Lp( TX_RX_IT );
//@note set to board definition
//#define CHECKFCNTDOWN 0


int main( ) {
    LoraWanObjet<LoraRegionsEU> Lp( TX_RX_IT );
    int i;
    led = 0;
    my_rtc_init ( );
    pcf.baud( 115200 );
    int UserPayloadSize = 14;
    uint8_t UserRxPayloadSize;
    uint8_t UserRxPayload [255];
    uint8_t UserPayload[UserPayloadSize];
    uint8_t UserFport = 3;
    uint8_t UserRxFport ;
    for ( i = 0; i < UserPayloadSize; i++ ){
        UserPayload[i] = i + 20;
    }
    uint8_t AvailableRxPacket = NOLORARXPACKETAVAILABLE ;
    eLoraWan_Process_States LpState = LWPSTATE_IDLE;    
    
    /************************************************/
    /*          Configure Adr Mode                  */
    /************************************************/
    Lp.SetDataRateStrategy( STATICADRMODE );

    /************************************************/
    /*           Restore Context from Flash         */
    /* fcnt up is incemented by FLASH_UPDATE_PERIOD */
    /************************************************/
    Lp.RestoreContext ( );
//@note join procedure, rajouter rejoin apres in adr rajouter fonction rejoin, protected lp.
    while(1) {
        pcf.printf("\n\n\n\n ");
        if ( Lp.IsJoined ( ) == JOINED ) {            
            pcf.printf("send payload \n");
            LpState = Lp.SendPayload( UserFport, UserPayload, UserPayloadSize, UNCONFDATAUP );
        } else {
            LpState = Lp.Join( );
        }
        
        led = 1;
        while ( LpState != LWPSTATE_IDLE ){
            LpState = Lp.LoraWanProcess( &AvailableRxPacket );
            wait_ms( 100 );
        }
        if ( AvailableRxPacket == LORARXPACKETAVAILABLE ) {
            Lp.ReceivePayload( &UserRxFport, UserRxPayload, &UserRxPayloadSize );
            pcf.printf("Receive an Applicative Downlink \n DATA[%d] = [ ",UserRxPayloadSize);
            for ( i = 0 ; i < UserRxPayloadSize ; i++){
                pcf.printf( "0x%.2x ",UserRxPayload[i]);
            }
            pcf.printf("]\n");
        }
        led = 0;
        for (int j = 0 ; j < 10; j ++) {
        wait_s( 6 );
        }

    }
}
