/*!
 * \file      appli.c
 *
 * \brief     Description : Example of usal application 
 *            This apllication uses an i2C sensor to get both Temperature and Hygro
 *            This application uses an analog input to get Vbat 
 *            This application transfers a float value for demo purpose
 *            This application send an APPLI_FW_VERSION as first byte of the payload
 *            This application used a part of the UID of the STM32 to set the DEVEUI 
 *            (It could be dangerous because deveui isn't necessary unique due to the fact that it is compute from a part of the uid and not the full uid)
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
  __  __ _       _                                 
 |  \/  (_)     (_)                                
 | \  / |_ _ __  _ _ __ ___   ___  _   _ ___  ___  
 | |\/| | | '_ \| | '_ ` _ \ / _ \| | | / __|/ _ \
 | |  | | | | | | | | | | | | (_) | |_| \__ \  __/ 
 |_|  |_|_|_| |_|_|_| |_| |_|\___/ \__,_|___/\___| 
                                                   
                                                   
 * \endcode

Maintainer        : Fabien Holin (SEMTECH)
*/

#include "mbed.h"
#include "appli.h"
#include "Define.h"
#include "LoraMacDataStoreInFlash.h"


payload ppayload;
int8_t TempMeas (void)
{
    wait_ms(100);
    I2C i2c(D14, D15);
    SHT21 sht(&i2c);
    int8_t ttemp=(int8_t)(sht.readTemp());
    return(ttemp);
}

uint8_t HydroMeas (void)
{
    uint8_t humidity;
    I2C i2c(D14, D15);
    SHT21 sht(&i2c);
    wait_ms(1);
    humidity=(uint8_t)(sht.readHumidity());
    humidity = 94;
    if ( (humidity > 0 ) && ( humidity < 100 )){
        return(humidity);
    } else {
        return (100);
    }
}

AnalogIn Vbat_value(A2);
uint8_t VbatMeas (void)
{
    float vbat=0;
    for (int j =0; j<256; j++) {
        vbat = vbat +Vbat_value.read();
    }
    vbat=vbat/256;
    vbat=((vbat*7)-3); 
    vbat=100*vbat;
    if (vbat>100) {
        vbat=100;
    }
    return((uint8_t)vbat);
}
void PrepareFrame (uint8_t *Buffer) {
    ppayload.Temp       =  TempMeas  ();
    ppayload.Hygro      =  HydroMeas ();
    ppayload.Vbat       =  VbatMeas  ();
    ppayload.Reserved   =  81.2;
    uint8_t* bytes = (uint8_t*)&ppayload;
    uint8_t temp;
    Buffer[0]=APPLI_FW_VERSION;
    for ( int i = 1; i < 8; i++) {
        Buffer[i]= bytes[i-1];
    }
    temp = Buffer[1];
    Buffer[1] = Buffer[4];
    Buffer[4] = temp;
    temp = Buffer[2];
    Buffer[2] = Buffer[3];
    Buffer[3] = temp;
    Buffer[8] = BackUpFlash.NbOfReset;
}


/**************************************************************/
/*              SHT21 CLASS                                   */
/**************************************************************/

SHT21::SHT21(I2C *i2c) :
_i2c(i2c)
{
}

int SHT21::triggerTemp()
{
    return wr(SHT_TRIG_TEMP);
}
 
int SHT21::requestTemp()
{
    int res;
    int rx_bytes = 3;
    char rx[3];
    res = _i2c->read(SHT_I2C_ADDR,rx,rx_bytes);
    unsigned short msb = (rx[0] << 8);
    unsigned short lsb = (rx[1] << 0);
    temperature = msb + lsb;
    return res;
}
 
float SHT21::readTemp()
{
    //First of all trigger the temperature reading
    //process on the sensor
    int trig = triggerTemp();
    
    if(trig != SHT_SUCCESS)
    {
        //if this has failed, exit function with specific error condition
        return SHT_TRIG_FAIL;
    }
    
    //else pause whilst sensor is measuring
    //maximum measuring time is: 85ms
    wait_ms(100);
    
    //Now request the temperature
    if(requestTemp() != SHT_SUCCESS)
    {
        //if this has failed, exit function with specific error condition
        return SHT_READ_FAIL;
    }    
    float realtemp;
    realtemp = -46 + 175 * ( ((float) temperature) / 65536 );
    return realtemp;
} 
 
int SHT21::triggerRH()
{
    return wr(SHT_TRIG_RH);
}

int SHT21::requestRH()
{
    int res;
    char rx[3];
    res = _i2c->read(SHT_I2C_ADDR,rx,3);
    humidity = (rx[0]<<8) + rx[1];
    return res;
}
 
float SHT21::readHumidity()
{
    //First of all trigger the temperature reading
    //process on the sensor
    if(triggerRH() != SHT_SUCCESS)
    {
        //if this has failed, exit function with specific error condition
        return SHT_TRIG_FAIL;
    }
    
    //else pause whilst sensor is measuring
    //maximum measuring time is: 85ms
    wait_ms(100);
    
    //Now request the temperature
    if(requestRH() != SHT_SUCCESS)
    {
        //if this has failed, exit function with specific error condition
        return SHT_READ_FAIL;
    }
    float realhum;
    realhum = -6 + 125 * ( ((float) humidity) / 65536 );
    return realhum;
} 
 
int SHT21::reset()
{
    return wr(SHT_SOFT_RESET);
}


int SHT21::setPrecision(char precision)
{
    int res;
    char command[2];
    command[0] = SHT_WRITE_REG;
    command[1] = precision;
    res = _i2c->write(SHT_I2C_ADDR,command,2);
    return res;
}

int SHT21::wr(int cmd)
{
    int res;
    char command[1];
    command[0] = cmd;
    res = _i2c->write(SHT_I2C_ADDR,command,1);    
    return res;
}
