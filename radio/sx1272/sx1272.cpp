/*

  __  __ _       _                                 
 |  \/  (_)     (_)                                
 | \  / |_ _ __  _ _ __ ___   ___  _   _ ___  ___  
 | |\/| | | '_ \| | '_ ` _ \ / _ \| | | / __|/ _ \
 | |  | | | | | | | | | | | | (_) | |_| \__ \  __/ 
 |_|  |_|_|_| |_|_|_| |_| |_|\___/ \__,_|___/\___| 
                                                   
                                                   
Description       : LoraWan Phy Layer objets.  


License           : Revised BSD License, see LICENSE.TXT file include in the project

Maintainer        : Olivier Gimenez (SEMTECH)
*/

#include "sx1272.h"
#include "Define.h"
#include "UserDefine.h"
#include "stdint.h"
#include "sx1272Regs-Fsk.h"
#include "sx1272Regs-LoRa.h"



/************************************************************************************************
 *                                 Public  Methods                                              *
 ************************************************************************************************/


SX1272::SX1272( PinName nss, PinName reset, PinName TxRxIt, PinName RxTimeOutIt) : pinCS( nss ), pinReset( reset ){
    mcu.SetValueDigitalOutPin ( pinCS, 1);
    mcu.Init_Irq ( TxRxIt ) ;
    mcu.Init_Irq ( RxTimeOutIt ) ;
}

void SX1272::ClearIrqFlagsLora( void ) {
    Write ( REG_LR_IRQFLAGS, 0xFF);
}

void SX1272::FetchPayloadLora( uint8_t *payloadSize, uint8_t payload[255], int16_t *snr, int16_t *signalRssi) {
    *payloadSize = Read( REG_LR_RXNBBYTES );
    ReadFifo( payload, *payloadSize );
    GetPacketStatusLora( NULL, snr, signalRssi );
}

IrqFlags_t SX1272::GetIrqFlagsLora( void ) {
    uint8_t irqFlags = 0x00;
    irqFlags = Read(REG_LR_IRQFLAGS);
    if ( ( irqFlags & IRQ_LR_RX_TX_TIMEOUT ) !=0 ) {
        irqFlags = RXTIMEOUT_IRQ_FLAG;
        return (IrqFlags_t) irqFlags;
    }
    if ( ( irqFlags & IRQ_LR_RX_DONE ) !=0 ) {
        irqFlags = RECEIVE_PACKET_IRQ_FLAG;
        return (IrqFlags_t) irqFlags;
    }

    if ( ( irqFlags & IRQ_LR_TX_DONE ) !=0 ) {
        irqFlags = (SENT_PACKET_IRQ_FLAG);
        return (IrqFlags_t) irqFlags;
    }

    if ( ( irqFlags & IRQ_LR_CRC_ERROR ) != 0 ) {
        irqFlags = BAD_PACKET_IRQ_FLAG;
        return (IrqFlags_t) irqFlags;
    }
    return (IrqFlags_t) irqFlags;
}

void SX1272::Reset( void ) {
    mcu.SetValueDigitalOutPin ( pinReset, 1);
    mcu.waitUnderIt( 100 );
    mcu.SetValueDigitalOutPin ( pinReset, 0);
    mcu.waitUnderIt( 5000 );
    SetOpMode( RF_OPMODE_SLEEP );
}

void SX1272::SendLora( uint8_t *payload, uint8_t payloadSize,
                        uint8_t    SF,
                        eBandWidth BW,
                        uint32_t   channel,
                        int8_t     power
                    ) {
    Channel = channel;
    Reset( );
   // CalibrateImage( );
    SetOpMode( RF_OPMODE_SLEEP );
/* Set Lora Mode and max payload to 0x40 */
    Write( REG_OPMODE, ( Read( REG_OPMODE ) & RFLR_OPMODE_LONGRANGEMODE_MASK ) | RFLR_OPMODE_LONGRANGEMODE_ON );
/* Configure Lora Tx */
    SetStandby( );
    SetRfFrequency( channel );
    SetModulationParamsTx( SF, BW, power );
    SetPayload( payload, payloadSize);

/* Configure IRQ Tx Done */
    Write ( REG_LR_IRQFLAGSMASK, 0xF7 ); 
    Write ( REG_DIOMAPPING1, RFLR_DIOMAPPING1_DIO0_01 );
    Write ( REG_DIOMAPPING2, 0x00 );
/* Send */
    SetOpMode( RF_OPMODE_TRANSMITTER );
}

