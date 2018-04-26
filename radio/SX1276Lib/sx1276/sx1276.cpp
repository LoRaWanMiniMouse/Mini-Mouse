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

#include "sx1276.h"
#include "Define.h"
#include "UserDefine.h"
#include "stdint.h"
#include "sx1276Regs-Fsk.h"
#include "sx1276Regs-LoRa.h"



/************************************************************************************************
 *                                 Public  Methods                                              *
 ************************************************************************************************/


SX1276::SX1276( PinName nss, PinName reset) : pinCS( nss ), pinReset( reset ){
    mcu.SetValueDigitalOutPin ( pinCS, 1);
}

void SX1276::ClearIrqFlags( void ) {
    Write ( REG_LR_IRQFLAGS, 0xFF);
}

void SX1276::FetchPayload( uint8_t *payloadSize, uint8_t payload[255], int16_t *snr, int16_t *signalRssi) {
    *payloadSize = Read( REG_LR_RXNBBYTES );
    ReadFifo( payload, *payloadSize );
    GetPacketStatusLora( NULL, snr, signalRssi );
}

uint8_t SX1276::GetIrqFlags( void ) {

    return Read(REG_LR_IRQFLAGS);
}

void SX1276::Reset( void ) {
    mcu.SetValueDigitalOutPin ( pinReset, 0);
    wait_ms( 1 );
    mcu.SetValueDigitalOutPin ( pinReset, 1);
    wait_ms( 6 );
}

void SX1276::SendLora( uint8_t *payload, uint8_t payloadSize,
                        uint8_t    SF,
                        eBandWidth BW,
                        uint32_t   channel,
                        int8_t     power
                    ) {
    Reset( );
    CalibrateImage( );
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

// @TODO: SetRxBoosted ?
void SX1276::RxLora(eBandWidth BW, uint8_t SF, uint32_t channel, uint16_t TimeOutMs ) {
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
    SetOpMode( RFLR_OPMODE_RECEIVER_SINGLE );

}

void SX1276::Sleep(  bool coldStart ) {
        SetOpMode( RF_OPMODE_SLEEP );
}



/************************************************************************************************
 *                                Private  Methods                                              *
 ************************************************************************************************/
void SX1276::CalibrateImage( void )
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


void SX1276::GetPacketStatusLora( int16_t *pktRssi, int16_t *snr, int16_t *signalRssi ) {

}





void SX1276::SetModulationParamsTx( uint8_t SF, eBandWidth BW, int8_t power ) {
    
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
        Write( REG_PADAC, 0x84 );
        Write( REG_PACONFIG, ( 0x70 +  power - 1 ));
    }

     /* Set Coding rate 4/5 , Implicite Header and BW */
        ValueTemp = 0x02 + ( ( BW + 7 ) << 4 ); 
        Write( REG_LR_MODEMCONFIG1, ValueTemp );
    
     /* Set Enable CRC and SF */
        ValueTemp =  4 + ( SF << 4 ) ;
        Write( REG_LR_MODEMCONFIG2, ValueTemp ) ;
    
     /* Enable/disable Low datarate optimized */
        if( ( ( BW == 0 ) && ( ( SF == 11 ) || ( SF == 12 ) ) ) || ( ( BW == 1 ) && ( SF == 12 ) ) ) {
             LowDatarateOptimize = 0x08;
        } else {
             LowDatarateOptimize = 0x00;
        }
        Write( REG_LR_MODEMCONFIG3,LowDatarateOptimize + 4 ); // + 4 for internal AGC loop
         
     /* Set Preamble = 8 */
        Write( REG_LR_PREAMBLEMSB, 0 );
        Write( REG_LR_PREAMBLELSB, 8 );
         
     /* Set Normal IQ */
        Write( REG_LR_INVERTIQ, 0x27) ;
        Write( REG_LR_INVERTIQ2, RFLR_INVERTIQ2_OFF );
         
     /* Set Public sync word */
        Write( REG_LR_SYNCWORD, 0x34);
}

