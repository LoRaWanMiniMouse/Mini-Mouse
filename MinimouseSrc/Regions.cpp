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

#include "Regions.h"
#include "Define.h"


/*************************************************/
/*                     Constructors              */
/*@note have to check init values                */
/*************************************************/
LoraRegionsEU :: LoraRegionsEU (  PinName interrupt ) : LoraWanContainer (interrupt){
    MacTxFrequency[0]    = 868100000;
    MacTxFrequency[1]    = 868300000;
    MacTxFrequency[2]    = 868500000;
    NbOfActiveChannel    = 3;
    MacRx2Frequency      = 869525000; 
    MacRx1DataRateOffset = 0;
    MacRx2DataRate       = 0;
    MacRx1Delay          = RECEIVE_DELAY1;// @note replace by default setting regions
    MacTxDataRateAdr     = 0 ;//@note tbdN if adr active , before the first adrReq datarate = 0  
}

/***********************************************************************************************/
/*                      Public  Methods                                                        */
/***********************************************************************************************/
 //@note Partionning Public/private not yet finalized
void LoraRegionsEU :: SetRegionsdefaultSettings ( void ) {
    
}

void LoraRegionsEU::RegionGiveNextDataRate( void ) {
     switch ( AdrModeSelect ) {
        case STATICADRMODE :
            MacTxDataRate = MacTxDataRateAdr;
            break;
        case MOBILELONGRANGEADRMODE:
            if ( MacTxDataRate == 0 ) { 
                MacTxDataRate = 1;
            } else {
                MacTxDataRate = 0;
            }
            break;
        case MOBILELOWPOWERADRMODE:
            ( MacTxDataRate == 5 ) ? MacTxDataRate = 0 : MacTxDataRate++ ;
             break;
        default: 
           MacTxDataRate = 0;
    }
    MacTxDataRate = ( MacTxDataRate > 5 ) ? 5 : MacTxDataRate;
    TxDataRateToSfBw ( MacTxDataRate );
}

void LoraRegionsEU::RegionSetRxConfig ( eRxWinType type ) {
    if ( type == RX1 ) {
        MacRx1Sf =  ( MacTxSf < 12 - MacRx1DataRateOffset) ? MacTxSf + MacRx1DataRateOffset : 12;
        MacRx1Bw = MacTxBw;
    } else if ( type == RX2 ) {
       Rx2DataRateToSfBw ( MacRx2DataRate );
    } else {
        DEBUG_MSG ("INVALID RX TYPE \n");
    }
}
void LoraRegionsEU::RegionSetPower ( uint8_t PowerCmd ) {
    switch ( PowerCmd ) {
        case 0 :
           MacTxPower = 20 ;
           break;
        case 1 :
           MacTxPower = 14 ;
           break;
        case 2 :
           MacTxPower = 11 ;
           break;
        case 3 :
           MacTxPower = 8 ;
           break;
        case 4 :
           MacTxPower = 5 ;
           break;
        case 5 :
           MacTxPower = 2 ;
           break;
        default :
           MacTxPower = 14 ;
           DEBUG_MSG ("INVALID POWER \n");
    }
}
/********************************************************************************/
/*           Chack parameter of received mac commands                           */
/********************************************************************************/
eStatusLoRaWan LoraRegionsEU::isValidRx1DrOffset ( uint8_t Rx1DataRateOffset ) {
    eStatusLoRaWan status = OKLORAWAN;
    if (Rx1DataRateOffset > 5) {
        status = ERRORLORAWAN ;
        DEBUG_MSG ( "RECEIVE AN INVALID RX1 DR OFFSET \n");
    }
    return ( status );
}

eStatusLoRaWan LoraRegionsEU::isValidDataRate ( uint8_t DataRate ){
    eStatusLoRaWan status = OKLORAWAN;
    if (DataRate > 7) {//@note must be impossible because RX2datarATe send over 3 bits !
        status = ERRORLORAWAN ;
        DEBUG_MSG ( "RECEIVE AN INVALID RX2 DR \n");
    }
    return ( status );
}
eStatusLoRaWan LoraRegionsEU::isValidMacFrequency ( uint32_t Frequency) {
    eStatusLoRaWan status = OKLORAWAN;
    if ( ( Frequency > FREQMAX ) || ( Frequency < FREQMIN ) ) {
        status = ERRORLORAWAN ;
        DEBUG_PRINTF ( "RECEIVE AN INVALID FREQUENCY = %d\n", Frequency);
    }
    return ( status );
}
eStatusLoRaWan LoraRegionsEU::isValidTxPower ( uint8_t Power) {
    eStatusLoRaWan status = OKLORAWAN;
    if ( ( Power > 5 ) ) {
        status = ERRORLORAWAN ;
        DEBUG_PRINTF ( "RECEIVE AN INVALID Power Cmd = %d\n", Power);
    }
    return ( status );
}
/***********************************************************************************************/
/*                      Private  Methods                                                        */
/***********************************************************************************************/
 //@note Partionning Public/private not yet finalized
void LoraRegionsEU :: TxDataRateToSfBw ( uint8_t dataRate ) {
    if ( dataRate < 6 ){ 
        MacTxSf = 12 - dataRate ;
        MacTxBw = BW125 ;
    } else if ( dataRate== 6 ){ 
        MacTxSf = 7;
        MacTxBw = BW250 ;}
    else if ( dataRate == 7 ) {
        //@note tbd manage fsk case }
    }
    else {
        MacTxSf = 12 ;
        MacTxBw = BW125 ;
        DEBUG_MSG( " Invalid Datarate \n" ) ; 
    }
}
void LoraRegionsEU :: Rx2DataRateToSfBw ( uint8_t dataRate ) {
    if ( dataRate < 6 ){ 
        MacRx2Sf = 12 - dataRate ;
        MacRx2Bw = BW125 ;
    } else if ( dataRate== 6 ){ 
        MacRx2Sf = 7;
        MacRx2Bw = BW250 ;}
    else if ( dataRate == 7 ) {
        //@note tbd manage fsk case }
    }
    else {
        MacRx2Sf = 12 ;
        MacRx2Bw = BW125 ;
        DEBUG_MSG( " Invalid Datarate \n" ) ; 
    }
}

