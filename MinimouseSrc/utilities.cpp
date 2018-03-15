/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Helper functions implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include <stdlib.h>
#include <stdio.h>

#include "utilities.h"
#include "LoraWanProcess.h"
/*!
 * Redefinition of rand() and srand() standard C functions.
 * These functions are redefined in order to get the same behavior across
 * different compiler toolchains implementations.
 */
// Standard random functions redefinition start
#define RAND_LOCAL_MAX 2147483647L

static uint32_t next = 1;

int32_t rand1( void )
{
    return ( ( next = next * 1103515245L + 12345L ) % RAND_LOCAL_MAX );
}

void srand1( uint32_t seed )
{
    next = seed;
}
// Standard random functions redefinition end

int32_t randr( int32_t min, int32_t max )
{
    return ( int32_t )rand1( ) % ( max - min + 1 ) + min;
}

void memcpy1( uint8_t *dst, const uint8_t *src, uint16_t size )
{
    while( size-- )
    {
        *dst++ = *src++;
    }
}

void memcpyr( uint8_t *dst, const uint8_t *src, uint16_t size )
{
    dst = dst + ( size - 1 );
    while( size-- )
    {
        *dst-- = *src++;
    }
}

void memset1( uint8_t *dst, uint8_t value, uint16_t size )
{
    while( size-- )
    {
        *dst++ = value;
    }
}

int8_t Nibble2HexChar( uint8_t a )
{
    if( a < 10 )
    {
        return '0' + a;
    }
    else if( a < 16 )
    {
        return 'A' + ( a - 10 );
    }
    else
    {
        return '?';
    }
}



#define POLY64REV     0x95AC9329AC4BC9B5
#define INITIALCRC    0xFFFFFFFFFFFFFFFF

void Crc64(uint8_t *dataIn, int size,uint32_t * crcLow, uint32_t * crcHigh )
{
    int i, j;
    uint64_t crc = INITIALCRC, part;
    static int init = 0;
    static  uint64_t  CRCTable[256];
    
    if (!init)
    {
        init = 1;
        for (i = 0; i < 256; i++)
        {
            part = i;
            for (j = 0; j < 8; j++)
            {
               if (part & 1)
                   part = (part >> 1) ^ POLY64REV;
               else
                   part >>= 1;
            }
            CRCTable[i] = part;
        }
    }
    
    for (i = 0 ; i < size ; i++)
    {
        crc = CRCTable[(crc ^ *dataIn++) & 0xff] ^ (crc >> 8);
    }
    /* 
    The output is done in two parts to avoid problems with 
    architecture-dependent word order
    */
    *crcLow = crc & 0xffffffff;
    *crcHigh = (crc >> 32) & 0xffffffff ; 
}


int  Certification ( bool NewCommand , uint8_t * UserFport , uint8_t * UserPayloadSize,  uint8_t * UserRxPayloadSize, uint8_t * MsgType, uint8_t * UserRxPayload, uint8_t * UserPayload, LoraWanObjet<LoraRegionsEU> *Lp){
    uint32_t temp ;
    static uint16_t FcntDwnCertif = 0;
    static uint32_t MsgTypePrevious = UNCONF_DATA_UP ;
    int i ;
    *UserFport       = 224;
    *UserPayloadSize = 2;
    *MsgType = UNCONF_DATA_UP ;
    if ( NewCommand == true) {
        switch ( UserRxPayload[0] ) {
            case 0 :  // end of test
                *UserFport       = 3;
                *UserPayloadSize = 14;
                for ( i = 0; i < 14 ; i ++) {
                    UserPayload[i]  = i;
                }
                break;
            case 1 :
                temp =  ( UserRxPayload[0] << 24 ) + ( UserRxPayload[1] << 16 ) + ( UserRxPayload[2] << 8 ) + ( UserRxPayload[3] );
                if ( temp == 0x01010101) {
                     Lp->SetDataRateStrategy( STATIC_ADR_MODE );
                     FcntDwnCertif   = 0;
                     UserPayload[0]  = FcntDwnCertif >> 8;
                     UserPayload[1]  = FcntDwnCertif & 0xFF;
                }
                break;            
            case 2 :  // Confirmed Uplink
                *MsgType = CONF_DATA_UP ; 
                MsgTypePrevious = *MsgType;
                UserPayload[0]  = FcntDwnCertif >> 8;
                UserPayload[1]  = FcntDwnCertif & 0xFF;
                break;
            case 3 :  // UnConfirmed Uplink
                *MsgType = UNCONF_DATA_UP ;
                MsgTypePrevious = *MsgType;   
                UserPayload[0]  = FcntDwnCertif >> 8;
                UserPayload[1]  = FcntDwnCertif & 0xFF;            
                break;
            case 4 :  //echo payload
                *UserPayloadSize = *UserRxPayloadSize;
                UserPayload[0] = 4;
                for ( i = 1 ; i < (*UserPayloadSize); i++ ) {
                    UserPayload[i]  = UserRxPayload [i] + 1;
                }
                break;
            case 5 :  // link check request 
              *UserPayloadSize = 1;
              UserPayload[0]  = 2;
              UserFport       = 0;
            case 6 :  // rejoin 
               Lp->NewJoin( );
               break;
            default :
                break;
        }
        FcntDwnCertif++;
    } else { // for the case of echo cmd
         *MsgType = MsgTypePrevious;
    }
    return ( UserRxPayload[0] );
}


