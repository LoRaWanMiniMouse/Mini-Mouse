/*

  __  __ _       _                                 
 |  \/  ( _)     ( _)                                
 | \  / |_ _ __  _ _ __ ___   ___  _   _ ___  ___  
 | |\/| | | '_ \| | '_ ` _ \ / _ \| | | / __|/ _ \
 | |  | | | | | | | | | | | | ( _) | |_| \__ \  __/ 
 |_|  |_|_|_| |_|_|_| |_| |_|\___/ \__,_|___/\___| 
                                                   
                                                   
Description       : Minimouse Services Specific objets.  


License           : Revised BSD License, see LICENSE.TXT file include in the project

Maintainer        : Fabien Holin ( SEMTECH)
*/
#ifndef ENCODER_MP_H
#define ENCODER_MP_H
#include "LoraWanProcess.h"

/*!
 * \brief	Function to determine whether a frame is a fragmentation command or fragmentation content
 *
 * \param	[IN]  input variable to be tested 
 * \param	[OUT] return true if x is a power of two
 */
bool IsPowerOfTwo( unsigned int x);


/*!
 * \brief	Pseudo random number generator : prbs23
 * \param	[IN] x - the input of the prbs23 generator
 */
int FragmentationPrbs23( int x );

/*!
 * \brief	Function to calculate a certain row from the parity check matrix
 *
 * \param	[IN] i - the index of the row to be calculated
 * \param	[IN] M - the size of the row to be calculted, the number of uncoded fragments used in the scheme,matrixRow - pointer to the boolean array
 * \param	[OUT] void
 */
void FragmentationGetParityMatrixRow( int N, int M, uint8_t * matrixRow  );

/*!
 * \brief	Function to xor two line of data 
 * 
 * \param	[IN] dataL1 and dataL2
 * \paral [IN] size : number of Bytes in dataL1
 * \param	[OUT] xor(dataL1,dataL2) in dataL1
 */
void XorLineData( uint8_t* dataL1,uint8_t* dataL2, int size);
    
enum
{
    ENCODER_NOT_DEFINED,
    ENCODER_DEFINED
};
#define WL 10 //tbd dynamically
#define CR 2 //tbd dynamically
#define FIFO_SIZE     1000 // mean 1000 bytes  
class EncoderMp {
    public :
    EncoderMp ( void ) { 
        InitEncoder = ENCODER_NOT_DEFINED;
    }
    
    void SetEncoderParam ( const uint32_t WindowLength, const uint8_t PayloadSize, uint8_t CodingRate ) {
        memset( &DataFifo[0] , 0, WindowLength * PayloadSize);
        DataCnt = 0;
        RedundancyCnt = 0;
        DataSize = PayloadSize;
        WLength = WindowLength;
        NbOfFrameInFifo = 0;
        CodeRate = CodingRate;
        SendNoEncodedFrameCnt = 0;
        SendEncodedFrame = false;
        FirstOneInMatrix = 0;
    };
    void AddDataInFifo (const uint8_t * DataIn , uint8_t * SdataOut ) {
        if ( NbOfFrameInFifo < WLength ) {
            memcpy( &DataFifo [ (NbOfFrameInFifo * DataSize)], DataIn, DataSize );
            NbOfFrameInFifo++;
            DEBUG_PRINTF("number of frames inside the Fifo %d \n",NbOfFrameInFifo);
        } else { 
            memcpy( &DataFifo[0], &DataFifo [DataSize], DataSize*( WLength -1) );
            memcpy( &DataFifo[DataSize*( WLength -1)], DataIn, DataSize );
            DEBUG_PRINTF("number of frames inside the Fifo %d \n",NbOfFrameInFifo);
        }
        SendNoEncodedFrameCnt ++ ;
        SdataOut[0] = DataCnt;
        DataCnt++;
        DataCnt = DataCnt & 0x7F;
        memcpy(&SdataOut[1], DataIn, DataSize );
        if ( SendNoEncodedFrameCnt + RedundancyCnt % ( CodeRate ) == 0) {
            SendEncodedFrame = false;
        } else {
            SendEncodedFrame = true;
        }
    };
    void BuildRedundancyFrame ( uint8_t * SdataOut ) {
        uint8_t matrixRow [255];
        uint8_t tmp [255];
        memset( tmp, 0, 255 );
        uint8_t tmp2 [255];
        memset( tmp2, 0, 255 );
        FirstOneInMatrix = 0;
        FragmentationGetParityMatrixRow( RedundancyCnt, WLength, &matrixRow[0] );
        for (int i = 0; i < WLength; i ++) {
            if ( matrixRow[i] == 1) {
                FirstOneInMatrix = (FirstOneInMatrix==0) ? i : FirstOneInMatrix;
                GetLineInFifo ( tmp, i ) ;                 
                XorLineData( tmp2, tmp, DataSize);
            }
        }
        
        SdataOut[0] = RedundancyCnt + 128 ;
        RedundancyCnt ++ ;
        RedundancyCnt = RedundancyCnt & 0x7F;
        memcpy(&SdataOut[1], tmp2, DataSize );
    
   
    }

    uint8_t   InitEncoder;    
    bool      SendEncodedFrame;
    private :
        
    void GetLineInFifo ( uint8_t* dataOut, int LineNumber ) {
        memcpy ( dataOut, &DataFifo[ LineNumber * DataSize ],DataSize);
    }
    uint8_t   DataCnt;
    uint8_t   RedundancyCnt;
    uint8_t   DataFifo [ FIFO_SIZE ];
    uint8_t   DataSize;
    uint32_t  WLength;
    uint8_t   NbOfFrameInFifo;

    uint8_t   CodeRate;
    uint8_t   FirstOneInMatrix;
    uint8_t   SendNoEncodedFrameCnt;

};
#endif
