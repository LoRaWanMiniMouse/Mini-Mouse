/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2016 Semtech

Description: 	Firmware update over the air with LoRa proof of concept
				General functions for the (de)fragmentation algorithm

By: Paul Marcelis
*/
#include <math.h>
#include "Fragmentation.h"


#include <stdint.h>     /* C99 types */
#include <stdbool.h>    /* bool type */
#include <stdio.h>      /* printf fprintf sprintf fopen fputs */

/*!
 * \brief	Function to determine whether a frame is a fragmentation command or fragmentation content
 *
 * \param	[IN] fragmentFirstByte - content of the first byte of a fragmentation frame
 * \param	[OUT] whether the fragmentation frame is a command
 */
bool IsFragmentationCommand( unsigned char fragmentFirstByte )
{
    return( fragmentFirstByte >= 128 );
}

/*!
 * \brief	Function to calculate a certain row from the parity check matrix
 *
 * \param	[IN] i - the index of the row to be calculated
 * \param	[IN] M - the size of the row to be calculted, the number of uncoded fragments used in the scheme,matrixRow - pointer to the boolean array
 * \param	[OUT] void
 */
void FragmentationGetParityMatrixRow( int N, int M, bool * matrixRow  )
{
   
	  int i;
    int m;
	  int x;
	  int nbCoeff = 0;
	  int r;
	  if ( IsPowerOfTwo(M) )
		{
			m=1;
		}
		else
		{
			m=0;
		}
		x = 1 + ( 1001 * N );
	  for (i=0;i<M;i++)
		{
			matrixRow[i]=0;
		}
    while( nbCoeff < M / 2 )
    {
			  r = 1 << 16;
        while( r >= M )
        {
            x = FragmentationPrbs23( x );
            r = x % ( M + m );
        }
        matrixRow[r] = 1;
        nbCoeff += 1;
    }
}

/*!
 * \brief	Pseudo random number generator : prbs23
 * \param	[IN] x - the input of the prbs23 generator
 */
int FragmentationPrbs23( int x )
{
    int b0 = x & 1;
    int b1 = (x & 0x20) >> 5; 
    x = ( int ) floor( ( double ) x / 2 ) + (( b0 ^ b1 ) << 22);
    return x;
}
/*!
 * \brief	Function to determine whether a frame is a fragmentation command or fragmentation content
 *
 * \param	[IN]  input variable to be tested 
 * \param	[OUT] return true if x is a power of two
 */
bool IsPowerOfTwo( unsigned int x)
{
    int i;
	  int bi;
	  int sumBit=0;
	  for (i=0;i<32;i++)
	  {
			bi = 1 << i;
			sumBit+= ( x & bi) >> i;
    }
		if ( sumBit == 1 )
		{
			return(true);
		}
		else
		{
			return(false);
		}
}