void SX1272::SendGen( uint8_t *payload, uint8_t payloadSize,
                        uint8_t    SF,
                        eBandWidth BW,
                        uint32_t   channel,
                        int8_t     power,
                        eIqMode    IqMode,
                        eCrcMode   CrcMode
                    ) {
    Channel = channel;
    Reset( );
   // CalibrateImage( );
    SetOpMode( RF_OPMODE_SLEEP );
/* Set Lora Mode and max payload to 0x40 */
    Write( REG_OPMODE, ( Read( REG_OPMODE ) & RFLR_OPMODE_LONGRANGEMODE_MASK ) | RFLR_OPMODE_LONGRANGEMODE_ON );
/* Configure Lora Tx */
    SetStandby( );
    SetRfFrequency( channel );
    SetModulationParamsTxGeneric( SF, BW, power, IqMode,  CrcMode);
    SetPayload( payload, payloadSize);

/* Configure IRQ Tx Done */
    Write ( REG_LR_IRQFLAGSMASK, 0xF7 ); 
    Write ( REG_DIOMAPPING1, RFLR_DIOMAPPING1_DIO0_01 );
    Write ( REG_DIOMAPPING2, 0x00 );
/* Send */
    SetOpMode( RF_OPMODE_TRANSMITTER );
}



// @TODO: SetRxBoosted ?
void SX1272::RxLora(eBandWidth BW, uint8_t SF, uint32_t channel, uint16_t TimeOutMs ) {
    Channel = channel;
/* Configure Lora Rx */
    SetOpMode( RF_OPMODE_SLEEP );
    SetStandby( );
    SetRfFrequency( channel );
    uint16_t symbTimeout = ( ( TimeOutMs & 0xFFFF ) * ( ( BW + 1 ) * 125 ) ) >> SF ;
    SetModulationParamsRx( SF, BW, symbTimeout);
    
/* Configure IRQ Rx Done or Rx timeout */
    Write ( REG_LR_IRQFLAGSMASK, 0x3F ); 
    Write( REG_DIOMAPPING1,0);
    Write ( REG_DIOMAPPING2, 0x00 );
/* Configure Fifo*/
    Write( REG_LR_FIFORXBASEADDR, 0 );
    Write( REG_LR_FIFOADDRPTR, 0 );
/* Receive */
    if ( TimeOutMs == 0 ) {
        SetOpMode( RFLR_OPMODE_RECEIVER );
    } else {
        SetOpMode( RFLR_OPMODE_RECEIVER_SINGLE );
    }
}



void SX1272::RxGen(eBandWidth BW, uint8_t SF, uint32_t channel, uint16_t TimeOutMs, eIqMode IqMode) {
    Channel = channel;
/* Configure Lora Rx */
    SetOpMode( RF_OPMODE_SLEEP );
    SetStandby( );
    SetRfFrequency( channel );
    uint16_t symbTimeout = ( ( TimeOutMs & 0xFFFF ) * ( ( BW + 1 ) * 125 ) ) >> SF ;
    SetModulationParamsRxGenric( SF, BW, symbTimeout, IqMode );
    
/* Configure IRQ Rx Done or Rx timeout */
    Write ( REG_LR_IRQFLAGSMASK, 0x3F ); 
    Write( REG_DIOMAPPING1,0);
    Write ( REG_DIOMAPPING2, 0x00 );
/* Configure Fifo*/
    Write( REG_LR_FIFORXBASEADDR, 0 );
    Write( REG_LR_FIFOADDRPTR, 0 );
/* Receive */
    if ( TimeOutMs == 0 ) {
        SetOpMode( RFLR_OPMODE_RECEIVER );
    } else {
        SetOpMode( RFLR_OPMODE_RECEIVER_SINGLE );
    }
}

