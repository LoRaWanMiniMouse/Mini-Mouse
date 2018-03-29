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
#include "mbed.h"


#define DATA_EEPROM_BASE       ( ( uint32_t )0x807F800U )              /*!< DATA_EEPROM base address in the alias region */
#define DATA_EEPROM_END        ( ( uint32_t )DATA_EEPROM_BASE + 4096 ) /*!< DATA EEPROM end address in the alias region */

void FlashPageErase( uint32_t page, uint32_t banks )
{
    // Check the parameters
    assert_param( IS_FLASH_PAGE( page ) );
    assert_param( IS_FLASH_BANK_EXCLUSIVE( banks ) );

    if( ( banks & FLASH_BANK_1 ) != RESET )
    {
        CLEAR_BIT( FLASH->CR, FLASH_CR_BKER );
    }
    else
    {
        SET_BIT( FLASH->CR, FLASH_CR_BKER );
    }

    // Proceed to erase the page
    MODIFY_REG( FLASH->CR, FLASH_CR_PNB, ( page << 3 ) );
    SET_BIT( FLASH->CR, FLASH_CR_PER );
    SET_BIT( FLASH->CR, FLASH_CR_STRT );
}

uint8_t EepromMcuWriteBuffer( uint32_t addr, uint8_t *buffer, uint16_t size )
{
    uint64_t *flash = ( uint64_t* )buffer;
    
    //assert_param( addr >= DATA_EEPROM_BASE );
    assert_param( buffer != NULL );
    assert_param( size < ( DATA_EEPROM_END - DATA_EEPROM_BASE ) );

    HAL_FLASH_Unlock( );
    
    FlashPageErase( 255, 1 );
    
    WRITE_REG( FLASH->CR, 0x40000000 );

    for( uint32_t i = 0; i < size; i++ )
    {
        HAL_FLASH_Program( FLASH_TYPEPROGRAM_DOUBLEWORD, addr + ( 8 * i ), flash[i] );
    }

    HAL_FLASH_Lock( );

    return SUCCESS;
}

uint8_t EepromMcuReadBuffer( uint32_t addr, uint8_t *buffer, uint16_t size )
{
    assert_param( buffer != NULL );

    //assert_param( addr >= DATA_EEPROM_BASE );
    assert_param( buffer != NULL );
    assert_param( size < ( DATA_EEPROM_END - DATA_EEPROM_BASE ) );
for( uint32_t i = 0; i < size; i++ )
    {
     buffer[i]= *((( uint8_t* )addr)+i);
    }     
    return SUCCESS;
}

void EepromMcuSetDeviceAddr( uint8_t addr )
{
    assert_param( FAIL );
}

uint8_t EepromMcuGetDeviceAddr( void )
{
    assert_param( FAIL );
    return 0;
}





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
    uint16_t sizet = size & 0xFFFF;
    EepromMcuReadBuffer( addr, buffer, sizet );
    return ( 0 ); 
}



int Flash::StoreContext(const void *buffer, uint32_t addr, uint32_t size){
    /* have to be implemented by mcu providers
    the following code propose a lite implementation without any error cases
    this section have to be very robust, have to support failure mode such as  power off during flash programmation    
    This basic implementation suppose that the addr is 4 Bytes aligned and suppose also that the size can be divide by 4.
    */
    uint16_t sizet = size & 0xFFFF;    
    EepromMcuWriteBuffer( addr,  (uint8_t*) buffer, sizet );    
    return ( 0 ); 
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
