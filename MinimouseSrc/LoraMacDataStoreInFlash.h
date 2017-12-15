/*

  __  __ _       _                                 
 |  \/  (_)     (_)                                
 | \  / |_ _ __  _ _ __ ___   ___  _   _ ___  ___  
 | |\/| | | '_ \| | '_ ` _ \ / _ \| | | / __|/ _ \
 | |  | | | | | | | | | | | | (_) | |_| \__ \  __/ 
 |_|  |_|_|_| |_|_|_| |_| |_|\___/ \__,_|___/\___| 
                                                   
                                                   
Description       : Data store in flash.  


License           : Revised BSD License, see LICENSE.TXT file include in the project

Maintainer        : Fabien Holin (SEMTECH)
*/
#ifndef DATASTOREINFLASH_H
#define DATASTOREINFLASH_H

struct sBackUpFlash
{    /*******************************************/
    /*      Update by Link ADR command         */
    /*******************************************/
    uint8_t    MacTxSf;
    uint8_t    MacTxPower;
    uint16_t   MacChMask;
    uint8_t    MacNbRepUnconfirmedTx; 
    /********************************************/
    /*     Update by TxParamaSetupRequest       */
    /********************************************/
    uint32_t   MacRx2Frequency ; 
    uint8_t    MacRx2Sf;
    uint8_t    MacRx1SfOffset;
    /********************************************/
    /*     Update by NewChannelReq command      */
    /********************************************/
    uint32_t   MacTxFrequency[16];
    uint32_t   MacMinMaxSFChannel[16];
    uint8_t    NbOfActiveChannel;
    /********************************************/
    /*   Update by RXTimingSetupReq command     */
    /********************************************/
    int        MacRx1Delay;
    /********************************************/
    /*   Other Data To store                    */
    /********************************************/
    uint16_t   FcntUp;
    uint32_t   FcntDwn;
    uint32_t   DevAddr;
    uint8_t    nwkSKey[16];
    uint8_t    appSKey[16];
    uint8_t    JoinedStatus; 
    uint8_t    Reserved [1]; 
} ;
extern struct sBackUpFlash BackUpFlash;
#endif