void SX1272::Sleep(  bool coldStart ) {
    SetOpMode( RF_OPMODE_SLEEP );
}



/************************************************************************************************
 *                                Private  Methods                                              *
 ************************************************************************************************/
void SX1272::CalibrateImage( void )
{
    uint8_t regPaConfigInitVal;
    uint32_t initialFreq;

    // Save context
    regPaConfigInitVal = Read( REG_PACONFIG );
    initialFreq = ( double )( ( ( uint32_t )this->Read( REG_FRFMSB ) << 16 ) |
                              ( ( uint32_t )this->Read( REG_FRFMID ) << 8 ) |
                              ( ( uint32_t )this->Read( REG_FRFLSB ) ) ) * ( double )FREQ_STEP;

    // Cut the PA just in case, RFO output, power = -1 dBm
    this->Write( REG_PACONFIG, 0x00 );

    // Launch Rx chain calibration for LF band
    Write ( REG_IMAGECAL, ( Read( REG_IMAGECAL ) & RF_IMAGECAL_IMAGECAL_MASK ) | RF_IMAGECAL_IMAGECAL_START );
    while( ( Read( REG_IMAGECAL ) & RF_IMAGECAL_IMAGECAL_RUNNING ) == RF_IMAGECAL_IMAGECAL_RUNNING )
    {
    }

    // Sets a Frequency in HF band
    SetRfFrequency( 868000000 );

    // Launch Rx chain calibration for HF band 
    Write ( REG_IMAGECAL, ( Read( REG_IMAGECAL ) & RF_IMAGECAL_IMAGECAL_MASK ) | RF_IMAGECAL_IMAGECAL_START );
    while( ( Read( REG_IMAGECAL ) & RF_IMAGECAL_IMAGECAL_RUNNING ) == RF_IMAGECAL_IMAGECAL_RUNNING )
    {
    }

    // Restore context
    this->Write( REG_PACONFIG, regPaConfigInitVal );
    SetRfFrequency( initialFreq );
}


void SX1272::GetPacketStatusLora( int16_t *pktRssi, int16_t *snr, int16_t *signalRssi ) {
    
    *snr = ((int16_t) Read( REG_LR_PKTSNRVALUE ))/4; 
    int16_t rssi = (int16_t) Read( REG_LR_PKTRSSIVALUE );
    rssi += rssi / 16; 
    rssi = (Channel > RF_MID_BAND_THRESH ) ? RSSI_OFFSET_HF + rssi : RSSI_OFFSET_LF + rssi;
    *signalRssi = (*snr < 0 ) ? *snr + rssi : rssi;
}





