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

#include "SX126x.h"
#include <string.h>
#include "Define.h"


/************************************************************************************************
 *                                 Public  Methods                                              *
 ************************************************************************************************/
const uint8_t SX126x::LoraSyncword[2] = {0x34, 0x44};

SX126x::SX126x( PinName Busy, PinName nss, PinName reset, PinName Interrupt ):

        pinBusy( Busy ),
        pinReset( reset ),
        pinCS( nss ) {
        mcu.SetValueDigitalOutPin ( pinCS, 1);
        mcu.Init_Irq ( Interrupt ) ;
}

void SX126x::ClearIrqFlags( void ) {
    ClearIrqStatus( IRQ_RADIO_ALL );
}

void SX126x::FetchPayload(
                        uint8_t *payloadSize,
                        uint8_t payload[255],
                        int16_t *snr,
                        int16_t *signalRssi
                    ) {

    uint8_t offset = 0;
    
    GetRxBufferStatus( payloadSize, &offset );
    ReadBuffer( offset, payload, *payloadSize );
    GetPacketStatusLora( NULL, snr, signalRssi );
                    }
IrqFlags_t SX126x::GetIrqFlags( void ) {
    uint8_t irqStatus[2];
    IrqFlags_t irqFlags = RADIO_IRQ_NONE;
    
    // Read IRQ status
    ReadCommand( GET_IRQ_STATUS, irqStatus, 2 );
    // Parse it
    if ( ( irqStatus[0] & ( IRQ_RX_TX_TIMEOUT >> 8 ) ) !=0 ) {
        irqFlags = (IrqFlags_t) (irqFlags | RXTIMEOUT_IRQ_FLAG);
    }
    if ( ( irqStatus[1] & IRQ_RX_DONE ) !=0 ) {
        irqFlags = (IrqFlags_t) (irqFlags | RECEIVE_PACKET_IRQ_FLAG);
    }
    /* Not used by the MAC for now
    if ( ( irqStatus[1] & IRQ_TX_DONE ) !=0 ) {
        irqFlags = (IrqFlags_t) (irqFlags | RADIO_IRQ_TX_DONE);
    }
    */
    if ( ( ( irqStatus[1] & IRQ_HEADER_ERROR ) != 0 ) ||
        ( ( irqStatus[1] & IRQ_CRC_ERROR ) != 0 ) ) {
        irqFlags = (IrqFlags_t) (irqFlags | BAD_PACKET_IRQ_FLAG);
    }
    
    return irqFlags;
}

void SX126x::Reset( void ) {
    // Reset radio
    mcu.SetValueDigitalOutPin ( pinReset, 0);
    //wait_us( 100 );
    mcu.mwait_ms( 1 );
    mcu.SetValueDigitalOutPin ( pinReset, 1);
    //wait_us( 200 );
    mcu.mwait_ms( 1 );
    radioMode = AWAKE;
}

void SX126x::SendCw(uint32_t frequency){
	  Reset();
		SetRegulatorMode( USE_DCDC );
    SetDio2AsRfSwitchCtrl( true );
    SetStandby( STDBY_XOSC );
		SetRfFrequency( frequency );
		SX126x::SetTxContinuousWave( );
}

void SX126x::SendLora(
                        uint8_t    *payload,
                        uint8_t    payloadSize,
                        uint8_t    SF,
                        eBandWidth BW,
                        uint32_t   channel,
                        int8_t     power
                    ) {
    // Init radio

    Reset( );
    SetRegulatorMode( USE_DCDC );
    SetDio2AsRfSwitchCtrl( true );

    SetStandby( STDBY_XOSC );
    // Configure the radio
    SetPacketType( LORA );
    //CalibrateImage( channel );
    SetRfFrequency( channel );
    SetModulationParams( LORA , SF, BW );
    SetPacketParams( LORA, payloadSize, IQ_STANDARD );
    SetTxParams( power );
    WriteRegisters( REG_LORA_SYNC_WORD_MSB, ( uint8_t * ) this->LoraSyncword, 2 );
    // Send the payload to the radio
    SetBufferBaseAddress( 0, 0 );
    WriteBuffer( 0, payload, payloadSize );

    ClearIrqStatus( IRQ_RADIO_ALL );
    // Configure IRQ
    SetDioIrqParams(
                        0xFFFF,
                        0xFFFF,
                        IRQ_RADIO_NONE,
                        IRQ_RADIO_NONE
                   );
    ClearIrqStatus( IRQ_RADIO_ALL );
    
    // Send ! No timeout here as it is already handled by the MAC
    SetTx( 0 );
}

