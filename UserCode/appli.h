 #include "stdint.h"
 #include "Define.h"
#ifndef APPLI__H
#define APPLI__H



#define APPLI_FW_VERSION    14


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

#define NB_NODE_IN_RELAY  16
typedef enum BoolYesNo {
    YES,
    NO,
} BoolYesNo;

typedef enum BoolOkKo {
    OK,
    KO,
} BoolOkKo;

class Relay  { 
public:
    Relay( ){
        for (int i = 0 ; i <NB_NODE_IN_RELAY; i ++ ) {
            WhiteList[i].Devaddr= 0xFFFFFFFF;    
            BlackList[i]= 0xFFFFFFFF;    
        }   
    };
    ~Relay ( ) {}; 
    uint32_t RelayExtractDevaddr ( uint8_t *data) {
        return ( data[3] + ( data[4] << 8) + ( data[5] << 16) +  ( data[6] << 24) );
    };
    BoolYesNo IsWhiteListedDevaddr (uint32_t devaddr) {
        BoolYesNo status = NO;
        for (int i = 0 ; i < NB_NODE_IN_RELAY ; i ++ ) {
            if ( WhiteList[i].Devaddr == devaddr ) {
                return (YES);
            }
        }
        return (status);
    };


    BoolOkKo RemoveDevaddrInBlackList (uint32_t devaddr){
        for (int i =0; i < NB_NODE_IN_RELAY; i++ ) {
            if ( BlackList[i] == devaddr) {
                BlackList[i] = 0xFFFFFFFF;
                return (OK);
            }
        }
        return (KO);
    };
    BoolOkKo AddDevaddrInBlackList (uint32_t devaddr){
        for (int i =0; i < NB_NODE_IN_RELAY; i++ ) {
            if ( BlackList[i] == devaddr) {
                return (KO);
            }
        }
        for (int i =0; i < NB_NODE_IN_RELAY; i++ ) {
            if ( BlackList[i] == 0xFFFFFFFF) {
                BlackList[i] = devaddr;
                return (OK);
            }
        }
        return (KO);
    };

    BoolOkKo RemoveDevaddrInWhiteList (uint32_t devaddr){
        for (int i =0; i < NB_NODE_IN_RELAY; i++ ) {
            if ( WhiteList[i].Devaddr == devaddr) {
                WhiteList[i].Devaddr = 0xFFFFFFFF;
                AddDevaddrInBlackList (devaddr);
                return (OK);
            }
        }
        return (KO);
    };
    BoolOkKo AddDevaddrInWhiteList (uint32_t devaddr){
        for (int i =0; i < NB_NODE_IN_RELAY; i++ ) {
            if ( WhiteList[i].Devaddr == devaddr) {
                return (KO);
            }
        }
        for (int i =0; i < NB_NODE_IN_RELAY; i++ ) {
            if ( WhiteList[i].Devaddr == 0xFFFFFFFF) {
                WhiteList[i].Devaddr = devaddr;
                RemoveDevaddrInBlackList (devaddr);
                return (OK);
            }
        }
        return (KO);
    };

    void builStatus( uint8_t * Payload, uint8_t *Size ) {
        int index = 1;
        uint8_t NbElementWhiteList = 0;
        for (int i =0; i < NB_NODE_IN_RELAY; i++ ) {
            if ( WhiteList[i].Devaddr != 0xFFFFFFFF) {
                Payload[index]   = ( WhiteList[i].Devaddr >> 24 ) & 0xFF;
                Payload[index+1] = ( WhiteList[i].Devaddr >> 16 ) & 0xFF;
                Payload[index+2] = ( WhiteList[i].Devaddr >> 8 ) & 0xFF;
                Payload[index+3] = ( WhiteList[i].Devaddr >> 0 ) & 0xFF;
                Payload[index+4] =  14;
                index = index + 5;
                NbElementWhiteList++;
            }
        }
        for (int i =0; i < NB_NODE_IN_RELAY; i++ ) {
            if ( BlackList[i] != 0xFFFFFFFF) {
                Payload[index]   = ( BlackList[i] >> 24 ) & 0xFF;
                Payload[index+1] = ( BlackList[i] >> 16 ) & 0xFF;
                Payload[index+2] = ( BlackList[i] >> 8 ) & 0xFF;
                Payload[index+3] = ( BlackList[i] >> 0 ) & 0xFF;
                Payload[index+4] =  14;
                index = index + 5;
            }
        }
        *Size = index;
        Payload[0] = NbElementWhiteList;
    };
private :
typedef struct SDevice{
    uint32_t  Devaddr;
    uint32_t  TxTimeForRx3;
    uint8_t   Rx3Activated;
}SDevice;
DECLARE_ARRAY ( SDevice, NB_NODE_IN_RELAY, WhiteList );
DECLARE_ARRAY ( uint32_t, NB_NODE_IN_RELAY, BlackList );
};

extern Relay relay;
#endif

