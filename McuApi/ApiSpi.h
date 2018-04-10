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
    /** Create a SPI master connected to the specified pins
    *
    *  @param mosi SPI Master Out, Slave In pin
    *  @param miso SPI Master In, Slave Out pin
    *  @param sclk SPI Clock pin
    */
    MMspi( PinName mosi, PinName miso, PinName sclk ):pspi(mosi,miso,sclk) {};
    ~MMspi(){};
    /** Write to the SPI Slave and return the response
    *
    *  @param value Data to be sent to the SPI slave
    *
    *  @returns
    *    Response from the SPI slave
    */
    int write(int value){ return ( pspi.write( value ) ); };
    
    /** Configure the data transmission format
    *
    *  @param bits Number of bits per SPI frame (4 - 16)
    *  @param mode Clock polarity and phase mode (0 - 3)
    *
    * @code
    * mode | POL PHA
    * -----+--------
    *   0  |  0   0
    *   1  |  0   1
    *   2  |  1   0
    *   3  |  1   1
    * @endcode
    */
    void format(int bits, int mode = 0){};
        
    /** Set the spi bus clock frequency
    *
    *  @param hz SCLK frequency in hz (default = 1MHz)
    */
    void frequency(int hz = 1000000){};

private : 
    SPI pspi;
};

#endif
