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

#define FSK_DATARATE_LORAWAN_REG_VALUE     0x280  // XTAL_FREQ / 50000
#define FSK_FDEV_MSB_LORAWAN_REG_VALUE     RF_FDEVMSB_25000_HZ
#define FSK_FDEV_LSB_LORAWAN_REG_VALUE     RF_FDEVLSB_25000_HZ
#define FSK_PREAMBLE_MSB_LORAWAN_REG_VALUE 0x00
#define FSK_PREAMBLE_LSB_LORAWAN_REG_VALUE 0x05
#define FSK_SYNCWORD_LORAWAN_REG_VALUE     0xC194C1
#define FSK_MAX_MODEM_PAYLOAD              64
#define FSK_THRESHOLD_REFILL_LIMIT         0x0F
#define LORAWAN_MIN_PACKET_SIZE            9
#define FSK_FAKE_IRQ_THRESHOLD             2


/************************************************************************************************
 *                                 Public  Methods                                              *
 ************************************************************************************************/

SX1276::SX1276( PinName nss, PinName reset, PinName TxRxIt, PinName RxTimeOutIt) :
isFakeIrq(false), fakeIrqFlag(RADIO_IRQ_NONE), pinCS( nss ), pinReset( reset ), lastPacketRssi(0){
    mcu.SetValueDigitalOutPin ( pinCS, 1);
    mcu.Init_Irq ( TxRxIt ) ;
    mcu.Init_Irq ( RxTimeOutIt ) ;
    rxPayloadSize = 0;
}

void SX1276::ClearIrqFlagsLora( void ) {
    Write ( REG_LR_IRQFLAGS, 0xFF);
}

void SX1276::ClearIrqFlagsFsk( void ) {
    Write ( REG_IRQFLAGS1, 0xFF);
    Write ( REG_IRQFLAGS2, 0xFF);
}

void SX1276::FetchPayloadLora( uint8_t *payloadSize, uint8_t payload[255], int16_t *snr, int16_t *signalRssi) {
    *payloadSize = Read( REG_LR_RXNBBYTES );
    ReadFifo( payload, *payloadSize );
    GetPacketStatusLora( NULL, snr, signalRssi );
}

void SX1276::FetchPayloadFsk( uint8_t *payloadSize, uint8_t payload[255], int16_t *snr, int16_t *signalRssi) {
    *payloadSize = this->rxPayloadSize;
    memcpy(payload, this->rxBuffer, this->rxPayloadSize);
    *snr = 0;
    *signalRssi = this->lastPacketRssi;
}

IrqFlags_t SX1276::GetIrqFlagsLora( void ) {
    uint8_t irqFlags = 0x00;

    // Read IRQ status
    irqFlags = Read(REG_LR_IRQFLAGS);
    // Parse it
    if ( ( irqFlags & IRQ_LR_RX_TX_TIMEOUT ) !=0 ) {
        irqFlags = RXTIMEOUT_IRQ_FLAG;
    }
    if ( ( irqFlags & IRQ_LR_RX_DONE ) !=0 ) {
        irqFlags = RECEIVE_PACKET_IRQ_FLAG;
    }

    if ( ( irqFlags & IRQ_LR_TX_DONE ) !=0 ) {
        irqFlags = (IrqFlags_t) (irqFlags | SENT_PACKET_IRQ_FLAG);
		}

    if ( ( irqFlags & IRQ_LR_CRC_ERROR ) != 0 ) {
        irqFlags = BAD_PACKET_IRQ_FLAG;
    }
    return (IrqFlags_t) irqFlags;
}

IrqFlags_t SX1276::GetIrqFlagsFsk( void ) {
    uint8_t irq1 = 0x00;
    uint8_t irq2 = 0x00;
    uint16_t irqs = 0x0000;
    uint16_t flags = RADIO_IRQ_NONE;
    if(this->IsFakeIRQ()){
        return this->fakeIrqFlag;
    }

    Read(REG_IRQFLAGS1, &irq1, 1);
    Read(REG_IRQFLAGS2, &irq2, 1);
    irqs = irq1<<8 | irq2;
    if ( ( irqs & IRQ_FSK_TIMEOUT ) !=0 ) {
        flags |= RXTIMEOUT_IRQ_FLAG;
    }
    if ( ( irqs & IRQ_FSK_PAYLOAD_READY ) !=0 ) {
        flags |= RECEIVE_PACKET_IRQ_FLAG;
    }
    if ( ( irqs & IRQ_FSK_PACKET_SENT ) !=0 ) {
        flags |= SENT_PACKET_IRQ_FLAG;
    }
    /* This flag cannot be set in FSK based on IRQ registers parsing
        if ( ( irqFlags & IRQ_LR_CRC_ERROR ) != 0 ) {
        irqFlags |= BAD_PACKET_IRQ_FLAG;
    }*/
      return (IrqFlags_t) flags;
}

