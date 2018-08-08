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
#include "EncoderMp.h"

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