// @TODO: SetRxBoosted ?
void SX126x::RxLora(
                        eBandWidth   BW,
                        uint8_t      SF,
                        uint32_t     channel,
                        uint32_t     rxTimeoutMs
                    ) {
    
    // Configure the radio
    SetPacketType( LORA );
    SetRfFrequency( channel );
    SetModulationParams( LORA , SF, BW );
    SetPacketParams( LORA, 0, IQ_INVERTED );
    StopTimerOnPreamble( true );
    WriteRegisters( REG_LORA_SYNC_WORD_MSB, ( uint8_t * ) this->LoraSyncword, 2 );
    // Configure IRQ
    SetDioIrqParams(
                        IRQ_RX_DONE | IRQ_CRC_ERROR | IRQ_RX_TX_TIMEOUT,
                        IRQ_RX_DONE | IRQ_CRC_ERROR | IRQ_RX_TX_TIMEOUT,
                        IRQ_RADIO_NONE,
                        IRQ_RADIO_NONE
                   );
    ClearIrqStatus( IRQ_RADIO_ALL );
    SetRx( rxTimeoutMs << 6 );
}

void SX126x::Sleep( bool coldStart ) {
    uint8_t mode;
    mode = (coldStart == true) ? 0x00 : 0x04;
    WriteCommand( SET_SLEEP, &mode, 1 );
    radioMode = SLEEP;
}

/************************************************************************************************
 *                                Private  Methods                                              *
 ************************************************************************************************/
void SX126x::CalibrateImage( uint32_t freq )
{
    uint8_t calFreq[2];
    
    if( freq > 900000000 ) {
        calFreq[0] = 0xE1;
        calFreq[1] = 0xE9;
    } else if( freq > 850000000 ) {
        calFreq[0] = 0xD7;
        calFreq[1] = 0xD8;
    } else if( freq > 770000000 ) {
        calFreq[0] = 0xC1;
        calFreq[1] = 0xC5;
    } else if( freq > 460000000 ) {
        calFreq[0] = 0x75;
        calFreq[1] = 0x81;
    } else if( freq > 425000000 ) {
        calFreq[0] = 0x6B;
        calFreq[1] = 0x6F;
    }
    WriteCommand( CALIBRATE_IMAGE, calFreq, 2 );
}

void SX126x::CheckDeviceReady( void ) {
    // @TODO: drive antenna switch
    if ( radioMode != SLEEP ) {
        waitOnBusy( );
    } else {
        // Busy is HIGH in sleep mode, wake-up the device
        mcu.SetValueDigitalOutPin ( pinCS, 0 );
        waitOnBusy( );
        mcu.SetValueDigitalOutPin ( pinCS, 1 );
        radioMode = AWAKE;
    }
}

void SX126x::GetPacketStatusLora( int16_t *pktRssi, int16_t *snr, int16_t *signalRssi ) {
    uint8_t pktStatus[3];
    
    // Read packet status
    ReadCommand( GET_PACKET_STATUS, pktStatus, 3 );
    *pktRssi = - (int)pktStatus[0] / 2;
    ( pktStatus[1] < 128 ) ? ( *snr = (int)pktStatus[1] / 4 ) : ( *snr = ( ( (int)pktStatus[1] - 256 ) /4 ) );
    *signalRssi = -(int)pktStatus[2] / 2;
}

void SX126x::GetRxBufferStatus( uint8_t *payloadSize, uint8_t *rxStartBufferPointer ) {
    uint8_t status[2];
    
    ReadCommand( GET_RX_BUFFER_STATUS, status, 2 );
    *payloadSize = status[0];
    *rxStartBufferPointer = status[1];
}

void SX126x::ReadBuffer( uint8_t offset, uint8_t *payload, uint8_t payloadSize ) {
    waitOnBusy( );
    
    mcu.SetValueDigitalOutPin ( pinCS, 0 );
    mcu.SpiWrite( READ_BUFFER );
    mcu.SpiWrite( offset );
    mcu.SpiWrite( 0 );
    for( uint8_t i = 0; i < payloadSize; i++ )
    {
        payload[i] = mcu.SpiWrite( 0 );
    }
    mcu.SetValueDigitalOutPin ( pinCS, 1 );
}

uint8_t SX126x::ReadRegister( uint16_t address ) {
    uint8_t buffer;
    ReadRegisters( address, &buffer, 1 );
    return buffer;
}

