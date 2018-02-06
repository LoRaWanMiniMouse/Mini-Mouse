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
#ifndef APIFLASH_H
#define APIFLASH_H
#include "mbed.h"
#include "Define.h"
#include "ApiTimers.h"
#define USERFLASHADRESS 0x8080000
class Flash { 
public: 
    Flash(); 
    ~Flash(); 

    /** Initialize a flash IAP device 
     * 
     *  Should be called once per lifetime of the object. 
     *  @return 0 on success or a negative error code on failure 
     */ 
    int init(); 
 

    /** RestoreContext data from a flash device.  
     * 
     *  This method invokes memcpy - reads number of bytes from the address 
     * 
     *  @param buffer Buffer to write to 
     *  @param addr   Flash address to begin reading from 
     *  @param size   Size to read in bytes 
     *  @return       0 on success, negative error code on failure 
     */ 
    int RestoreContext(uint8_t *buffer, uint32_t addr, uint32_t size); 

 
    /** StoreContext data to flash
     *  To be safer this function have to implement a read/check data sequence after programation 
     *  
     * 
     *  @param buffer Buffer of data to be written 
     *  @param addr   Flash Address to begin writing to,
     *  @param size   Size to write in bytes,
     *  @return       0 on success, negative error code on failure 
     */ 
    int StoreContext(const void *buffer, uint32_t addr, uint32_t size); 

 
    /** Erase sectors 
     * 
     *  The state of an erased sector is undefined until it has been programmed 
     * 
     *  @param addr Address of a sector to begin erasing, 
     *  @param size Size to erase in bytes,
     *  @return     0 on success, negative error code on failure 
     */ 
    int erase(uint32_t addr, uint32_t size);  

 
    /** Get the flash start address  
     * 
     *  @return Flash start address  
     */ 
    uint32_t get_flash_start(); 

 
    /** Get the flash size 
     * 
     *  @return Flash size  
     */ 
    uint32_t get_flash_size(); 
}; 

extern Flash gFlash;
#endif