void SX1276::Reset( void ) {
    mcu.SetValueDigitalOutPin ( pinReset, 0);
    mcu.mwait_ms( 1 );
    mcu.SetValueDigitalOutPin ( pinReset, 1);
    mcu.mwait_ms( 6 );
    this->ResetFakeIrq();
    lastPacketRssi = 0;
}

void SX1276::SendLora( uint8_t *payload, uint8_t payloadSize,
                       uint8_t    SF,
                       eBandWidth BW,
                       uint32_t   channel,
                       int8_t     power
                    ) {
    Channel = channel;
    Reset( );
    CalibrateImage( );
    /* Set Lora Mode and max payload to 0x40 */
    SetOpModeLora( RFLR_OPMODE_ACCESSSHAREDREG_DISABLE, RFLR_OPMODE_LONGRANGEMODE_ON, RF_OPMODE_SLEEP );
    /* Configure Lora Tx */
    SetStandby( );
    SetRfFrequency( channel );
    SetPowerParamsTx( power );
    SetModulationParamsTxLora( SF, BW );
    SetPayload( payload, payloadSize);

    /* Configure IRQ Tx Done */
    Write ( REG_LR_IRQFLAGSMASK, 0xF7 );
    Write ( REG_DIOMAPPING1, RFLR_DIOMAPPING1_DIO0_01 );
    Write ( REG_DIOMAPPING2, 0x00 );
    /* Send */
    SetOpMode( RF_OPMODE_TRANSMITTER );
}

void SX1276::SendFsk( uint8_t *payload, uint8_t payloadSize,
                      uint32_t   channel,
                      int8_t     power
                    ) {
    uint8_t payloadChunkSize;
    uint8_t bytesAlreadyIFifo = 0;
    uint8_t remainingBytes = payloadSize;
    Channel = channel;
    Reset( );
    CalibrateImage( );

    /* Configure FSK Tx */
    this->Sleep( false );
    SetOpModeFsk( RF_OPMODE_MODULATIONTYPE_FSK, RFLR_OPMODE_FREQMODE_ACCESS_LF, RF_OPMODE_SLEEP );
    SetRfFrequency( channel );
    SetPowerParamsTx( power );
    SetModulationParamsTxFsk( );

    if( payloadSize < FSK_MAX_MODEM_PAYLOAD ) {
        WriteFifo( &payloadSize, 1);
        WriteFifo( payload, payloadSize);
        SetOpMode( RF_OPMODE_TRANSMITTER );
        return;
    }
    else {
        SetFifoThreshold(FSK_THRESHOLD_REFILL_LIMIT);
        payloadChunkSize = FSK_THRESHOLD_REFILL_LIMIT;
        WriteFifo( &payloadSize, 1);
        WriteFifo( payload + bytesAlreadyIFifo, FSK_MAX_MODEM_PAYLOAD - 1);
        bytesAlreadyIFifo += FSK_MAX_MODEM_PAYLOAD - 1;
        remainingBytes = payloadSize - bytesAlreadyIFifo;
        SetOpMode( RF_OPMODE_TRANSMITTER );
        while( remainingBytes > payloadChunkSize ) {
            while(IsFskFifoLevelReached()){}
            WriteFifo( payload + bytesAlreadyIFifo, payloadChunkSize);
            bytesAlreadyIFifo += payloadChunkSize;
            remainingBytes = payloadSize - bytesAlreadyIFifo;
        }
        while(IsFskFifoLevelReached()){}
        WriteFifo( payload + bytesAlreadyIFifo, remainingBytes);
        return;
    }
}

