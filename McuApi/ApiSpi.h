/*

  __  __ _       _                                 
 |  \/  (_)     (_)                                
 | \  / |_ _ __  _ _ __ ___   ___  _   _ ___  ___  
 | |\/| | | '_ \| | '_ ` _ \ / _ \| | | / __|/ _ \
 | |  | | | | | | | | | | | | (_) | |_| \__ \  __/ 
 |_|  |_|_|_| |_|_|_| |_| |_|\___/ \__,_|___/\___| 
                                                   
                                                   
Description       : Flash Api.  


License           : Revised BSD License, see LICENSE.TXT file include in the project

Maintainer        : Fabien Holin (SEMTECH)
*/
#ifndef APISPI_H
#define APISPI_H
#include "SPI.h"

class MMspi { 
public : 
    MMspi( PinName mosi, PinName miso, PinName sclk ):pspi(mosi,miso,sclk) {};
    ~MMspi(){};
    int write(int value){ return ( pspi.write( value ) ); };
    void format(int bits, int mode = 0){};
    void frequency(int hz = 1000000){};

private : 
    SPI pspi;
};

#endif
