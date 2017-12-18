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
#include "ApiFlash.h"
Flash gFlash;
Flash::Flash(){
      // to be completed by mcu providers
}
Flash::~Flash(){
      // to be completed by mcu providers
} 

int Flash::init() {
    // to be completed by mcu providers
    return (0);
}
int Flash::RestoreContext(uint8_t *buffer, uint32_t addr, uint32_t size){
     /* have to be implemented by mcu providers
    the following code propose a lite implementation without any error cases*/
    int i;
    for (i=0;i<size;i++){
        buffer[i]= *((uint8_t*)(addr)+i);
    }
     return 0;
}



int Flash::StoreContext(const void *buffer, uint32_t addr, uint32_t size){
    /* have to be implemented by mcu providers
    the following code propose a lite implementation without any error cases
    this section have to be very robust, have to support failure mode such as  power off during flash programmation    
    This basic implementation suppose that the addr is 4 Bytes aligned and suppose also that the size can be divide by 4.
    */
    HAL_StatusTypeDef res = HAL_OK; 
    HAL_FLASH_Unlock(); 

    for (; size >= 4; size -= 4, addr += 4) { 
        res = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, *(uint32_t const*)buffer); 
        buffer = (uint32_t const*)buffer + 1; 
        if (res != HAL_OK) 
            goto done; 
    } 

    done: 
        HAL_FLASH_Lock(); 
        return res == HAL_OK ? 0 : -1; 
} 
   

int Flash::erase(uint32_t addr, uint32_t size){
     /* have to be implemented by mcu providers*/
    return 0;
}

uint32_t Flash::get_flash_start(){
     /* have to be implemented by mcu providers*/
     return 0;
}

uint32_t Flash:: get_flash_size(){
     /* have to be implemented by mcu providers*/
     return 0;
}    