// @TODO: SetRxBoosted ?
void SX1276::RxLora(eBandWidth BW, uint8_t SF, uint32_t channel, uint16_t TimeOutMs ) {
    Channel = channel;
    /* Configure Lora Rx */
    Reset( );
    CalibrateImage( );
    SetOpMode( RF_OPMODE_SLEEP );
	/* Set Lora Mode and max payload to 0x40 */
    Write( REG_OPMODE, ( Read( REG_OPMODE ) & RFLR_OPMODE_LONGRANGEMODE_MASK ) | RFLR_OPMODE_LONGRANGEMODE_ON );
    SetStandby( );
    SetRfFrequency( channel );
    uint16_t symbTimeout = ( ( TimeOutMs & 0xFFFF ) * ( ( BW + 1 ) * 125 ) ) >> SF ;
	  if ( symbTimeout > 0x3FF ) {
			symbTimeout = 0x3FF ;
		}
    SetModulationParamsRxLora( SF, BW, symbTimeout);
    
/* Configure IRQ Rx Done or Rx timeout */
    Write ( REG_LR_IRQFLAGSMASK, 0x3F ); 
    Write ( REG_DIOMAPPING1,0);
    Write ( REG_DIOMAPPING2, 0x00 );
    /* Configure Fifo*/
    Write( REG_LR_FIFORXBASEADDR, 0 );
    Write( REG_LR_FIFOADDRPTR, 0 );
    /* Receive */
    SetOpMode( RFLR_OPMODE_RECEIVER_SINGLE );

}

void SX1276::RxFsk(uint32_t channel, uint16_t timeOutMs) {
    this->Reset();
    Channel = channel;
    rxPayloadSize = 0;
    uint8_t bytesReceived = 0;
    uint8_t remainingBytes = 0;
    uint8_t payloadChunkSize = FSK_THRESHOLD_REFILL_LIMIT;
    uint32_t timeoutExpectedMS = 0x00000000;
    bool preamble_detected = false;

    SetOpModeFsk( RF_OPMODE_MODULATIONTYPE_FSK, RFLR_OPMODE_FREQMODE_ACCESS_LF, RF_OPMODE_SLEEP );
    SetRfFrequency( channel );
    SetModulationParamsRxFsk( );

    SetFifoThreshold(LORAWAN_MIN_PACKET_SIZE - 1);
    SetOpMode( RF_OPMODE_RECEIVER );
    timeoutExpectedMS = mcu.RtcGetTimeMs() + timeOutMs;
    while(true) {
        if(mcu.RtcGetTimeMs() > timeoutExpectedMS) {
            if(this->HasDetectedPreamble() && !preamble_detected){
                // A timeout has been detected so a payload might be in the process of being received.
                // Allocate here the amount of time required to receive the 3 bytes of address and the
                // LORAWAN_MIN_PACKET_SIZE that should trigger the FifoLevelReached.
                timeoutExpectedMS += 2;
                DEBUG_MSG("   ...Timeout but preamble detected...\n");
                preamble_detected = true;
            }
            else{
                this->Sleep(false);
                this->SetAndGenerateFakeIRQ(RXTIMEOUT_IRQ_FLAG);
                return;
            }
        }
        mcu.mwait_ms(5);
        if(this->IsFskFifoLevelReached()){
            break;
        }
    }
    bytesReceived = LORAWAN_MIN_PACKET_SIZE - 1;   // -1 because the first one is the payload size, which is not included into the payload
    ReadFifo( &rxPayloadSize, 1 );
    ReadFifo( rxBuffer, bytesReceived );
    remainingBytes = rxPayloadSize - bytesReceived;

    if(rxPayloadSize > (FSK_MAX_MODEM_PAYLOAD - 1)){
        SetFifoThreshold(payloadChunkSize - 1);
        while(remainingBytes > payloadChunkSize) {
            while(!IsFskFifoLevelReached()) {
                mcu.mwait_ms(3);
            }
            ReadFifo( &rxBuffer[0] + bytesReceived, payloadChunkSize );
            bytesReceived += payloadChunkSize;
            remainingBytes = rxPayloadSize - bytesReceived;
        }
    }

    while(!IsPayloadReady()) {
        mcu.mwait_ms(2);
    }
    ReadFifo( &rxBuffer[0] + bytesReceived, remainingBytes );
    lastPacketRssi = this->GetCurrentRssi();
    this->Sleep(false);
    this->SetAndGenerateFakeIRQ(RECEIVE_PACKET_IRQ_FLAG);
}

