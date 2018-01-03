/*

  __  __ _       _                                 
 |  \/  (_)     (_)                                
 | \  / |_ _ __  _ _ __ ___   ___  _   _ ___  ___  
 | |\/| | | '_ \| | '_ ` _ \ / _ \| | | / __|/ _ \
 | |  | | | | | | | | | | | | (_) | |_| \__ \  __/ 
 |_|  |_|_|_| |_|_|_| |_| |_|\___/ \__,_|___/\___| 
                                                   
                                                   
Description       : Crypto Api.  


License           : Revised BSD License, see LICENSE.TXT file include in the project

Maintainer        : Fabien Holin (SEMTECH)
*/
#ifndef APICRYPTO_H
#define APICRYPTO_H
#include "mbed.h"
#include "LoraMacCrypto.h"

    /** Crypto_setKey Function 
    *  This function allows the application to provision session keys in an ABP devices.
    * 
    *  @param key    16 Bytes Key
    *  @param keyIndex   Flash address to begin reading from 
    */ 
    void Crypto_setKey( uint8_t key[16] , uint8_t keyIndex );








    /** Crypto_deriveMcKey Function 
    *  This function allows the application to provision a multicast group session keys from an encrypted McKey.
    * 
    *  @param encrypted_McKey    16 Bytes Key
    *  @param McGoupIndex        Integer in Range [0:3]
    */ 
    // @note Not Yet implemented will used in class C
    void Crypto_deriveMcKey( uint8_t encrypted_McKey [16] , uint8_t McGoupIndex) ;
#endif
