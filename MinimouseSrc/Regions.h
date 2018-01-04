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
    

    
public: 
    LoraRegionsEU (  PinName interrupt ); 
    virtual void SetRegionsdefaultSettings       ( void );
    virtual void RegionGiveNextDataRate          ( void );
    virtual void RegionSetRxConfig               ( eRxWinType type );
    virtual eStatusLoRaWan isValidRx1DrOffset     ( uint8_t Rx1DataRateOffset );
    virtual eStatusLoRaWan isValidMacRx2Dr        ( uint8_t Rx2DataRate );
    virtual eStatusLoRaWan isValidMacFrequency    ( uint32_t Frequency);
/*********************************************************************/
/*            Define Regional parameter                              */
/*********************************************************************/
    static const int      JOIN_ACCEPT_DELAY1 = 5 ; // define in seconds
    static const int      JOIN_ACCEPT_DELAY2 = 6 ; // define in seconds
    static const int      RECEIVE_DELAY1     = 1 ; // define in seconds
    static const int      RECEIVE_DELAY2     = 2 ; // define in seconds
    static const int      ADR_ACK_LIMIT      = 64 ;
    static const int      ADR_ACK_DELAY      = 32 ;
    static const int      ACK_TIMEOUT        = 2 ;// +/- 1 s (random delay between 1 and 3 seconds)
    static const uint32_t FREQMIN            = 8630000 ;// MHz/100 coded over 24 bits
    static const uint32_t FREQMAX            = 8700000 ;// MHz/100 coded over 24 bits
private :

    void TxDataRateToSfBw                  ( uint8_t dataRate );
    void Rx2DataRateToSfBw                 ( uint8_t dataRate );
};
#endif

