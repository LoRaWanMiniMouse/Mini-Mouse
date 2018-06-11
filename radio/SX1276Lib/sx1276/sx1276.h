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
#ifndef SX1276_H
#define SX1276_H
#include <stdint.h>
#include "Define.h"
#include "ApiMcu.h"
/*!
 * SX1276 definitions
 */
#define XTAL_FREQ                                   32000000
#define FREQ_STEP                                   61.03515625
#define FREQ_STEP_8                                 15625 /* FREQ_STEP<<8 */
#define RX_BUFFER_SIZE                              256

/*!
 * Constant values need to compute the RSSI value
 */
#define RSSI_OFFSET_LF                              -164.0
#define RSSI_OFFSET_HF                              -157.0
#define RF_MID_BAND_THRESH                          525000000


class SX1276  {
public:
    SX1276( PinName nss, PinName reset , PinName TxRxIt, PinName RxTimeOutIt);
    ~SX1276(){};
    void ClearIrqFlags( void );
    uint8_t GetIrqFlags( void );
    void FetchPayload( uint8_t *payloadSize, uint8_t payload[255], int16_t *snr, int16_t *signalRssi);
    void Reset( void );
    void SendLora( uint8_t *payload, uint8_t payloadSize, uint8_t SF, eBandWidth BW, uint32_t channel, int8_t power);
    void RxLora( eBandWidth BW, uint8_t SF, uint32_t channel, uint16_t TimeOutMs );
    void Sleep(  bool coldStart );

    uint32_t Channel;
private:


    /*!
    * \brief Calibrates the Image rejection depending of the frequency
    */
    void CalibrateImage( void );

    /*!
    * \brief Gets the last received packet status of a LoRa packet (RSSI, SNR)
    * @param [Out] RSSI average over last received packet
    * @param [Out] SNR estimation over last received packet
    * @param [Out] RSSI estimation of the LoRa signal
    */
    void GetPacketStatusLora( int16_t *pktRssi, int16_t *snr, int16_t *signalRssi );

    /*!
    * \brief Set the modulation parameters for Tx
    * @param [IN] Speading factor
    * @param [IN] Bandwith
    * @param [IN] Power
    */
    void SetModulationParamsTx( uint8_t SF, eBandWidth BW, int8_t power );
    /*!
    * \brief Set the modulation parameters for Rx
    * @param [IN] Speading factor
    * @param [IN] Bandwith
    * @param [IN] TimeOut : number of symbols
    */
    void SetModulationParamsRx( uint8_t SF, eBandWidth BW, uint16_t symbTimeout );
    
    /*!
    * \brief Set the RF frequency
    * @param [IN] Frequency [Hz]
    */
    void SetRfFrequency( uint32_t frequency );

    /*!
    * \brief Sets the radio in configuration mode
    * @param [IN]  mode          Standby mode to put the radio into
    */
    void SetStandby( void );

    /*!
    * \brief Sets the radio opmode for Lora operations
		* TODO
    */
    void SetOpModeLora( uint8_t accessSharedReg, uint8_t lowFrequencyModeOn, uint8_t opMode );

    /*!
    * \brief Sets the radio opmode for FSK operations
		* TODO
		* @param [IN]  modulationType      The modulation scheme to be used for FSK/OOK:
		* @param [IN]  lowFrequencyModeOn  Access Low Frequency mode registers. Powwible values 
		*                                    - RFLR_OPMODE_FREQMODE_ACCESS_LF
		*                                    - RFLR_OPMODE_FREQMODE_ACCESS_HF
    * @param [IN]  opMode              mode to put the radio into
    */
    void SetOpModeFsk( uint8_t modulationType, uint8_t lowFrequencyModeOn, uint8_t opMode );

		/*!
    * \brief Sets the radio opmode
    * @param [IN]  opMode        mode to put the radio into
		*/
		void SetOpMode( uint8_t opMode );

    /*!
    * \brief Write Payload inside the sx1276 fifo 
    * @param [in] *payload      Buffer of the data
    * @param [in] payloadSize   Size of the data
    */
    void SetPayload(uint8_t *payload, uint8_t payloadSize);

    /*!
    * \brief Read data from the buffer holding the payload in the radio
    * \param [out] payload       A pointer to a buffer holding the data from the radio
    * \param [in]  payloadSize   The number of byte to be read
    */
    void ReadFifo( uint8_t *buffer, uint8_t size );
    
    /*!
    * \brief  Read 1 byte from  radio registers
    * \param  [In]  address      Address of the byte to read
    * \return Read byte
    */
    uint8_t Read( uint8_t addr ) ;

    /*!
    * \brief  Read data from radio registers
    *
    * \param  [In]  address      Address of the first byte to read
    * \param  [Out] buffer       Buffer that holds read data
    * \param  [In]  size         The number of bytes to read
    */
    void Read( uint8_t addr, uint8_t *buffer, uint8_t size );
    
    /*!
    * \brief Write data to the buffer holding the payload in the radio
    *
    * \param [in]  buffer        The data to be written (the payload)
    * \param [in]  size          The number of bytes to be written
    */
    void WriteFifo( uint8_t *buffer, uint8_t size );


    /*!
    * \brief Write several  bytes  to the radio registers
    * @param [in] address      The address of the first byte to write in the radio
    * @param [in] value        The data to be written in radio's memory
    * @param [in] size         Size of the data
    */
    void Write( uint8_t addr, uint8_t *buffer, uint8_t size );

    /*!
    * \brief  Write 1 bytes  to the radio registers
    * \param  [In]  address      
    * \return Read byte
    */
    void Write( uint8_t addr, uint8_t data );
    PinName pinCS;
    PinName pinReset;
};
#endif

