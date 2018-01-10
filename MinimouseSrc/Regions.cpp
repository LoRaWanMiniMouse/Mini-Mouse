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
#include "utilities.h"

/*************************************************/
/*                     Constructors              */
/*@note have to check init values                */
/*************************************************/
LoraRegionsEU :: LoraRegionsEU (  PinName interrupt ) : LoraWanContainer (interrupt){
    memset( MacChannelIndexEnabled, CHANNEL_DISABLED, NUMBER_OF_CHANNEL );
    MacChannelIndexEnabled [0] = CHANNEL_ENABLED;
    MacChannelIndexEnabled [1] = CHANNEL_ENABLED;
    MacChannelIndexEnabled [2] = CHANNEL_ENABLED;
    MacMinDataRateChannel [0] = 0;
    MacMinDataRateChannel [1] = 0;
    MacMinDataRateChannel [2] = 0;
    MacMaxDataRateChannel [0] = 5;
    MacMaxDataRateChannel [1] = 5;
    MacMaxDataRateChannel [2] = 5;
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

void  LoraRegionsEU::RegionGiveNextChannel( void ) {
    uint8_t temp =  randr( 0, NbOfActiveChannel - 1 ) ;
    int i = 0;
    int ChannelIndex = 0;
    eValidChannel status = UNVALIDCHANNEL;
    ChannelIndex = FindEnabledChannel ( temp ); // @note datarate valid not yett tested
    if ( ChannelIndex == -1 ) {
        DEBUG_MSG ("INVALID CHANNEL\n");
    } else {
        MacTxFrequencyCurrent = MacTxFrequency[ChannelIndex];
    }

};

void LoraRegionsEU::RegionSetRxConfig ( eRxWinType type ) {
    if ( type == RX1 ) {
        MacRx1SfCurrent =  ( MacTxSfCurrent < 12 - MacRx1DataRateOffset) ? MacTxSfCurrent + MacRx1DataRateOffset : 12;
        MacRx1BwCurrent = MacTxBwCurrent;
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
void LoraRegionsEU::RegionInitChannelMask ( void ) {
    UnwrappedChannelMask = 0;
};
void LoraRegionsEU::RegionSetMask ( void ) {
    int i;
    int cpt = 0;
    for (i = 0 ; i < NUMBER_OF_CHANNEL ; i ++ ) {
            MacChannelIndexEnabled [i] = ( UnwrappedChannelMask >> i ) & 0x1;
        if ( ( ( UnwrappedChannelMask >> i ) & 0x1) == 1 ) {
            cpt ++;
        }
    }
    NbOfActiveChannel = cpt ;
};
eStatusChannel LoraRegionsEU::RegionBuildChannelMask ( uint8_t ChMaskCntl, uint16_t ChMask ) {
    eStatusChannel status = OKCHANNEL;
    switch ( ChMaskCntl ) {
        case 0 :
            UnwrappedChannelMask = UnwrappedChannelMask ^ ChMask;
            for ( int i = 0 ; i < NUMBER_OF_CHANNEL ; i++) {
                if ( ( ( ( UnwrappedChannelMask >> i) & 0x1 ) == 1 ) && ( MacTxFrequency[i] == 0) ) {  // test channel not defined
                    status = ERRORCHANNELMASK ;   //@note this status is used only for the last multiple link adr req
                }
            }
            break;
        case 6 :
            for ( int i = 0 ; i < NUMBER_OF_CHANNEL ; i++) {
                if ( MacTxFrequency[i] > 0 ) {
                    UnwrappedChannelMask = UnwrappedChannelMask ^ (1 << i ) ;
                }
            }
            break;
        default : 
             status = ERRORCHANNELCNTL;//@note tbd if at least on chMaskctl is not valid i reject the multiple linkadr
    }
    if ( UnwrappedChannelMask == 0 ) {
         status = ERRORCHANNELMASK ; 
    }        
    return ( status );
};
void LoraRegionsEU::RegionGetCFList ( void ) {
    int cpt = 0 ;
    for ( int i = 0 ; i < 5 ; i++ ) {
        MacTxFrequency [3 + i] = 100 * ( ( CFList[0 + ( 3 * i )] ) + ( CFList[1 + ( 3 * i )] << 8 )+ ( CFList[2 + ( 3 * i )] << 16 ) );
        if ( ( MacTxFrequency [3 + i] >= 863000000) && ( MacTxFrequency [3 + i] <= 870000000) ) {
            cpt++;
            MacMinDataRateChannel [3 + i] = 0;
            MacMaxDataRateChannel [3 + i] = 5;
            MacChannelIndexEnabled [3 + i] = CHANNEL_ENABLED ;
            DEBUG_PRINTF( " MacTxFrequency [%d] = %d \n",i,MacTxFrequency [3 + i]);
            DEBUG_PRINTF( " MacMinDataRateChannel [%d] = %d \n",i,MacMinDataRateChannel [3 + i]);
            DEBUG_PRINTF( " MacMaxDataRateChannel [%d] = %d \n",i,MacMaxDataRateChannel [3 + i]);
            DEBUG_PRINTF( " MacChannelIndexEnabled [%d] = %d \n",i,MacChannelIndexEnabled [3 + i]);
        }
    }
    NbOfActiveChannel += cpt ;
    DEBUG_PRINTF( " NbOfActiveChannel = %d \n",NbOfActiveChannel);
}
/********************************************************************************/
/*           Check parameter of received mac commands                           */
/********************************************************************************/
eStatusLoRaWan LoraRegionsEU::RegionIsValidRx1DrOffset ( uint8_t Rx1DataRateOffset ) {
    eStatusLoRaWan status = OKLORAWAN;
    if (Rx1DataRateOffset > 5) {
        status = ERRORLORAWAN ;
        DEBUG_MSG ( "RECEIVE AN INVALID RX1 DR OFFSET \n");
    }
    return ( status );
}

eStatusLoRaWan LoraRegionsEU:: RegionIsValidDataRateRx2 ( uint8_t temp ) {
    eStatusLoRaWan status ;
    status = ( temp > 7) ? ERRORLORAWAN : OKLORAWAN;
    return ( status );
}
    
eStatusLoRaWan LoraRegionsEU::RegionIsValidDataRate ( uint8_t DataRate ) {
    eStatusLoRaWan status = ERRORLORAWAN;
    for ( int i = 0 ; i < NUMBER_OF_CHANNEL; i++) {
        if ( ( ( UnwrappedChannelMask >> i) & 0x1) == 1 ) {
            if ( ( DataRate >= MacMinDataRateChannel [i] ) && ( DataRate <= MacMaxDataRateChannel [i] ) ) {
                return ( OKLORAWAN );
            }
        }
    }
    return ( status );
}
eStatusLoRaWan LoraRegionsEU::RegionIsValidMacFrequency ( uint32_t Frequency) {
    eStatusLoRaWan status = OKLORAWAN;
    if ( ( Frequency > FREQMAX ) || ( Frequency < FREQMIN ) ) {
        status = ERRORLORAWAN ;
        DEBUG_PRINTF ( "RECEIVE AN INVALID FREQUENCY = %d\n", Frequency);
    }
    return ( status );
}
eStatusLoRaWan LoraRegionsEU::RegionIsValidTxPower ( uint8_t Power) {
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
        MacTxSfCurrent = 12 - dataRate ;
        MacTxBwCurrent = BW125 ;
    } else if ( dataRate == 6 ){ 
        MacTxSfCurrent = 7;
        MacTxBwCurrent = BW250 ;}
    else if ( dataRate == 7 ) {
        //@note tbd manage fsk case }
    }
    else {
        MacTxSfCurrent = 12 ;
        MacTxBwCurrent = BW125 ;
        DEBUG_MSG( " Invalid Datarate \n" ) ; 
    }
}
void LoraRegionsEU :: Rx2DataRateToSfBw ( uint8_t dataRate ) {
    if ( dataRate < 6 ){ 
        MacRx2SfCurrent = 12 - dataRate ;
        MacRx2BwCurrent = BW125 ;
    } else if ( dataRate== 6 ){ 
        MacRx2SfCurrent = 7;
        MacRx2BwCurrent = BW250 ;}
    else if ( dataRate == 7 ) {
        //@note tbd manage fsk case }
    }
    else {
        MacRx2SfCurrent = 12 ;
        MacRx2BwCurrent = BW125 ;
        DEBUG_MSG( " Invalid Datarate \n" ) ; 
    }
}