void SX1272::SetModulationParamsTx( uint8_t SF, eBandWidth BW, int8_t power ) {
    
    uint8_t LowDatarateOptimize;
    uint8_t ValueTemp;
    power = ( power > 20 ) ? 20 : power;

    /*  Power  Setting  registers*/
    if ( PA_BOOST_CONNECTED == 1 ) {
        power = ( power > 20 ) ? 20 : power;
        power = ( power < 2 )  ? 2  : power;
        ValueTemp = ( power > 17 ) ? 0x87 : 0x84; // Enable/disable +20dbM option ON PA_BOOSt pin
        Write( REG_PADAC, ValueTemp );
        ValueTemp = ( power > 17 ) ? 0xf0 + ( power - 5 ) : 0xf0 + ( power - 2 ); 
        Write( REG_PACONFIG, ValueTemp );
    } else {
        power = ( power > 14 ) ? 14 : power;
        power = ( power < -1 ) ? -1 : power;
        //Write( REG_PADAC, 0x84 );
        Write( REG_PACONFIG, (  power + 1 ));
    }

    
     /* Set SF */
        ValueTemp =  ( SF << 4 ) ;
        Write( REG_LR_MODEMCONFIG2, ValueTemp ) ;
    
     /* Enable/disable Low datarate optimized */
        if( ( ( BW == 0 ) && ( ( SF == 11 ) || ( SF == 12 ) ) ) || ( ( BW == 1 ) && ( SF == 12 ) ) ) {
            LowDatarateOptimize = 0x01;
        } else {
            LowDatarateOptimize = 0x00;
        }
     /* Set Coding rate 4/5 , Explicite Header and BW */
        ValueTemp = (0x01<<3) + ( ( BW ) << 6 )  + 0x2 +LowDatarateOptimize; 
        Write( REG_LR_MODEMCONFIG1, ValueTemp );

     /* Set Preamble = 8 */
        Write( REG_LR_PREAMBLEMSB, 0 );
        Write( REG_LR_PREAMBLELSB, 8 );         
     /* Set Normal IQ */
        Write( REG_LR_INVERTIQ, 0x27) ;
        Write( REG_LR_INVERTIQ2, 0x1D );
        
     /* Set Public sync word */
        Write( REG_LR_SYNCWORD, 0x34);
        Write( REG_LR_DETECTOPTIMIZE, ( Read( REG_LR_DETECTOPTIMIZE ) & RFLR_DETECTIONOPTIMIZE_MASK ) | RFLR_DETECTIONOPTIMIZE_SF7_TO_SF12 );
        Write( REG_LR_DETECTIONTHRESHOLD, RFLR_DETECTIONTHRESH_SF7_TO_SF12 );
}


void SX1272::SetModulationParamsTxGeneric( uint8_t SF, eBandWidth BW, int8_t power , eIqMode IqMode, eCrcMode CrcMode ) {
    
    uint8_t LowDatarateOptimize;
    uint8_t ValueTemp;
    power = ( power > 20 ) ? 20 : power;

    /*  Power  Setting  registers*/
    if ( PA_BOOST_CONNECTED == 1 ) {
        power = ( power > 20 ) ? 20 : power;
        power = ( power < 2 )  ? 2  : power;
        ValueTemp = ( power > 17 ) ? 0x87 : 0x84; // Enable/disable +20dbM option ON PA_BOOSt pin
        Write( REG_PADAC, ValueTemp );
        ValueTemp = ( power > 17 ) ? 0xf0 + ( power - 5 ) : 0xf0 + ( power - 2 ); 
        Write( REG_PACONFIG, ValueTemp );
    } else {
        power = ( power > 14 ) ? 14 : power;
        power = ( power < -1 ) ? -1 : power;
        //Write( REG_PADAC, 0x84 );
        Write( REG_PACONFIG, (  power + 1 ));
    }

    
     /* Set SF */
        ValueTemp =  ( SF << 4 ) ;
        Write( REG_LR_MODEMCONFIG2, ValueTemp ) ;
    
     /* Enable/disable Low datarate optimized */
        if( ( ( BW == 0 ) && ( ( SF == 11 ) || ( SF == 12 ) ) ) || ( ( BW == 1 ) && ( SF == 12 ) ) ) {
            LowDatarateOptimize = 0x01;
        } else {
            LowDatarateOptimize = 0x00;
        }
     /* Set Coding rate 4/5 , Explicite Header and BW */
        ValueTemp = (0x01<<3) + ( ( BW ) << 6 )  + ( ( CrcMode == CRC_YES )? 2 : 0 ) + LowDatarateOptimize; 
        DEBUG_PRINTF ("Modem Config register 1 = %x ",ValueTemp);
        Write( REG_LR_MODEMCONFIG1, ValueTemp );

     /* Set Preamble = 8 */
        Write( REG_LR_PREAMBLEMSB, 0 );
        Write( REG_LR_PREAMBLELSB, 8 );         
     /* Set Normal IQ */
        if ( IqMode == IQ_NORMAL ) {
            Write( REG_LR_INVERTIQ, 0x27) ;
            Write( REG_LR_INVERTIQ2, 0x1D );
        } else {
        /* Set inverted IQ */
            Write( REG_LR_INVERTIQ, 0x67) ;
            Write( REG_LR_INVERTIQ2, 0x19) ;
        }
        
     /* Set Public sync word */
        Write( REG_LR_SYNCWORD, 0x34);
        Write( REG_LR_DETECTOPTIMIZE, ( Read( REG_LR_DETECTOPTIMIZE ) & RFLR_DETECTIONOPTIMIZE_MASK ) | RFLR_DETECTIONOPTIMIZE_SF7_TO_SF12 );
        Write( REG_LR_DETECTIONTHRESHOLD, RFLR_DETECTIONTHRESH_SF7_TO_SF12 );
}



