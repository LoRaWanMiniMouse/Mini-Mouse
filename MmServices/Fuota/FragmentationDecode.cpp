/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2017 Semtech

Description: 	Firmware update over the air with LoRa proof of concept
				Functions for the decoding
*/
#include "Fragmentation.h"
#include "FragmentationDecode.h"
//#include "FragmentationParameters.h"


#include <stdio.h>
#include "stdbool.h"
#include <string.h>
#include "ApiMcu.h"

//uint8_t FlashData[NBOFUTILFRAMEMAX][SIZEOFFRAMETRANSMITMAX];
FotaParameter sFotaParameter;
static bool  s[REDUNDANCYMAX];
                static int m2l = 0;
static int first = 0;
/*!
 * \brief	Function to store data in flash
 *
 * \param	[IN] raw data size(1,size) 
 * \param [IN] index : row index
 * \param	[OUT] void
 */
void StoreRowInFlash( uint8_t* rowData, int index, int bank ) 
{
	//EepromMcuWriteBuffer( sFotaParameter.DataSize*index, rowData, sFotaParameter.DataSize, 2 );
	//memcpy (FlashData[index],rowData,sFotaParameter.DataSize); // index-1 because index is given by fcnt with a min equal to 0
	//EepromMcuWriteBuffer( 0x8080000+sFotaParameter.DataSize*index, rowData, sFotaParameter.DataSize);
	mcu.WriteFlashWithoutErase(rowData,0x8080000+sFotaParameter.DataSize*index, sFotaParameter.DataSize);
	uint8_t temrowData[255];
	mcu.RestoreContext(temrowData,0x8080000+sFotaParameter.DataSize*index, sFotaParameter.DataSize);
	for (int i = 0 ; i < sFotaParameter.DataSize; i++){
		if (temrowData[i] == rowData[i]) {
		} else {
						while (1) {DEBUG_MSG("ERROR HAL FLASH \n");
						}
		}
	}
	
}
/*!
 * \brief	Function to catch the fcnt of the  receive frame
 *
 * \param	[IN] raw data (length = SIZEOFFRAMETRANSMIT)
 * \param	[OUT] frame counter
 */
int CatchFrameCounter( uint8_t* rowData ) 
{
	return (rowData[1] + rowData[0] * 256);
}
/*!
 * \brief	Function to remove lorawan 9 first Bytes
 *
 * \param	[IN] raw data , size of raw data
 * \param	[OUT] void
 */
void RemoveLoraExtraByte ( uint8_t* rowData, int size)
{
    int i;
	  for (i = 0; i < size-2 ;i ++)
	  {
			rowData[i]=rowData[i+2];
		}
}	

/*!
 * \brief	Function init FotaParameter
 *        
* \param	[IN]  M : Number of util frame
* \param	[IN]  Redundancy : number of extra encoded frame 
 * \param	[OUT] FotaParameter sturcture
 */

void FotaParameterInit( int M, int Redundancy, int DataSize ) 
{ 
	int i;
    sFotaParameter.M = M; // NbOfUtilFrames=SIZEOFFRAMETRANSMIT;
    sFotaParameter.Redundancy = Redundancy; // nbr of extra frame 
	sFotaParameter.DataSize = DataSize - EXTRABYTELORAWAN; // number of byte on a row
	sFotaParameter.LastReceiveFrameCnt = 0;
    sFotaParameter.NumberOfLoosingFrame = 0;
     m2l = 0;
    first = 0;
    for (i = 0; i < NBOFUTILFRAMEMAX; i++){
      sFotaParameter.MissingFrameIndex[i] = 1;
	}
		
	for (i = 0; i < REDUNDANCYMAX; i++){
		s[i]=0;
	}
	for (i = 0; i < REDUNDANCYMAXB*REDUNDANCYMAX; i++){
		sFotaParameter.MatrixM2B[i] = 0;
	}
	printf("decodage init NBOF FRAME%d  SIZE %d  REDUn %d \n",sFotaParameter.M, sFotaParameter.DataSize,sFotaParameter.Redundancy);
}

/*!
 * \brief	Function to find missing receive frame 
 *      
 * \param	[IN] current fram counter
 * \param	[OUT] void sFotaParameter is updated inside the function
 */