void SX1276::Sleep(  bool coldStart ) {
    SetOpMode( RF_OPMODE_SLEEP );
}

/************************************************************************************************
 *                                Private  Methods                                              *
 ************************************************************************************************/
void SX1276::SetAndGenerateFakeIRQ(IrqFlags_t fakeIrqFlag ) {
    this->isFakeIrq = true;
    this->fakeIrqFlag = fakeIrqFlag;
    this->generateFakeIrq();
}

void SX1276::ResetFakeIrq(void) {
    this->isFakeIrq = false;
    this->fakeIrqFlag = RADIO_IRQ_NONE;
}

bool SX1276::IsFakeIRQ(void) {
    if(this->isFakeIrq == true ){
        this->isFakeIrq = false;
        return true;
    }
    return false;
}

void SX1276::generateFakeIrq(void) {
    Write( REG_DIOMAPPING1, RF_DIOMAPPING1_DIO0_10 | RF_DIOMAPPING1_DIO1_11 | RF_DIOMAPPING1_DIO2_10 | RF_DIOMAPPING1_DIO3_01 );
    SetFifoThreshold(FSK_FAKE_IRQ_THRESHOLD);
    WriteFifo(this->rxBuffer, FSK_FAKE_IRQ_THRESHOLD + 1);
    Write( REG_DIOMAPPING1, RF_DIOMAPPING1_DIO0_10 | RF_DIOMAPPING1_DIO1_00 | RF_DIOMAPPING1_DIO2_10 | RF_DIOMAPPING1_DIO3_01 );
}

