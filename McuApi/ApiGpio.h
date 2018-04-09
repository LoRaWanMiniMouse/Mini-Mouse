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
    
    MMDigitalOut(PinName pin) : DigOutMbed(pin) {};
        
    MMDigitalOut& operator= (int value) {
        DigOutMbed = value;
        return *this;
    }
    
protected :
    DigitalOut DigOutMbed;
};



class MMDigitalIn {
public :
    
    MMDigitalIn(PinName pin) : DigInMbed(pin) {};
    operator int() {
        return ( DigInMbed );
    }
    
protected :
    DigitalIn DigInMbed;
};



class MMInterruptIn {
public:

    MMInterruptIn(PinName pin) : InterruptInMbed ( pin ) { };
    
    void rise(Callback<void()> func) {
        InterruptInMbed.rise(func);
    }
protected :
    InterruptIn InterruptInMbed;
};
#endif