void FindMissingReceiveFrame( int  frameCounter )
{
	  int q;
	 
	  for (q = sFotaParameter.LastReceiveFrameCnt; q < (frameCounter-1); q++)
		{   
			  if (q < sFotaParameter.M)
				{	
			    sFotaParameter.NumberOfLoosingFrame++;
		                  printf("LOOSING FRAME , NumberOfLoosingFrame = %d\n",sFotaParameter.NumberOfLoosingFrame);
				  sFotaParameter.MissingFrameIndex[q] = sFotaParameter.NumberOfLoosingFrame;
				}
		}
		if (q < sFotaParameter.M)
		{	
			sFotaParameter.LastReceiveFrameCnt=frameCounter;
		}
		else
		{
			sFotaParameter.LastReceiveFrameCnt = sFotaParameter.M + 1;
		}
		
}	

/*!
 * \brief	Function to xor two line of data 
 * 
 * \param	[IN] dataL1 and dataL2
 * \paral [IN] size : number of Bytes in dataL1
 * \param	[OUT] xor(dataL1,dataL2) in dataL1
 */
void XorLineData( uint8_t* dataL1,uint8_t* dataL2, int size)
 {
    int i ;
	  uint8_t dataTemp[size];
	  for(i = 0 ;i < size; i++)
	  {
			dataTemp[i] = dataL1[i] ^ dataL2[i];
		}
		for(i = 0 ;i < size; i++)
	  {
		  dataL1[i] = dataTemp[i] ;
		}
 }

 
/*!
 * \brief	Function to xor two line of data 
 * 
 * \param	[IN] dataL1 and dataL2
 * \paral [IN] size : number of bool in dataL1
 * \param	[OUT] xor(dataL1,dataL2) store in dataL1
 */ 
void  XorLineBool( bool* dataL1,bool* dataL2, int size)
{
    int i ;
	  bool dataTemp[size];

	  for(i = 0 ;i < size; i++)
	  { 
			dataTemp[i] = dataL1[i] ^ dataL2[i];
		}
		for(i = 0 ;i < size; i++)
	  {
			dataL1[i] = dataTemp[i] ;
		}
 } 

/*!
 * \brief	Function to get row l in flash
 * 
 * \param	[IN] line number l
 * \param	[OUT] the row number l store in flash
 */
void GetRowInFlash(int l, uint8_t* rowData, int bank )
{
    //     memcpy(rowData,FlashData[l],sFotaParameter.DataSize);
    mcu.RestoreContext(rowData,0x8080000+sFotaParameter.DataSize*l, sFotaParameter.DataSize);
}

/*!
 * \brief	Function to find the index (fnct) of the x th missing frame 
 * 
 * \param	[IN] x th missing frame
 * \param	[OUT] the fnct associated to the x th missing frame
 */
int  FindMissingFrameIndex ( int x )
{
	  int i;
	  for (i = 0 ; i < sFotaParameter.M ; i++)
	  {
			if (sFotaParameter.MissingFrameIndex[i] == (x+1))
			{
				return(i);
			}
		}
		return(0);
}
/*!
 * \brief	Function to find the first one in a bolean vector
 * 
 * \param	[IN] bool vector and size of vector
 * \param	[OUT] the position of the first one in the row vector
 */

int FindFirstOne( bool* boolData, int size)
{
	  int i;
	  for (i = 0; i < size; i++)
	  {
			if ( boolData[i] == 1)
			{
				return i;
			}
		}
		return 0;
}
/*!
 * \brief	Function to test if a vector is null
 * 
 * \param	[IN] bool vector and size of vector
 * \param	[OUT] bool : true if vector is null
 */
bool VectorIsNull( bool* boolData, int size)
{
	  int i;
	  for (i = 0; i < size; i++)
	  {
			if ( boolData[i] == 1)
			{
				return false;
			}
		}
		return true;
}

/*!
 * \brief	Function extact a row from the binary matrix and expand to a bool vector
 * 
 * \param	[IN] row number
 * \param	[IN] bool vector, number of Bits in one row
 */
void ExtractLineFromBinaryMatrix( bool* boolVector, int rownumber, int numberOfBit)
{
	  int i;
	  int findByte ;     
		int findBitInByte;	
	  if ( rownumber > 0)
		{
			findByte =      ( rownumber * numberOfBit - ((rownumber * ( rownumber - 1)) / 2) ) / 8 ;
			findBitInByte	= ( rownumber * numberOfBit - ((rownumber * ( rownumber - 1)) / 2) ) % 8;  
		}
		else
		{
			findByte = 0;
			findBitInByte = 0;
		}
    if (rownumber>0) 
		{	
	   for (i = 0; i < rownumber; i++)
	   {
			boolVector[i] = 0 ; 
		 }
	  }
	  for (i = rownumber; i < numberOfBit; i++)
	  {
			 boolVector[i] =(sFotaParameter.MatrixM2B[findByte] >> ( 7 - findBitInByte)) & 0x01 ;
		   findBitInByte++;
		   if ( findBitInByte == 8)
			 {
					findBitInByte = 0;
					findByte++;
			 }
		}
}