// @TODO: Consistant prototype size vs payloadSize...
void SX126x::ReadRegisters( uint16_t address, uint8_t *buffer, uint16_t size ) {
    // wait on low busy
    CheckDeviceReady( );
    
    // Read registers
    mcu.SetValueDigitalOutPin ( pinCS, 0 );
    mcu.SpiWrite( READ_REGISTER );
    mcu.SpiWrite( ( address & 0xFF00 ) >> 8 );
    mcu.SpiWrite( address & 0x00FF );
    mcu.SpiWrite( 0 );
    
    for( uint16_t i = 0; i < size; i++ ) {
        buffer[i] = mcu.SpiWrite( 0 );
    }
    
    mcu.SetValueDigitalOutPin ( pinCS, 1 );
}


void SX126x::SetDioIrqParams(
                uint16_t irqMask,
                uint16_t dio1Mask,
                uint16_t dio2Mask,
                uint16_t dio3Mask
            ) {
    uint8_t buf[8];

    buf[0] = ( uint8_t )( ( irqMask >> 8 ) & 0x00FF );
    buf[1] = ( uint8_t )( irqMask & 0x00FF );
    buf[2] = ( uint8_t )( ( dio1Mask >> 8 ) & 0x00FF );
    buf[3] = ( uint8_t )( dio1Mask & 0x00FF );
    buf[4] = ( uint8_t )( ( dio2Mask >> 8 ) & 0x00FF );
    buf[5] = ( uint8_t )( dio2Mask & 0x00FF );
    buf[6] = ( uint8_t )( ( dio3Mask >> 8 ) & 0x00FF );
    buf[7] = ( uint8_t )( dio3Mask & 0x00FF );
    WriteCommand( SET_DIO_IRQ_PARAMS, buf, 8 );

}

void SX126x::ClearIrqStatus( uint16_t irq ) {
    uint8_t buf[2];

    buf[0] = ( uint8_t )( ( ( uint16_t )irq >> 8 ) & 0x00FF );
    buf[1] = ( uint8_t )( ( uint16_t )irq & 0x00FF );
    WriteCommand( CLR_IRQ_STATUS, buf, 2 );
}

uint8_t SX126x::ReadCommand( OpCode_t command, uint8_t *buffer, uint16_t size ) {
    uint8_t status;
    waitOnBusy( );
    
    mcu.SetValueDigitalOutPin ( pinCS, 0 );
    // Send command
    mcu.SpiWrite( ( uint8_t ) command );
    status = mcu.SpiWrite( 0x00 );
    // Read data
    for( uint16_t i = 0; i < size; i++ ) {
         buffer[i] = mcu.SpiWrite( 0x00 );
    }
    mcu.SetValueDigitalOutPin ( pinCS, 1 );
    
    return status;
}

void SX126x::SetBufferBaseAddress( uint8_t txBaseAddress, uint8_t rxBaseAddress ) {
    uint8_t buf[2];

    buf[0] = txBaseAddress;
    buf[1] = rxBaseAddress;
    WriteCommand( SET_BUFFER_BASE_ADDRESS, buf, 2 );
}