void SX1272::SetModulationParamsRx( uint8_t SF, eBandWidth BW, uint16_t symbTimeout )
{
    uint8_t LowDatarateOptimize;
    uint8_t ValueTemp;
    /* Set SF */
    ValueTemp =  ( SF << 4 ) ;
    Write( REG_LR_MODEMCONFIG2, ValueTemp ) ;

    /* Enable/disable Low datarate optimized */
    if( ( ( BW == 0 ) && ( ( SF == 11 ) || ( SF == 12 ) ) ) || ( ( BW == 1 ) && ( SF == 12 ) ) ) {
        LowDatarateOptimize = 0x01;
    } else {
        LowDatarateOptimize = 0x00;
    }
    /* Set Coding rate 4/5 , Explicite Header and BW */
    ValueTemp = (0x01<<3) + ( ( BW ) << 6 )  + 0x2 +LowDatarateOptimize; 
    Write( REG_LR_MODEMCONFIG1, ValueTemp );
    /* Set LSB Timeout*/
    Write( REG_LR_SYMBTIMEOUTLSB, ( uint8_t )( symbTimeout & 0x00FF ) );
 /* Set Preamble = 8 */
    Write( REG_LR_PREAMBLEMSB, 0 );
    Write( REG_LR_PREAMBLELSB, 8 );
     
 /* Set inverted IQ */
    Write( REG_LR_INVERTIQ, 0x67) ;
    Write( REG_LR_INVERTIQ2, RFLR_INVERTIQ2_ON );
/* sensitivity optimization */
    Write( REG_LR_DETECTOPTIMIZE,( Read( REG_LR_DETECTOPTIMIZE ) & RFLR_DETECTIONOPTIMIZE_MASK ) | RFLR_DETECTIONOPTIMIZE_SF7_TO_SF12 );
    Write( REG_LR_DETECTIONTHRESHOLD, RFLR_DETECTIONTHRESH_SF7_TO_SF12 );
     /* Set Public sync word */
    Write( REG_LR_SYNCWORD, 0x34);
}

void SX1272::SetModulationParamsRxGenric( uint8_t SF, eBandWidth BW, uint16_t symbTimeout , eIqMode IqMode  ){
    uint8_t LowDatarateOptimize;
    uint8_t ValueTemp;
    /* Set SF */
    ValueTemp =  ( SF << 4 ) ;
    Write( REG_LR_MODEMCONFIG2, ValueTemp ) ;

    /* Enable/disable Low datarate optimized */
    if( ( ( BW == 0 ) && ( ( SF == 11 ) || ( SF == 12 ) ) ) || ( ( BW == 1 ) && ( SF == 12 ) ) ) {
        LowDatarateOptimize = 0x01;
    } else {
        LowDatarateOptimize = 0x00;
    }
    /* Set Coding rate 4/5 , Explicite Header and BW */
    ValueTemp = (0x01<<3) + ( ( BW ) << 6 )  + 0x2 +LowDatarateOptimize; 
    Write( REG_LR_MODEMCONFIG1, ValueTemp );
    /* Set LSB Timeout*/
    Write( REG_LR_SYMBTIMEOUTLSB, ( uint8_t )( symbTimeout & 0x00FF ) );
 /* Set Preamble = 8 */
    Write( REG_LR_PREAMBLEMSB, 0 );
    Write( REG_LR_PREAMBLELSB, 8 );
     
 /* Set inverted IQ */
     if ( IqMode == IQ_NORMAL ) {
            Write( REG_LR_INVERTIQ, 0x27) ;
            Write( REG_LR_INVERTIQ2, 0x1D );
    } else {
        /* Set inverted IQ */
        
            Write( REG_LR_INVERTIQ, 0x67) ;
            Write( REG_LR_INVERTIQ2, 0x19) ;
    }
        
/* sensitivity optimization */
    Write( REG_LR_DETECTOPTIMIZE,( Read( REG_LR_DETECTOPTIMIZE ) & RFLR_DETECTIONOPTIMIZE_MASK ) | RFLR_DETECTIONOPTIMIZE_SF7_TO_SF12 );
    Write( REG_LR_DETECTIONTHRESHOLD, RFLR_DETECTIONTHRESH_SF7_TO_SF12 );
     /* Set Public sync word */
    Write( REG_LR_SYNCWORD, 0x34);
}