/*!
 * \brief	Function Collapse and Push  a row vector to the binary matrix 
 * 
 * \param	[IN] row number
 * \param	[IN] bool vector, number of Bits in one row
 */
void PushLineToBinaryMatrix( bool* boolVector, int rownumber, int numberOfBit)
{
	  int i;
	  int findByte ;     
		int findBitInByte;	
	  if ( rownumber > 0)
		{
			findByte =      ( rownumber * numberOfBit - ((rownumber * ( rownumber - 1)) / 2) ) / 8 ;
			findBitInByte	= ( rownumber * numberOfBit - ((rownumber * ( rownumber - 1)) / 2) ) % 8;  
		}
		else
		{
			findByte = 0;
			findBitInByte = 0;
		}
	  for (i = rownumber; i < numberOfBit; i++)
	  { 
			if ( boolVector[i] == 1)
			{	
			 sFotaParameter.MatrixM2B[findByte] = (sFotaParameter.MatrixM2B[findByte] & (0xFF-(1 << ( 7 - findBitInByte)))) + (1 << ( 7 - findBitInByte));
			}
			else
			{	
			 sFotaParameter.MatrixM2B[findByte] = (sFotaParameter.MatrixM2B[findByte] & (0xFF-(1 << ( 7 - findBitInByte))));
			}	
			findBitInByte++;
      if ( findBitInByte == 8)
				{
					findBitInByte = 0;
					findByte++;
				}			
		}
}


/*!
 * \brief	Function to decode and reconstruct the binary file
 *        Called on each receive frame
* \param	[IN] raw data (length = SIZEOFFRAMETRANSMIT), bank : bank 1 or 2 in flash memory
 * \param	[OUT] frame counter
 */