void SX1276::SetModulationParamsRx( uint8_t SF, eBandWidth BW, uint16_t symbTimeout )
{
    uint8_t LowDatarateOptimize;
    uint8_t ValueTemp;
 /* Set Coding rate 4/5 , Implicite Header and BW */
    ValueTemp = 0x02 + ( ( BW + 7 ) << 4 );
    Write( REG_LR_MODEMCONFIG1, ValueTemp );
  /* Set Enable CRC and SF + MSB Timeout*/
    ValueTemp = ( SF << 4 ) + 4 + ( ( symbTimeout >> 8 ) & 0x3 ) ;
    Write( REG_LR_MODEMCONFIG2, ValueTemp ) ;
  /* Set LSB Timeout*/
    Write( REG_LR_SYMBTIMEOUTLSB, ( uint8_t )( symbTimeout & 0x00FF ) );
    
 /* Enable/disable Low datarate optimized */
    if( ( ( BW == 0 ) && ( ( SF == 11 ) || ( SF == 12 ) ) ) || ( ( BW == 1 ) && ( SF == 12 ) ) ) {
         LowDatarateOptimize = 0x08;
    } else {
         LowDatarateOptimize = 0x00;
    }
    Write( REG_LR_MODEMCONFIG3,LowDatarateOptimize + 4 ); // + 4 for internal AGC loop
 /* Set Preamble = 8 */
    Write( REG_LR_PREAMBLEMSB, 0 );
    Write( REG_LR_PREAMBLELSB, 8 );
     
 /* Set inverted IQ */
    Write( REG_LR_INVERTIQ, 0x67) ;
    Write( REG_LR_INVERTIQ2, RFLR_INVERTIQ2_ON );
/* sensitivity optimization */
    
    Write( REG_LR_DETECTOPTIMIZE,( Read( REG_LR_DETECTOPTIMIZE ) & RFLR_DETECTIONOPTIMIZE_MASK ) | RFLR_DETECTIONOPTIMIZE_SF7_TO_SF12 );
    Write( REG_LR_DETECTIONTHRESHOLD, RFLR_DETECTIONTHRESH_SF7_TO_SF12 );

    if( ( BW == 2 ) && ( RF_MID_BAND_THRESH ) )
    {
        // ERRATA 2.1 - Sensitivity Optimization with a 500 kHz Bandwidth 
        Write( REG_LR_TEST36, 0x02 );
        Write( REG_LR_TEST3A, 0x64 );
        // ERRATA 2.3 - Receiver Spurious Reception of a LoRa Signal
        Write( REG_LR_DETECTOPTIMIZE, Read( REG_LR_DETECTOPTIMIZE ) | 0x80 );
    }
    else if( BW == 2 )
    {
        // ERRATA 2.1 - Sensitivity Optimization with a 500 kHz Bandwidth
        Write( REG_LR_TEST36, 0x02 );
        Write( REG_LR_TEST3A, 0x7F );
        // ERRATA 2.3 - Receiver Spurious Reception of a LoRa Signal
        Write( REG_LR_DETECTOPTIMIZE, Read( REG_LR_DETECTOPTIMIZE ) | 0x80 );
    }
    else
    {
        // ERRATA 2.1 - Sensitivity Optimization 
        Write( REG_LR_TEST36, 0x03 );
        // ERRATA 2.3 - Receiver Spurious Reception of a LoRa Signal
        Write( REG_LR_DETECTOPTIMIZE, Read( REG_LR_DETECTOPTIMIZE ) & 0x7F );
        Write( REG_LR_TEST30, 0x00 );
        Write( REG_LR_TEST2F, 0x40 );
    }
     /* Set Public sync word */
    Write( REG_LR_SYNCWORD, 0x34);
}



void SX1276::SetPayload (uint8_t *payload, uint8_t payloadSize) {
// Initializes the payload size
        Write( REG_LR_PAYLOADLENGTH, payloadSize );

        // Full buffer used for Tx            
        Write( REG_LR_FIFOTXBASEADDR, 0 );
        Write( REG_LR_FIFOADDRPTR, 0 );

        // FIFO operations can not take place in Sleep mode
        if( ( Read( REG_OPMODE ) & ~RF_OPMODE_MASK ) == RF_OPMODE_SLEEP )
        {
            SetStandby( );
            wait_ms( 1 );
        }
        // Write payload buffer
        WriteFifo( payload, payloadSize );
}


void SX1276::SetRfFrequency( uint32_t frequency ) {
    uint32_t initialFreqInt, initialFreqFrac;
    initialFreqInt = frequency / FREQ_STEP_8;
    initialFreqFrac = frequency - ( initialFreqInt * FREQ_STEP_8 );
    frequency = ( initialFreqInt << 8 ) + ( ( ( initialFreqFrac << 8 ) + ( FREQ_STEP_8 / 2 ) ) / FREQ_STEP_8 ); 
    Write( REG_FRFMSB, ( uint8_t )( ( frequency >> 16 ) & 0xFF ) );
    Write( REG_FRFMID, ( uint8_t )( ( frequency >> 8 ) & 0xFF ) );
    Write( REG_FRFLSB, ( uint8_t )( frequency & 0xFF ) );
}

void SX1276::SetStandby( void ) {
    Write( REG_OPMODE, ( Read( REG_OPMODE ) & RF_OPMODE_MASK ) | RF_OPMODE_STANDBY );
}


void SX1276::Write( uint8_t addr, uint8_t *buffer, uint8_t size )
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

void SX1276::Read( uint8_t addr, uint8_t *buffer, uint8_t size )
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
void SX1276::Write( uint8_t addr, uint8_t data )
{
    Write( addr, &data, 1 );
}
void SX1276::WriteFifo( uint8_t *buffer, uint8_t size )
{
    Write( 0, buffer, size );
}

uint8_t SX1276::Read( uint8_t addr )
{
    uint8_t data;
    Read( addr, &data, 1 );
    return data;
}
void SX1276::ReadFifo( uint8_t *buffer, uint8_t size )
{
    Read( 0, buffer, size );
}

void SX1276::SetOpMode( uint8_t opMode )
{
    Write( REG_OPMODE, ( Read( REG_OPMODE ) & RF_OPMODE_MASK ) | opMode );
}
