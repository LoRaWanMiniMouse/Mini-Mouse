/*

  __  __ _       _                                 
 |  \/  ( _)     ( _)                                
 | \  / |_ _ __  _ _ __ ___   ___  _   _ ___  ___  
 | |\/| | | '_ \| | '_ ` _ \ / _ \| | | / __|/ _ \
 | |  | | | | | | | | | | | | ( _) | |_| \__ \  __/ 
 |_|  |_|_|_| |_|_|_| |_| |_|\___/ \__,_|___/\___| 
                                                   
                                                   
Description       : LoraWan Regions Specific objets.  


License           : Revised BSD License, see LICENSE.TXT file include in the project

Maintainer        : Fabien Holin ( SEMTECH)
*/
#ifndef REGIONS_H
#define REGIONS_H
#include "mbed.h"
#include  "Define.h"
#include "MacLayer.h"

/*****************************************************************/
/*         Define specific for EU 863-870                        */
/*****************************************************************/





class LoraRegionsEU : public LoraWanContainer { 
    
#define RECEIVE_DELAY1          1 // define in seconds
#define RECEIVE_DELAY2          2 // define in seconds (must be RECEIVE_DELAY1 + 1s)
#define JOIN_ACCEPT_DELAY1      5 // define in seconds
#define JOIN_ACCEPT_DELAY2      6 // define in seconds
#define MAX_FCNT_GAP            16384
#define ADR_ACK_LIMIT           64
#define ADR_ACK_DELAY           32
#define ACK_TIMEOUT             2 // +/- 1 s (random delay between 1 and 3 seconds)
#define FREQMIN                 8630000  // code on 24 bits 
#define FREQMAX                 8700000
    
public: 
    LoraRegionsEU (  PinName interrupt ); 
    void SetRegionsdefaultSettings       ( void );
    void RegionGiveNextDataRate          ( void );
    void RegionSetRxConfig               ( eRxWinType type );



    eStatusLoRaWan isValidRx1DrOffset     ( uint8_t Rx1DataRateOffset );
    eStatusLoRaWan isValidMacRx2Dr        ( uint8_t Rx2DataRate );
    eStatusLoRaWan isValidMacFrequency    ( uint32_t Frequency);
private :
    void TxDataRateToSfBw                  ( uint8_t dataRate );
    void Rx2DataRateToSfBw                 ( uint8_t dataRate );
};
#endif