int FragmentationDecodeCore( uint8_t* rowData, int bank ) 
{
	int frameCounter;
	int i;
  	int j;
    int l;
    int li; 
    int lj;
    int firstOneInRow;
    bool matrixRow[NBOFUTILFRAMEMAX] ;
    uint8_t matrixDataTemp[NBOFUTILFRAMEMAX] ;
//		static int m2l = 0;
  //  int first;
	  int noInfo = 0;
	  bool dataTempVector[REDUNDANCYMAX];
	  bool dataTempVector2[REDUNDANCYMAX];
    for (i =0; i < REDUNDANCYMAX; i++)
	  {
			dataTempVector[i]=0;
			dataTempVector2[i]=0;
		}
		for (i =0; i < NBOFUTILFRAMEMAX; i++)
	  {
			matrixDataTemp[i]=0;
			matrixRow[i]=0;
		}
		first = 0;
    frameCounter=CatchFrameCounter( rowData ); // find frame counter of the current receive frame
	  printf("Receive Frame With Fcnt = %d  %d  %d \n",frameCounter,rowData[0],rowData[1],rowData[2]);
    if ( frameCounter < sFotaParameter.LastReceiveFrameCnt)
    {
			return 0xFFFFFFFF;  // drop frame out of order
    }			
	  RemoveLoraExtraByte(rowData,sFotaParameter.DataSize + EXTRABYTELORAWAN );// 9 for extra byte lorawan
		/*The M first packet aren't encoded or in other words they are encoded with the unitary matrix*/
    if ( frameCounter < sFotaParameter.M+1)
		{			
			StoreRowInFlash(rowData, frameCounter-1,bank); // the M first frame are not encoded
			sFotaParameter.MissingFrameIndex[frameCounter-1] = 0;
			FindMissingReceiveFrame(frameCounter); // update the sFotaParameter.MissingFrameIndex with the loosing frame
			
		}
		else  // at this point we receive encoded frame and the number of loosing frame is well known : sFotaParameter.NumberOfLoosingFrame -1;
		{  
			printf("nb of loosing frames = %d \n",sFotaParameter.NumberOfLoosingFrame);
			 FindMissingReceiveFrame(frameCounter); //in case of the end of true data is missing
			if (sFotaParameter.NumberOfLoosingFrame == 0) // the case : all the M first row have been transmitted with no error
				 {
					 return(	sFotaParameter.NumberOfLoosingFrame );
				 }	
       FragmentationGetParityMatrixRow(frameCounter-sFotaParameter.M,sFotaParameter.M,matrixRow); //frameCounter-sFotaParameter.M
			 for (l = 0;l < ( sFotaParameter.M ); l++)
			 {
				 if (matrixRow[l] == 1)
				 {  
					 if (sFotaParameter.MissingFrameIndex[l] == 0) // xor with already receive frame
					 {
						  matrixRow[l]=0;
							GetRowInFlash(l,matrixDataTemp,bank);
							XorLineData( rowData , matrixDataTemp, sFotaParameter.DataSize);
					 }
					 else  // fill the "little" boolean matrix m2
					 {
						 dataTempVector[sFotaParameter.MissingFrameIndex[l]-1] =1;
						 if ( first == 0)
						 {
							 first = 1;
						 }
					 }
				 }
			 }
			 firstOneInRow=FindFirstOne(dataTempVector,sFotaParameter.NumberOfLoosingFrame);
			 if (first > 0)  //manage a new line in MatrixM2
			 {	
			   while (s[firstOneInRow]==1) // row already diagonalized exist&(sFotaParameter.MatrixM2[firstOneInRow][0])
			   { 
				   ExtractLineFromBinaryMatrix(dataTempVector2,firstOneInRow,sFotaParameter.NumberOfLoosingFrame);
					 XorLineBool( dataTempVector , dataTempVector2, sFotaParameter.NumberOfLoosingFrame );
				   li=FindMissingFrameIndex(firstOneInRow);  // have to store it in the mi th position of the missing frame
					 GetRowInFlash(li,matrixDataTemp,bank);
				   XorLineData( rowData , matrixDataTemp, sFotaParameter.DataSize);
				   if (VectorIsNull( dataTempVector,sFotaParameter.NumberOfLoosingFrame))
				   {
					   noInfo = 1;
					   break;
				   }
				   firstOneInRow=FindFirstOne(dataTempVector,sFotaParameter.NumberOfLoosingFrame);	
			   }
				 if ( noInfo == 0)
				 {
					 PushLineToBinaryMatrix( dataTempVector, firstOneInRow ,sFotaParameter.NumberOfLoosingFrame);
					 li=FindMissingFrameIndex(firstOneInRow);
					 StoreRowInFlash(rowData, li,bank);
					 s[firstOneInRow]=1; 
					 m2l++;
         }
				 				 
         if ( m2l == sFotaParameter.NumberOfLoosingFrame ) // then last step diagonalized
				 { 
					 printf("starting diag\n ");
					 if ( sFotaParameter.NumberOfLoosingFrame > 1 )
					 {
					  for( i = (sFotaParameter.NumberOfLoosingFrame-2); i >=0 ; i--)
					  { 
						  li=FindMissingFrameIndex(i);
						  GetRowInFlash(li,matrixDataTemp,bank);
						  for( j = (sFotaParameter.NumberOfLoosingFrame-1); j >i; j--)
							{
								  ExtractLineFromBinaryMatrix(dataTempVector2,i,sFotaParameter.NumberOfLoosingFrame);
									ExtractLineFromBinaryMatrix(dataTempVector,j,sFotaParameter.NumberOfLoosingFrame);
								  if (dataTempVector2[j] == 1)	
								  {	
									  XorLineBool( dataTempVector2 ,dataTempVector, sFotaParameter.NumberOfLoosingFrame );
									  PushLineToBinaryMatrix( dataTempVector2, i ,sFotaParameter.NumberOfLoosingFrame);
									 
								    lj=FindMissingFrameIndex(j);
									 
									  GetRowInFlash(lj,rowData,bank);
                    XorLineData( matrixDataTemp , rowData , sFotaParameter.DataSize);      
                  }		
							}
							 StoreRowInFlash(matrixDataTemp, li,bank);
						 }
					  return(	sFotaParameter.NumberOfLoosingFrame );
					 }
					 else //ifnot ( sFotaParameter.NumberOfLoosingFrame > 1 )
	         {
						 return(	sFotaParameter.NumberOfLoosingFrame ); 
					 }
				 }						 
		   }
    }
		return 0xFFFFFFFF;
}

