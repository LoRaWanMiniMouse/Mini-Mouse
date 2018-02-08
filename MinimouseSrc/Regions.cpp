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
LoraRegionsEU :: LoraRegionsEU ( sLoRaWanKeys LoRaWanKeys ) : LoraWanContainer<16>  ( LoRaWanKeys ){
    
    memset( MacChannelIndexEnabled, CHANNEL_DISABLED, NUMBER_OF_CHANNEL );
    memset( MacMinDataRateChannel, 0, NUMBER_OF_CHANNEL );
    for (int i = 0 ; i < NUMBER_OF_CHANNEL ; i ++ ) {
        MacTxFrequency [i] = 0;
        MacRx1Frequency [i] = 0;
    }
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
    MacRx1Frequency[0]   = 868100000;
    MacRx1Frequency[1]   = 868300000;
    MacRx1Frequency[2]   = 868500000;
    MacRx2Frequency      = 869525000; 
    MacTxPower           = TX_POWER;
    MacRx1DataRateOffset = 0;
    MacRx2DataRate       = RX2DR_INIT;
    MacRx1Delay          = RECEIVE_DELAY1;// @note replace by default setting regions
    MacTxDataRateAdr     = 0 ;//@note tbdN if adr active , before the first adrReq datarate = 0  
    memset(DistriDataRateInit,0,8);
}

/***********************************************************************************************/
/*                      Protected  Methods                                                     */
/***********************************************************************************************/
//@note Partionning Public/private not yet finalized


/********************************************************************/
/*                  Region Tx Power  Configuration                  */
/* Chapter 7.1.3 LoRaWan 1.0.1 specification                        */
/*  TXPower    Configuration                                        */
/* Max TX POwer is suppose = to 14                                  */
/*                                                                  */
/********************************************************************/

void LoraRegionsEU::RegionSetPower ( uint8_t PowerCmd ) {
    //@note return error status ?
    uint8_t PowerTab [ 8 ] = { TX_POWER, TX_POWER-2, TX_POWER-4, TX_POWER-6, TX_POWER-8, TX_POWER-10, TX_POWER-12, TX_POWER-14 };
    if  ( PowerCmd > 7 ) {
            MacTxPower = 14 ;
            DEBUG_MSG ("INVALID POWER \n");
    } else {
        MacTxPower = PowerTab [ PowerCmd ] ;
    }
}

/********************************************************************/
/*                  Region Get Cf List                              */
/* Chapter 7.1.4 LoRaWan 1.0.1 specification                        */
/********************************************************************/
void LoraRegionsEU::RegionGetCFList ( void ) {
    for ( int i = 0 ; i < 5 ; i++ ) {
        MacTxFrequency [3 + i] = 100 * ( ( CFList[0 + ( 3 * i )] ) + ( CFList[1 + ( 3 * i )] << 8 )+ ( CFList[2 + ( 3 * i )] << 16 ) );
        MacRx1Frequency [3 + i] = 100 * ( ( CFList[0 + ( 3 * i )] ) + ( CFList[1 + ( 3 * i )] << 8 )+ ( CFList[2 + ( 3 * i )] << 16 ) );
        if ( ( MacTxFrequency [3 + i] >= ( FREQMIN * 100 ) ) && ( MacTxFrequency [3 + i] <=( FREQMAX * 100 ) ) ) {
            MacMinDataRateChannel [3 + i]  = 0;
            MacMaxDataRateChannel [3 + i]  = 5;
            MacChannelIndexEnabled [3 + i] = CHANNEL_ENABLED ;
            DEBUG_PRINTF( " MacTxFrequency [%d] = %d \n",i,MacTxFrequency [3 + i]);
            DEBUG_PRINTF( " MacMinDataRateChannel [%d] = %d \n",i,MacMinDataRateChannel [3 + i]);
            DEBUG_PRINTF( " MacMaxDataRateChannel [%d] = %d \n",i,MacMaxDataRateChannel [3 + i]);
            DEBUG_PRINTF( " MacChannelIndexEnabled [%d] = %d \n",i,MacChannelIndexEnabled [3 + i]);
        } else {
            MacTxFrequency  [3 + i] = 0;
            MacRx1Frequency [3 + i] = 0;
            DEBUG_MSG ("INVALID TX FREQUENCY IN CFLIST \n");
        }
    }
}

/********************************************************************/
/*                  Region Set Channel MAsk                         */
/* Chapter 7.1.5 LoRaWan 1.0.1 specification                        */
/********************************************************************/