void SX1276::CalibrateImage( void ) {
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

bool SX1276::IsFskFifoLevelReached() {
    uint8_t irqFlags = 0x00;
    Read(REG_IRQFLAGS2, &irqFlags, 1);
    return (irqFlags & 0x20);
}

bool SX1276::IsPayloadReady() {
    uint8_t irqFlags = 0x00;
    Read(REG_IRQFLAGS2, &irqFlags, 1);
    return (irqFlags & 0x04);
}

bool SX1276::IsFskFifoEmpty() {
    uint8_t irqFlags = 0x00;
    Read(REG_IRQFLAGS2, &irqFlags, 1);
    return (irqFlags & 0x40);
}

bool SX1276::HasTimeouted() {
    uint8_t irqFlags = 0x00;
    Read(REG_IRQFLAGS1, &irqFlags, 1);
    return (irqFlags & 0x04);
}

bool SX1276::HasDetectedPreamble() {
    uint8_t irqFlags = 0x00;
    Read(REG_IRQFLAGS1, &irqFlags, 1);
    return (irqFlags & 0x02);
}

void SX1276::GetPacketStatusLora( int16_t *pktRssi, int16_t *snr, int16_t *signalRssi ) {
    *snr = ((int16_t) Read( REG_LR_PKTSNRVALUE ))/4;
    int16_t rssi = (int16_t) Read( REG_LR_PKTRSSIVALUE );
    rssi += rssi / 16;
    rssi = (Channel > RF_MID_BAND_THRESH ) ? RSSI_OFFSET_HF + rssi : RSSI_OFFSET_LF + rssi;
    *signalRssi = (*snr < 0 ) ? *snr + rssi : rssi;
}

void SX1276::SetPowerParamsTx( int8_t power ) {
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
}

void SX1276::SetModulationParamsTxLora( uint8_t SF, eBandWidth BW ) {
    uint8_t LowDatarateOptimize;
    uint8_t ValueTemp;

    /* Set Coding rate 4/5 , Implicite Header and BW */
    ValueTemp = 0x02 + ( ( BW + 7 ) << 4 );
    Write( REG_LR_MODEMCONFIG1, ValueTemp );

    /* Set Enable CRC and SF */
    ValueTemp =  4 + ( SF << 4 );
    Write( REG_LR_MODEMCONFIG2, ValueTemp );

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

void SX1276::SetModulationParamsCommonFsk( ) {
    // Set Bitrate
    Write( REG_BITRATEMSB, ( uint8_t )( FSK_DATARATE_LORAWAN_REG_VALUE >> 8 ) );
    Write( REG_BITRATELSB, ( uint8_t )( FSK_DATARATE_LORAWAN_REG_VALUE & 0xFF ) );

    // Set Fdev
    Write( REG_FDEVMSB, FSK_FDEV_MSB_LORAWAN_REG_VALUE );
    Write( REG_FDEVLSB, FSK_FDEV_LSB_LORAWAN_REG_VALUE );

    // Set Preamble
    Write( REG_PREAMBLEMSB, FSK_PREAMBLE_MSB_LORAWAN_REG_VALUE );
    Write( REG_PREAMBLELSB, FSK_PREAMBLE_LSB_LORAWAN_REG_VALUE );

    // Set sync config
    Write( REG_SYNCCONFIG, RF_SYNCCONFIG_AUTORESTARTRXMODE_OFF | RF_SYNCCONFIG_PREAMBLEPOLARITY_AA |
                           RF_SYNCCONFIG_SYNC_ON | RF_SYNCCONFIG_SYNCSIZE_3
    );

    // Set Packet Config 1
    Write( REG_PACKETCONFIG1, RF_PACKETCONFIG1_PACKETFORMAT_VARIABLE | RF_PACKETCONFIG1_DCFREE_WHITENING |
                              RF_PACKETCONFIG1_CRC_ON | RF_PACKETCONFIG1_CRCAUTOCLEAR_OFF |
                              RF_PACKETCONFIG1_ADDRSFILTERING_OFF | RF_PACKETCONFIG1_CRCWHITENINGTYPE_CCITT
    );

      // Set Packet Config 2
    Write( REG_PACKETCONFIG2, RF_PACKETCONFIG2_DATAMODE_PACKET | RF_PACKETCONFIG2_IOHOME_OFF |
                              RF_PACKETCONFIG2_BEACON_OFF
    );

    // Set Sync Values
    Write( REG_SYNCVALUE1, (FSK_SYNCWORD_LORAWAN_REG_VALUE >> 16) & 0x0000FF );
    Write( REG_SYNCVALUE2, (FSK_SYNCWORD_LORAWAN_REG_VALUE >> 8) & 0x0000FF );
    Write( REG_SYNCVALUE3, FSK_SYNCWORD_LORAWAN_REG_VALUE & 0x0000FF );
}

void SX1276::SetFifoThreshold(uint8_t threshold){
    Write( REG_FIFOTHRESH, RF_FIFOTHRESH_TXSTARTCONDITION_FIFONOTEMPTY | (threshold & ~RF_FIFOTHRESH_FIFOTHRESHOLD_MASK));
}

void SX1276::SetModulationParamsTxFsk( ) {
    this->SetModulationParamsCommonFsk();
    Write( REG_FIFOTHRESH, RF_FIFOTHRESH_TXSTARTCONDITION_FIFONOTEMPTY | FSK_THRESHOLD_REFILL_LIMIT);
    Write( REG_DIOMAPPING1, RF_DIOMAPPING1_DIO0_00 | RF_DIOMAPPING1_DIO1_11 | RF_DIOMAPPING1_DIO2_01 );
    Write( REG_DIOMAPPING2, 0x00 );
}

void SX1276::SetModulationParamsRxFsk( void ) {
    this->SetModulationParamsCommonFsk();
    this->ConfigureRssi();
    Write( REG_DIOMAPPING1, RF_DIOMAPPING1_DIO0_10 | RF_DIOMAPPING1_DIO1_11 | RF_DIOMAPPING1_DIO2_10 | RF_DIOMAPPING1_DIO3_01 );
    Write( REG_DIOMAPPING2, 0x00 );
    Write( REG_RXCONFIG, RF_RXCONFIG_RESTARTRXONCOLLISION_OFF |
                         RF_RXCONFIG_AFCAUTO_ON | RF_RXCONFIG_AGCAUTO_ON | RF_RXCONFIG_RXTRIGER_PREAMBLEDETECT );
    Write( REG_RXBW, RF_RXBW_MANT_20 | RF_RXBW_EXP_3 );   // 50 kHz
    Write( REG_AFCBW, RF_RXBW_MANT_24 | RF_RXBW_EXP_2 );  // 83.3 kHz
    Write( REG_PREAMBLEDETECT, RF_PREAMBLEDETECT_DETECTOR_ON | RF_PREAMBLEDETECT_DETECTORSIZE_2 | RF_PREAMBLEDETECT_DETECTORTOL_10 );
    Write( REG_AFCFEI, RF_AFCFEI_AFCAUTOCLEAR_ON );
    Write( REG_LNA, RF_LNA_GAIN_G1 | RF_LNA_BOOST_ON );
    Write( REG_PAYLOADLENGTH, 0xFF );
    Write( REG_RXTIMEOUT1, 0x00 );
    Write( REG_RXTIMEOUT2, 0x00 );
    Write( REG_RXTIMEOUT3, 0x00 );
}

void SX1276::SetModulationParamsRxLora( uint8_t SF, eBandWidth BW, uint16_t symbTimeout ) {
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

    if( ( BW == 2 ) && ( Channel > RF_MID_BAND_THRESH ) ) {
        // ERRATA 2.1 - Sensitivity Optimization with a 500 kHz Bandwidth
        Write( REG_LR_TEST36, 0x02 );
        Write( REG_LR_TEST3A, 0x64 );
        // ERRATA 2.3 - Receiver Spurious Reception of a LoRa Signal
        Write( REG_LR_DETECTOPTIMIZE, Read( REG_LR_DETECTOPTIMIZE ) | 0x80 );
    }
    else if( BW == 2 ) {
        // ERRATA 2.1 - Sensitivity Optimization with a 500 kHz Bandwidth
        Write( REG_LR_TEST36, 0x02 );
        Write( REG_LR_TEST3A, 0x7F );
        // ERRATA 2.3 - Receiver Spurious Reception of a LoRa Signal
        Write( REG_LR_DETECTOPTIMIZE, Read( REG_LR_DETECTOPTIMIZE ) | 0x80 );
    }
    else {
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
    if( ( Read( REG_OPMODE ) & ~RF_OPMODE_MASK ) == RF_OPMODE_SLEEP ) {
        SetStandby( );
        mcu.mwait_ms( 1 );
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

void SX1276::Write( uint8_t addr, uint8_t *buffer, uint8_t size ) {
    uint8_t i;

    mcu.SetValueDigitalOutPin ( pinCS,0);
    mcu.SpiWrite( addr | 0x80 );
    for( i = 0; i < size; i++ )
    {
        mcu.SpiWrite( buffer[i] );
    }
    mcu.SetValueDigitalOutPin ( pinCS, 1);
}

void SX1276::Read( uint8_t addr, uint8_t *buffer, uint8_t size ) {
    uint8_t i;

    mcu.SetValueDigitalOutPin ( pinCS, 0);
    mcu.SpiWrite( addr & 0x7F );
    for( i = 0; i < size; i++ )
    {
        buffer[i] = mcu.SpiWrite( 0 );
    }
    mcu.SetValueDigitalOutPin ( pinCS, 1);
}

void SX1276::Write( uint8_t addr, uint8_t data ) {
    Write( addr, &data, 1 );
}

void SX1276::WriteFifo( uint8_t *buffer, uint8_t size ) {
    Write( 0, buffer, size );
}

uint8_t SX1276::Read( uint8_t addr ) {
    uint8_t data;
    Read( addr, &data, 1 );
    return data;
}

void SX1276::ReadFifo( uint8_t *buffer, uint8_t size ) {
    Read( 0, buffer, size );
}

void SX1276::SetOpModeLora( uint8_t accessSharedReg, uint8_t lowFrequencyModeOn, uint8_t opMode ) {
    Write( REG_OPMODE, RFLR_OPMODE_LONGRANGEMODE_ON | accessSharedReg | lowFrequencyModeOn | opMode );
}

void SX1276::SetOpModeFsk( uint8_t modulationType, uint8_t lowFrequencyModeOn, uint8_t opMode ) {
    Write( REG_OPMODE, RFLR_OPMODE_LONGRANGEMODE_OFF | modulationType | lowFrequencyModeOn | opMode );
}

void SX1276::SetOpMode( uint8_t opMode ) {
    Write( REG_OPMODE, ( Read( REG_OPMODE ) & RF_OPMODE_MASK ) | opMode );
}

void SX1276::ConfigureRssi(void){
    Write(REG_RSSICONFIG, RF_RSSICONFIG_OFFSET_P_00_DB | RF_RSSICONFIG_SMOOTHING_256);
}

int8_t SX1276::GetCurrentRssi(void){
    uint8_t regVal = 0x00;
    Read(REG_RSSIVALUE, &regVal, 1);
    return -(regVal >> 1);
}
