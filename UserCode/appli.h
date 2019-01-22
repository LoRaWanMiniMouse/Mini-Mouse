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

typedef enum Rx3Activation {
    RX3_ENABLE,
    RX3_DISABLE,
} Rx3Activation;

class Relay  { 
public:
    Relay( ){
        for (int i = 0 ; i <NB_NODE_IN_RELAY; i ++ ) {
            WhiteList[i].Devaddr= 0xFFFFFFFF;    
            BlackList[i].Devaddr= 0xFFFFFFFF;
            memset (JoinWhiteList[i].DevEui,0xFF,8);
            memset (JoinBlackList[i].DevEui,0xFF,8);    
        }   
    };
    ~Relay ( ) {}; 
    uint32_t RelayExtractDevaddr ( uint8_t *data) {
        return ( data[3] + ( data[4] << 8) + ( data[5] << 16) +  ( data[6] << 24) );
    };
    uint32_t ExtractDevaddrDownLink ( uint8_t *data) {
        return ( data[1] + ( data[2] << 8) + ( data[3] << 16) +  ( data[4] << 24) );
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
            if ( BlackList[i].Devaddr == devaddr) {
                BlackList[i].Devaddr = 0xFFFFFFFF;
                return (OK);
            }
        }
        return (KO);
    };
    BoolOkKo RemoveDevEuiInJoinBlackList (uint8_t devEuiIn[8]){
        for (int i =0; i < NB_NODE_IN_RELAY; i++ ) {
            if ( memcmp ( JoinBlackList[i].DevEui ,devEuiIn , 8 ) == 0 ) {
                memset(JoinBlackList[i].DevEui,0xFF,8);
                return (OK);
            }
        }
        return (KO);
    };
    BoolOkKo AddDevaddrInBlackList (uint32_t devaddr){
        for (int i =0; i < NB_NODE_IN_RELAY; i++ ) {
            if ( BlackList[i].Devaddr == devaddr) {
                return (KO);
            }
        }
        for (int i =0; i < NB_NODE_IN_RELAY; i++ ) {
            if ( BlackList[i].Devaddr == 0xFFFFFFFF) {
                BlackList[i].Devaddr = devaddr;
                return (OK);
            }
        }
        return (KO);
    };
    BoolOkKo AddDevEuiInJoinBlackList (uint8_t devEuiIn[8] ){
        for (int i =0; i < NB_NODE_IN_RELAY; i++ ) {
            if ( memcmp ( JoinBlackList[i].DevEui ,devEuiIn , 8 ) == 0 ) {
                return (KO);
            }
        }
        uint8_t VectTemp[8];
        memset ( VectTemp, 0xFF, 8);
        for (int i =0; i < NB_NODE_IN_RELAY; i++ ) {
            if ( memcmp (JoinBlackList[i].DevEui , VectTemp, 8 ) == 0 ) {
                memcpy ( JoinBlackList[i].DevEui, devEuiIn , 8 ) ;
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
    BoolOkKo RemoveDevEuiInJoinWhiteList (uint8_t devEuiIn[8] ){
        for (int i =0; i < NB_NODE_IN_RELAY; i++ ) {
            if ( memcmp ( JoinWhiteList[i].DevEui ,devEuiIn , 8 ) == 0 ) {
                memset(JoinWhiteList[i].DevEui,0xFF,8);
                AddDevEuiInJoinBlackList (devEuiIn);
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
    BoolOkKo AddDevEuiInJoinWhiteList (uint8_t devEuiIn[8]){
        for (int i =0; i < NB_NODE_IN_RELAY; i++ ) {
            if ( memcmp (devEuiIn , JoinWhiteList[i].DevEui, 8 ) == 0 ) {
                return (KO);
            }
        }
        uint8_t VectTemp[8];
        memset ( VectTemp, 0xFF, 8);
        for (int i =0; i < NB_NODE_IN_RELAY; i++ ) {
            if ( memcmp (JoinWhiteList[i].DevEui , VectTemp, 8 ) == 0 ) {
                memcpy ( JoinWhiteList[i].DevEui, devEuiIn , 8 ) ;
                AddDevEuiInJoinBlackList (devEuiIn);
                return (OK);
            }
        }
        return (KO);
    };
    BoolOkKo SetRssiStatus (uint32_t DevaddrIn , uint8_t Rssi ){
        for (int i =0; i < NB_NODE_IN_RELAY; i++ ) {
            if ( WhiteList[i].Devaddr == DevaddrIn) {
                WhiteList[i].Rssi = Rssi;
                return (OK);
            }
            if ( BlackList[i].Devaddr == DevaddrIn) {
                BlackList[i].Rssi = Rssi;
                return (OK);
            }
        }
        return (KO);
    }
    void builStatus( uint8_t * Payload, uint8_t *Size ) {
        int index = 1;
        uint8_t NbElementWhiteList = 0;
        for (int i =0; i < NB_NODE_IN_RELAY; i++ ) {
            if ( WhiteList[i].Devaddr != 0xFFFFFFFF) {
                Payload[index]   = ( WhiteList[i].Devaddr >> 24 ) & 0xFF;
                Payload[index+1] = ( WhiteList[i].Devaddr >> 16 ) & 0xFF;
                Payload[index+2] = ( WhiteList[i].Devaddr >> 8 ) & 0xFF;
                Payload[index+3] = ( WhiteList[i].Devaddr >> 0 ) & 0xFF;
                Payload[index+4] =  WhiteList[i].Rssi;
                index = index + 5;
                NbElementWhiteList++;
            }
        }
        for (int i =0; i < NB_NODE_IN_RELAY; i++ ) {
            if ( BlackList[i].Devaddr != 0xFFFFFFFF) {
                Payload[index]   = ( BlackList[i].Devaddr >> 24 ) & 0xFF;
                Payload[index+1] = ( BlackList[i].Devaddr >> 16 ) & 0xFF;
                Payload[index+2] = ( BlackList[i].Devaddr >> 8 ) & 0xFF;
                Payload[index+3] = ( BlackList[i].Devaddr >> 0 ) & 0xFF;
                Payload[index+4] =  BlackList[i].Rssi;
                index = index + 5;
            }
        }
        *Size = index;
        Payload[0] = NbElementWhiteList;
    };
     void buildJoinStatus( uint8_t * Payload, uint8_t *Size ) {
        int index = 1;
        uint8_t NbElementJoinWhiteList = 0;
        uint8_t VectTemp[8];
        memset ( VectTemp, 0xFF, 8);
        for (int i =0; i < NB_NODE_IN_RELAY; i++ ) {
            if ( memcmp ( JoinWhiteList[i].DevEui , VectTemp, 8 ) != 0 ) {
                memcpy ( &Payload[index], JoinWhiteList[i].DevEui, 8 );    
                Payload[index+8] = 13;
                index = index + 9;
                NbElementJoinWhiteList++;
            }
        }
        for (int i =0; i < NB_NODE_IN_RELAY; i++ ) {
            if ( memcmp ( JoinBlackList[i].DevEui , VectTemp, 8 ) != 0 ) {
                memcpy ( &Payload[index], JoinBlackList[i].DevEui, 8 );    
                Payload[index+8] = 14;
                index = index + 9;
            }
        }
        *Size = index;
        Payload[0] = NbElementJoinWhiteList + 0x80;
    };
    BoolOkKo SetTxTimeForRx3 (  uint32_t  TxTimeForRx3In , uint32_t  DevaddrIn ) {
        for (int i =0; i < NB_NODE_IN_RELAY; i++ ) {
            if ( WhiteList[i].Devaddr == DevaddrIn) {
                WhiteList[i].TxTimeForRx3 = TxTimeForRx3In;
                WhiteList[i].Rx3Activated = RX3_ENABLE;
                return (OK);
            }
        }
        return (KO);
    }
    BoolOkKo GetTxTimeForRx3 (  uint32_t * TxTimeForRx3Out , uint32_t  DevaddrIn ) {
        for (int i =0; i < NB_NODE_IN_RELAY; i++ ) {
            if ( WhiteList[i].Devaddr == DevaddrIn) {
                *TxTimeForRx3Out = WhiteList[i].TxTimeForRx3 ;
                return (OK);
            }
        }
        return (KO);
    }
    BoolYesNo IsRx3Activated (uint32_t devaddrIn) {
        for (int i =0; i < NB_NODE_IN_RELAY; i++ ) {
            if ( ( WhiteList[i].Devaddr == devaddrIn) && ( WhiteList[i].Rx3Activated == RX3_ENABLE ) ) {
                return (YES);
            }
        }
        return (NO);
    }
    BoolOkKo ClearRx3Activation ( Rx3Activation   Rx3ActivatedIn , uint32_t  DevaddrIn ) {
        for (int i =0; i < NB_NODE_IN_RELAY; i++ ) {
            if ( WhiteList[i].Devaddr == DevaddrIn) {
                WhiteList[i].Rx3Activated = Rx3ActivatedIn ;
                return (OK);
            }
        }
        return (KO);
    }
private :
typedef struct SDevice{
    uint32_t        Devaddr;
    uint32_t        TxTimeForRx3;
    Rx3Activation   Rx3Activated;
    uint8_t         Rssi;
}SDevice;

typedef struct SDeviceNotJoin{
    uint8_t         DevEui[8];
    uint32_t        TxTimeForRx3;
    Rx3Activation   Rx3Activated;
    uint8_t         Rssi;
}SDeviceNotJoin;
DECLARE_ARRAY ( SDevice, NB_NODE_IN_RELAY, WhiteList );
DECLARE_ARRAY ( SDevice, NB_NODE_IN_RELAY, BlackList );
DECLARE_ARRAY ( SDeviceNotJoin, NB_NODE_IN_RELAY, JoinBlackList  );
DECLARE_ARRAY ( SDeviceNotJoin, NB_NODE_IN_RELAY, JoinWhiteList  );
};

extern Relay relay;
#endif