eStatusChannel LoraRegionsEU::RegionBuildChannelMask ( uint8_t ChMaskCntl, uint16_t ChMask ) {
    eStatusChannel status = OKCHANNEL;
    //@notereview modif tab uint8
    switch ( ChMaskCntl ) {
        case 0 :
            UnwrappedChannelMask = UnwrappedChannelMask ^ ChMask; //@note for EU UnwrappedChannelMask is still on 16 bits 
            for ( int i = 0 ; i < NUMBER_OF_CHANNEL ; i++) {
                if ( ( ( ( UnwrappedChannelMask >> i) & 0x1 ) == 1 ) && ( MacTxFrequency[i] == 0) ) {  // test channel not defined
                    status = ERROR_CHANNEL_MASK ;   //@note this status is used only for the last multiple link adr req
                }
            }
            break;
        case 6 :
            UnwrappedChannelMask = 0;
            for ( int i = 0 ; i < NUMBER_OF_CHANNEL ; i++) {
                if ( MacTxFrequency[i] > 0 ) {
                    UnwrappedChannelMask = UnwrappedChannelMask ^ (1 << i ) ;
                }
            }
            break;
        default : 
            status = ERROR_CHANNEL_CNTL;
    }
    if ( UnwrappedChannelMask == 0 ) {
        status = ERROR_CHANNEL_MASK ; 
    }        
    return ( status );
};

void LoraRegionsEU::RegionInitChannelMask ( void ) {
    UnwrappedChannelMask = 0;
};
void LoraRegionsEU::RegionSetMask ( void ) {
    DEBUG_MSG(" \n Mask = ");
    for (int i = 0 ; i < NUMBER_OF_CHANNEL ; i ++ ) {
        MacChannelIndexEnabled [i] = ( UnwrappedChannelMask >> i ) & 0x1; // @note trade off between size and code simplification
        DEBUG_PRINTF(" %d ",MacChannelIndexEnabled [i]);
    }
    DEBUG_MSG(" \n");
};
/********************************************************************/
/*                  Region MAx Payload SIze  Configuration          */
/* Chapter 7.1.6 LoRaWan 1.0.1 specification                        */
/********************************************************************/
eStatusLoRaWan LoraRegionsEU ::RegionMaxPayloadSize ( uint8_t sizeIn ) {
    eStatusLoRaWan  status ;
    uint8_t M [ 8 ] = { 59, 59, 59, 123, 230, 230, 230, 230 };
    status = ( sizeIn >= M [MacTxDataRate] ) ? ERRORLORAWAN : OKLORAWAN ;
    return ( status );
}

/********************************************************************/
/*                  Region Rx Window  Configuration                 */
/* Chapter 7.1.7 LoRaWan 1.0.1 specification                        */
/********************************************************************/
//@notereview return statusjhfrekhfkje dre
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

eStatusLoRaWan LoraRegionsEU:: RegionIsValidDataRate ( uint8_t temp ) {
    eStatusLoRaWan status ;
    status = ( temp > 7) ? ERRORLORAWAN : OKLORAWAN;
    return ( status );
}
    
eStatusLoRaWan LoraRegionsEU::RegionIsAcceptableDataRate ( uint8_t DataRate ) {
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
    if ( Frequency == 0) {
        return ( status );
    }
    if ( ( Frequency > FREQMAX ) || ( Frequency < FREQMIN ) ) {
        status = ERRORLORAWAN ;
        DEBUG_PRINTF ( "RECEIVE AN INVALID FREQUENCY = %d\n", Frequency);
    }
    return ( status );
}
eStatusLoRaWan LoraRegionsEU::RegionIsValidTxPower ( uint8_t Power) {
    eStatusLoRaWan status = OKLORAWAN;
    if ( ( Power > 7 ) ) {
        status = ERRORLORAWAN ;
        DEBUG_PRINTF ( "RECEIVE AN INVALID Power Cmd = %d\n", Power);
    }
    return ( status );
}
eStatusLoRaWan LoraRegionsEU::RegionIsValidChannelIndex ( uint8_t ChannelIndex) {
    eStatusLoRaWan status = OKLORAWAN;
    if ( ( ChannelIndex  < 3 ) || ( ChannelIndex  > 15 ) ) {
        status = ERRORLORAWAN ;
    }

    return ( status );
};


/********************************************************************************/
/*           RegionGiveNextDataRate                                             */
/*    method to set the next data Rate in different mode                        */
/********************************************************************************/

void LoraRegionsEU::RegionSetDataRateDistribution( uint8_t adrMode ) {
    memset(DistriDataRateInit,0 , 8);
    switch ( adrMode ) {

        case MOBILE_LONGRANGE_DR_DISTRIBUTION:  // in this example 4/7 dr2 2/7 dr1 and 1/7 dr0
            DistriDataRateInit[2]    = 4; 
            DistriDataRateInit[1]    = 2; 
            DistriDataRateInit[0]    = 1; 
            break;
        case MOBILE_LOWPER_DR_DISTRIBUTION://in this example 8/13 dr5 4/13 dr4 and 1/13 dr0
            DistriDataRateInit[7]    = 0; 
            DistriDataRateInit[5]    = 1; 
            DistriDataRateInit[3]    = 0; 
            DistriDataRateInit[0]    = 1; 
            break;
        case JOIN_DR_DISTRIBUTION: //in this example 1/3 dr5 1/3 dr4 and 1/3 dr0
            DistriDataRateInit[5]    = 0; 
            DistriDataRateInit[4]    = 0; 
            DistriDataRateInit[0]    = 1; 
            break;
        default: 
            DistriDataRateInit[0]    = 1; 
    }
    memcpy(DistriDataRate, DistriDataRateInit, 8);
}

