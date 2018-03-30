 #include "mbed.h"
#ifndef APPLI__H
#define APPLI__H

//Sensor I2C address
#define SHT_I2C_ADDR        0x80
//Commands...
//Trigger Temp with hold master
#define SHT_TRIG_TEMP_HOLD  0xE3
//Trigger RH with hold master
#define SHT_TRIG_RH_HOLD    0xE5
//Trigger Temp with no hold master
#define SHT_TRIG_TEMP       0xF3
//Trigger RH with no hold master
#define SHT_TRIG_RH         0xF5
//Write to user register
#define SHT_WRITE_REG       0xE6
//Read from user register
#define SHT_READ_REG        0xE7
//Soft reset the sensor
#define SHT_SOFT_RESET      0xFE
//Data precision settings
//RH 12 T 14 - default
#define SHT_PREC_1214       0x00
//RH 8  T 10 
#define SHT_PREC_0812       0x01
//RH 10 T 13
#define SHT_PREC_1013       0x80
//RH 11 T 11
#define SHT_PREC_1111       0x81
//Battery status
#define SHT_BATTERY_STAT    0x40
//Enable on chip heater
#define SHT_HEATER          0x04
//Disable OTP reload
#define SHT_DISABLE_OTP     0x02
//Fail conditions on the I2C bus
#define SHT_FAIL            1
#define SHT_SUCCESS         0
//Author fail conditions
//1, 2, 3 can be used because these are status bits
//in the received measurement value
#define SHT_GOOD            0xFFFC
#define SHT_TRIG_FAIL       1
#define SHT_READ_FAIL       2


#define APPLI_FW_VERSION    14
/** SHT21 Connection class, utilizing a I2C interface*
*/
class SHT21
{
private:
    I2C *_i2c;
    int triggerTemp();  
    int requestTemp();
    unsigned short temperature;
    int triggerRH();
    int requestRH();
    unsigned short humidity;
    int wr(int cmd);
    
public:

    SHT21(I2C *i2c);
    float readTemp();
    float readHumidity();
    int reset();
    int setPrecision(char precision);
};

int8_t  TempMeas  ( void );
uint8_t HydroMeas ( void );
uint8_t VbatMeas  ( void );
float   LoadMeas  ( void );
typedef struct {
    float    Reserved;
    uint8_t  Vbat;
    int8_t   Temp;
    uint8_t  Hygro;
} payload;
void PrepareFrame (uint8_t *Buffer);

#endif

