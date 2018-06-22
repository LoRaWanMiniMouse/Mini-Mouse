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
#ifndef __FRAGMENTATION_H
#define __FRAGMENTATION_H

#define FRAG_COMMAND_DATA_BLOCK_RECEPTION_ACKNOWLEDGE 0x01
#define FRAG_COMMAND_DATA_BLOCK_AUTHENTICATION_REQUEST 0x02
#define FRAG_COMMAND_DATA_BLOCK_AUTHENTICATION_ANSWER 0x03
#define FRAG_COMMAND_FRAGMENTATION_SESSION_STATUS_REQ 0x04
#define FRAG_COMMAND_FRAGMENTATION_SESSION_STATUS_ANS 0x05
#include <stdint.h>     /* C99 types */
#include <stdbool.h>    /* bool type */
#include <stdio.h>      /* printf fprintf sprintf fopen fputs */
bool IsFragmentationCommand( unsigned char );
void FragmentationGetParityMatrixRow( int N, int M, bool * matrixRow  );
int FragmentationPrbs23( int );
bool IsPowerOfTwo( unsigned int x);


#endif
