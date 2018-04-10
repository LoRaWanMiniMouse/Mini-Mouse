/*

  __  __ _       _                                 
 |  \/  (_)     (_)                                
 | \  / |_ _ __  _ _ __ ___   ___  _   _ ___  ___  
 | |\/| | | '_ \| | '_ ` _ \ / _ \| | | / __|/ _ \
 | |  | | | | | | | | | | | | (_) | |_| \__ \  __/ 
 |_|  |_|_|_| |_|_|_| |_| |_|\___/ \__,_|___/\___| 
                                                   
                                                   
Description       : Gpio Api.  


License           : Revised BSD License, see LICENSE.TXT file include in the project

Maintainer        : Fabien Holin (SEMTECH)
*/
#ifndef APIGPIO_H
#define APIGPIO_H


class MMDigitalOut {
public :
    /** Create a DigitalOut connected to the specified pin
    *
    *  @param pin DigitalOut pin to connect to
    */
    MMDigitalOut(PinName pin) : DigOutMbed(pin) {};
        
    /** A shorthand for write the Pin value
    */
    MMDigitalOut& operator= (int value) {
        DigOutMbed = value;
        return *this;
    }
    
protected :
    DigitalOut DigOutMbed;
};



class MMDigitalIn {
public :
    /** Create a DigitalIn connected to the specified pin
    *
    *  @param pin DigitalIn pin to connect to
    */
    MMDigitalIn(PinName pin) : DigInMbed(pin) {};
        
    /** An operator shorthand for read the pin value
    */
    operator int() {
        return ( DigInMbed );
    }
    
protected :
    DigitalIn DigInMbed;
};



class MMInterruptIn {
public:
    /** Create an InterruptIn connected to the specified pin
    *
    *  @param pin InterruptIn pin to connect to
    */
    MMInterruptIn(PinName pin) : InterruptInMbed ( pin ) { };
    
    /** Attach a function to call when a rising edge occurs on the input
    *
    *  @param func A pointer to a void function, or 0 to set as none
    */
    void rise(Callback<void()> func) {
        InterruptInMbed.rise(func);
    }
protected :
    InterruptIn InterruptInMbed;
};
#endif