void SX126x::SetModulationParams(
                    eModulationType modulation,
                    uint8_t    SF,
                    eBandWidth BW
                ) {
    uint8_t n;
    // uint32_t tempVal = 0;
    uint8_t buf[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    
    switch( modulation ) {
    case FSK:
        // TODO
        /*
        n = 8;
        tempVal = ( uint32_t )( 32 * ( ( double )XTAL_FREQ / ( double )modulationParams->Params.Gfsk.BitRate ) );
        buf[0] = ( tempVal >> 16 ) & 0xFF;
        buf[1] = ( tempVal >> 8 ) & 0xFF;
        buf[2] = tempVal & 0xFF;
        buf[3] = modulationParams->Params.Gfsk.ModulationShaping;
        buf[4] = modulationParams->Params.Gfsk.Bandwidth;
        tempVal = ( uint32_t )( ( double )modulationParams->Params.Gfsk.Fdev / ( double )FREQ_STEP );
        buf[5] = ( tempVal >> 16 ) & 0xFF;
        buf[6] = ( tempVal >> 8 ) & 0xFF;
        buf[7] = ( tempVal& 0xFF );
        SX126xWriteCommand( RADIO_SET_MODULATIONPARAMS, buf, n );
        */
        break;
    case LORA:
        n = 4;
        buf[0] = SF;
        switch ( BW ) {
            case BW125:
                buf[1] = 4;
                break;
            case BW250:
                buf[1] = 5;
                break;
            case BW500:
                buf[1] = 6;
                break;
            default:
                // @TODO: Default value or error code.
                break;
            
            }
        // @TODO - Need to be variable ?
        buf[2] = 0x01; // Coding rate = 4/5
        if( ( ( BW == BW125 ) && ( ( SF == 11 ) || ( SF == 12 ) ) ) ||
            ( ( BW == BW250 ) && ( SF == 12 ) ) ) {
            buf[3] = 0x01;
        } else {
            buf[3] = 0x00;
        }
        
        break;
    default:
        // TODO - Add error ?
        return;
    }
    
    WriteCommand( SET_MODULATION_PARAMS, buf, n );
}

void SX126x::SetPacketParams(
                eModulationType modulation,
                uint8_t payloadSize,
                InvertIQ_t invertIQ
            ) {
    uint8_t n;
    // uint8_t crcVal = 0;
    uint8_t buf[9] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    
    switch( modulation )
    {
        case FSK:
        // TODO
        /*
        if( packetParams->Params.Gfsk.CrcLength == RADIO_CRC_2_BYTES_IBM )
        {
            SX126xSetCrcSeed( CRC_IBM_SEED );
            SX126xSetCrcPolynomial( CRC_POLYNOMIAL_IBM );
            crcVal = RADIO_CRC_2_BYTES;
        }
        else if( packetParams->Params.Gfsk.CrcLength == RADIO_CRC_2_BYTES_CCIT )
        {
            SX126xSetCrcSeed( CRC_CCITT_SEED );
            SX126xSetCrcPolynomial( CRC_POLYNOMIAL_CCITT );
            crcVal = RADIO_CRC_2_BYTES_INV;
        }
        else
        {
            crcVal = packetParams->Params.Gfsk.CrcLength;
        }
        n = 9;
        buf[0] = ( packetParams->Params.Gfsk.PreambleLength >> 8 ) & 0xFF;
        buf[1] = packetParams->Params.Gfsk.PreambleLength;
        buf[2] = packetParams->Params.Gfsk.PreambleMinDetect;
        buf[3] = ( packetParams->Params.Gfsk.SyncWordLength ); // convert from byte to bit - /!\ <<3 ?
        buf[4] = packetParams->Params.Gfsk.AddrComp;
        buf[5] = packetParams->Params.Gfsk.HeaderType;
        buf[6] = packetParams->Params.Gfsk.PayloadLength;
        buf[7] = crcVal;
        buf[8] = packetParams->Params.Gfsk.DcFree;
        */
        break;
    case LORA:
        n = 6;
        // TODO - Need to be variable ?
        buf[0] = 0x00;  // Preamble of 0x08 symbols
        buf[1] = 0x08;
        buf[2] = 0x00;  // Explicit header (Variable packet length)
        buf[3] = payloadSize;
        buf[4] = 0x01;  // Uplink: CRC ON
        buf[5] = invertIQ;  // Uplink: Standard IQ
        break;
    default:
        // @TODO - Add error ?
        return;
    }
    WriteCommand( SET_PACKET_PARAMS, buf, n );
}

void SX126x::SetPacketType( eModulationType modulation ) {
    if ( modulation == LORA ) {
        uint8_t mod = 1;
    WriteCommand( SET_PACKET_TYPE, ( uint8_t* )&mod, 1 );
    }
}

void SX126x::SetPaConfig(
                    uint8_t paDutyCycle,
                    uint8_t hpMax,
                    uint8_t deviceSel,
                    uint8_t paLut
                ) {
    uint8_t buf[4];

    buf[0] = paDutyCycle;
    buf[1] = hpMax;
    buf[2] = deviceSel;
    buf[3] = paLut;
    WriteCommand( SET_PA_CONFIG, buf, 4 );
}


void SX126x::SetRfFrequency( uint32_t frequency ) {
    uint8_t buf[4];
    uint32_t freq = 0;
    
    // Set frequency
    freq = ( uint32_t )( ( double )frequency / ( double )RADIO_FREQ_STEP );
    buf[0] = ( uint8_t )( ( freq >> 24 ) & 0xFF );
    buf[1] = ( uint8_t )( ( freq >> 16 ) & 0xFF );
    buf[2] = ( uint8_t )( ( freq >> 8 ) & 0xFF );
    buf[3] = ( uint8_t )( freq & 0xFF );
    WriteCommand( SET_RF_FREQUENCY, buf, 4 );
}

void SX126x::SetStandby( StandbyModes_t standbyConfig ) {
    WriteCommand( SET_STANDBY, ( uint8_t* )&standbyConfig, 1 );
}

void SX126x::SetRegulatorMode( RadioRegulatorMode_t mode ) {
    WriteCommand( SET_REGULATORMODE, ( uint8_t* )&mode, 1 );
}

void SX126x::SetDio2AsRfSwitchCtrl( uint8_t enable ) {
    WriteCommand( SET_RFSWITCHMODE, &enable, 1 );
}

void SX126x::SetTxContinuousWave( void ) {
    WriteCommand( SET_TXCONTINUOUSWAVE, 0, 0 );
}

void SX126x::SetTxInfinitePreamble( void ) {
    WriteCommand( SET_TXCONTINUOUSPREAMBLE, 0, 0 );
}
        
void SX126x::SetRx( uint32_t timeout ) {
    uint8_t buf[3];
    
    buf[0] = ( uint8_t )( ( timeout >> 16 ) & 0xFF );
    buf[1] = ( uint8_t )( ( timeout >> 8 ) & 0xFF );
    buf[2] = ( uint8_t )( timeout & 0xFF );
    WriteCommand( SET_RX, buf, 3 );
}

void SX126x::SetTx( uint32_t timeout ) {
    uint8_t buf[3];
    
    buf[0] = ( uint8_t )( ( timeout >> 16 ) & 0xFF );
    buf[1] = ( uint8_t )( ( timeout >> 8 ) & 0xFF );
    buf[2] = ( uint8_t )( timeout & 0xFF );
    WriteCommand( SET_TX, buf, 3 );
}


void SX126x::SetTxParams( int8_t power ) {
    uint8_t buf[2];
    
    // TODO - SX1262
    if( power == 15 ) {
        SX126x::SetPaConfig( 0x06, 0x00, 0x01, 0x01 );
    } else {
        SX126x::SetPaConfig( 0x04, 0x00, 0x01, 0x01 );
    }
    if( power > 14 ) {
        power = 14;
    } else if( power < -3 ) {
        power = -3;
    }
    buf[0] = 0x18;
    WriteRegisters( REG_OCP, buf, 1 ); // current max is 80 mA for the whole device
   
    buf[0] = power;
    // TODO - Variable ?
    buf[1] = 0x04; // Ramp time : 200 µs
    WriteCommand( SET_TX_PARAMS, buf, 2 );
}

void SX126x::StopTimerOnPreamble( bool state ) {
    uint8_t temp;
    temp = ( state == true ) ? 0x01 : 0x00;
    WriteCommand( STOP_TIMER_ON_PREAMBLE, &temp, 1 );
}

void SX126x::waitOnBusy( void ) {
    while( mcu.GetValueDigitalInPin ( pinBusy )  == 1 ) { };
}

void SX126x::WriteBuffer( uint8_t offset, uint8_t *buffer, uint8_t size ) {
     // wait on low busy
    CheckDeviceReady( );
    
    // Write buffers
    mcu.SetValueDigitalOutPin ( pinCS, 0 );
    mcu.SpiWrite( WRITE_BUFFER );
    mcu.SpiWrite(  offset );
    for( uint16_t i = 0; i < size; i++ ) {
        mcu.SpiWrite( buffer[i] );
    }
    mcu.SetValueDigitalOutPin ( pinCS, 1);
}


void SX126x::WriteCommand( OpCode_t opCode, uint8_t *buffer, uint16_t size ) {
    // wait on low busy
    CheckDeviceReady( );
    
    // Send command
    mcu.SetValueDigitalOutPin ( pinCS, 0 );
    mcu.SpiWrite( (uint8_t) opCode );
    for( uint16_t i = 0; i < size; i++ ) {
        mcu.SpiWrite( buffer[i] );
    }
    mcu.SetValueDigitalOutPin ( pinCS, 1 );
}

void SX126x::WriteRegisters( uint16_t address, uint8_t *buffer, uint16_t size ) {
    // wait on low busy
    CheckDeviceReady( );
    
    // Write registers
    mcu.SetValueDigitalOutPin ( pinCS, 0 );
    mcu.SpiWrite( WRITE_REGISTER );
    mcu.SpiWrite( ( address & 0xFF00 ) >> 8 );
    mcu.SpiWrite( address & 0x00FF );
    
    for( uint16_t i = 0; i < size; i++ ) {
       mcu.SpiWrite( buffer[i] );
    }
    
    mcu.SetValueDigitalOutPin ( pinCS, 1 );
}