void LoraRegionsEU::RegionGiveNextDataRate( void ) {
    if ( AdrModeSelect == STATIC_ADR_MODE ) {
        MacTxDataRate = MacTxDataRateAdr;
        AdrEnable = 1;
    } else {
        int i;
        uint8_t DistriSum = 0;
        for ( i= 0 ; i < 8; i++ ){
            DistriSum += DistriDataRate[i];
        }
        if ( DistriSum == 0) {
            memcpy(DistriDataRate,DistriDataRateInit,8);
        }
        uint8_t Newdr = randr(0,7);
        while (DistriDataRate[Newdr] == 0) {
            Newdr = randr(0,7);
        }
        MacTxDataRate = Newdr;
        DistriDataRate[Newdr] -- ;
        AdrEnable = 0;
    }
    MacTxDataRate = ( MacTxDataRate > 7 ) ? 7 : MacTxDataRate;
    TxDataRateToSfBw ( MacTxDataRate );
    DEBUG_PRINTF("  Tx data rate = %d\n",MacTxDataRate );
}

/********************************************************************************/
/*           RegionDecreaseDataRate                                             */
/*    method to update Datarate in ADR Mode                                     */
/********************************************************************************/
void LoraRegionsEU::RegionDecreaseDataRate ( void ) {
    uint8_t ValidTemp = 0;//@notereview boolfjerrek
    while ( ( MacTxDataRateAdr > 0 ) && ( ValidTemp == 0 ) ) {
        MacTxDataRateAdr --;
        for ( int i = 0 ; i < NUMBER_OF_CHANNEL ; i ++ ) {
            if ( MacChannelIndexEnabled [i] == CHANNEL_ENABLED ) {
                if ( ( MacTxDataRateAdr <= MacMaxDataRateChannel [i] ) && ( MacTxDataRateAdr >= MacMinDataRateChannel [i] ) ) {
                    ValidTemp ++;
                }
            }
        }
    }
    /* if adr DR = 0 enable the default channel*/
    if ( ( MacTxDataRateAdr == 0 ) && ( ValidTemp == 0) ) {
        MacChannelIndexEnabled [0] = CHANNEL_ENABLED ;
        MacChannelIndexEnabled [1] = CHANNEL_ENABLED ;
        MacChannelIndexEnabled [2] = CHANNEL_ENABLED ;;
    }
//@notereview    join continuer flag 
}


/********************************************************************************/
/*           RegionGiveNextChannel                                              */
/*    method to set the next enable channel                                     */
/********************************************************************************/
void  LoraRegionsEU::RegionGiveNextChannel( void ) {
    uint8_t NbOfActiveChannel = 0 ;
    for (int i = 0 ; i < NUMBER_OF_CHANNEL ; i ++ ) {
        if ( MacChannelIndexEnabled [i] == CHANNEL_ENABLED ) { 
            NbOfActiveChannel++;
        }
    }
    uint8_t temp = randr ( 0, ( NbOfActiveChannel - 1 ) );
    int ChannelIndex = 0;
    ChannelIndex = FindEnabledChannel ( temp ); // @note datarate valid not yett tested
    if ( ChannelIndex == -1 ) {
        DEBUG_PRINTF ("INVALID CHANNEL  active channel = %d and random channel = %d \n",NbOfActiveChannel,temp);
    } else {
        MacTxFrequencyCurrent = MacTxFrequency[ChannelIndex];
        MacRx1FrequencyCurrent = MacRx1Frequency[ChannelIndex];
    }

};

uint8_t  LoraRegionsEU::RegionGetAdrAckLimit( void ) {
    return ( ADR_ACK_LIMIT );
}
uint8_t  LoraRegionsEU::RegionGetAdrAckDelay( void ) {
    return ( ADR_ACK_DELAY );
}
/***********************************************************************************************/
/*                      Private  Methods                                                        */
/***********************************************************************************************/
//@notereview function a commun
void LoraRegionsEU :: TxDataRateToSfBw ( uint8_t dataRate ) {
    MacTxModulationCurrent = LORA ;
    if ( dataRate < 6 ){ 
        MacTxSfCurrent = 12 - dataRate ;
        MacTxBwCurrent = BW125 ;
    } else if ( dataRate == 6 ){ 
        MacTxSfCurrent = 7;
        MacTxBwCurrent = BW250 ;}
    else if ( dataRate == 7 ) {
        MacTxModulationCurrent = FSK ;
    } else {
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