void SX1272::SetPayload (uint8_t *payload, uint8_t payloadSize) {
// Initializes the payload size
    Write( REG_LR_PAYLOADLENGTH, payloadSize );

    // Full buffer used for Tx            
    Write( REG_LR_FIFOTXBASEADDR, 0 );
    Write( REG_LR_FIFOADDRPTR, 0 );

    // FIFO operations can not take place in Sleep mode
    if( ( Read( REG_OPMODE ) & ~RF_OPMODE_MASK ) == RF_OPMODE_SLEEP )
    {
        SetStandby( );
        mcu.mwait_ms( 1 );
    }
    // Write payload buffer
    WriteFifo( payload, payloadSize );
}


void SX1272::SetRfFrequency( uint32_t frequency ) {
    uint32_t initialFreqInt, initialFreqFrac;
    initialFreqInt = frequency / FREQ_STEP_8;
    initialFreqFrac = frequency - ( initialFreqInt * FREQ_STEP_8 );
    frequency = ( initialFreqInt << 8 ) + ( ( ( initialFreqFrac << 8 ) + ( FREQ_STEP_8 / 2 ) ) / FREQ_STEP_8 ); 
    Write( REG_FRFMSB, ( uint8_t )( ( frequency >> 16 ) & 0xFF ) );
    Write( REG_FRFMID, ( uint8_t )( ( frequency >> 8 ) & 0xFF ) );
    Write( REG_FRFLSB, ( uint8_t )( frequency & 0xFF ) );
}

void SX1272::SetStandby( void ) {
    Write( REG_OPMODE, ( Read( REG_OPMODE ) & RF_OPMODE_MASK ) | RF_OPMODE_STANDBY );
}


void SX1272::Write( uint8_t addr, uint8_t *buffer, uint8_t size )
{
    uint8_t i;
    mcu.SetValueDigitalOutPin ( pinCS,0);
    mcu.SpiWrite( addr | 0x80 );
    for( i = 0; i < size; i++ )
    {
        mcu.SpiWrite( buffer[i] );
    }
    mcu.SetValueDigitalOutPin ( pinCS, 1);
}

void SX1272::Read( uint8_t addr, uint8_t *buffer, uint8_t size )
{
    uint8_t i;
    mcu.SetValueDigitalOutPin ( pinCS, 0);
    mcu.SpiWrite( addr & 0x7F );
    for( i = 0; i < size; i++ )
    {
        buffer[i] = mcu.SpiWrite( 0 );
    }
    mcu.SetValueDigitalOutPin ( pinCS, 1);
}
void SX1272::Write( uint8_t addr, uint8_t data )
{
    Write( addr, &data, 1 );
}
void SX1272::WriteFifo( uint8_t *buffer, uint8_t size )
{
    Write( 0, buffer, size );
}

uint8_t SX1272::Read( uint8_t addr )
{
    uint8_t data;
    Read( addr, &data, 1 );
    return data;
}
void SX1272::ReadFifo( uint8_t *buffer, uint8_t size )
{
    Read( 0, buffer, size );
}

void SX1272::SetOpMode( uint8_t opMode )
{
    Write( REG_OPMODE, ( Read( REG_OPMODE ) & RF_OPMODE_MASK ) | opMode );
}
