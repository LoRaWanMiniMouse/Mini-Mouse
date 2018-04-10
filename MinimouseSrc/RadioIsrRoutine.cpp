
/*
  __  __ _       _                                 
 |  \/  (_)     (_)                                
 | \  / |_ _ __  _ _ __ ___   ___  _   _ ___  ___  
 | |\/| | | '_ \| | '_ ` _ \ / _ \| | | / __|/ _ \
 | |  | | | | | | | | | | | | (_) | |_| \__ \  __/ 
 |_|  |_|_|_| |_|_|_| |_| |_|\___/ \__,_|___/\___| 
                                                   
                                                   
Description       : LoraWan Radio ISR Routine.  
Note              : the isr routine isn't a global function , it is a method of RadioContainer template class. 
                  : It could be inside the PhyLayer.cpp file but for more readibility , it is choose to create a .cpp file for this method
License           : Revised BSD License, see LICENSE.TXT file include in the project

Maintainer        : Fabien Holin (SEMTECH)
*/
#include "PhyLayer.h"
#include "ApiTimers.h"
#include "Define.h"
template class RadioContainer<SX1276>;
template class RadioContainer<SX126x>;
template <class R> void RadioContainer <R>::IsrRadio( void ) {
    int status = OKLORAWAN;
    uint32_t tCurrentMillisec;
    RegIrqFlag = Radio->GetIrqFlags( );
    Radio->ClearIrqFlags( ); 
    if ( RegIrqFlag == RECEIVE_PACKET_IRQ_FLAG ) {//@ note (important for phy ) remove all IT mask in config send or rx and check if regirqflag = rxdone + header crc valid 
        status = DumpRxPayloadAndMetadata ( );
        Radio->Sleep ( false );
        if ( status != OKLORAWAN ) { // Case receive a packet but it isn't a valid packet 
            RegIrqFlag = BAD_PACKET_IRQ_FLAG ; // this case is exactly the same than the case of rx timeout
            tCurrentMillisec =  RtcGetTimeMs( );
            uint32_t timeoutMs = LastTimeRxWindowsMs - tCurrentMillisec ;
            if ( (int)( LastTimeRxWindowsMs - tCurrentMillisec - 5 * SymbolDuration ) > 0 ) {
                if ( RxMod == LORA ) {
                      Radio->RxLora( RxBw, RxSf, RxFrequency, timeoutMs );
                } else {
//                    FSK @todo
                }
            DEBUG_MSG( "Receive a packet But rejected\n");
            DEBUG_PRINTF( "tcurrent %u timeout = %d, end time %u \n ", tCurrentMillisec, timeoutMs, LastTimeRxWindowsMs);
            return;
            }
        }
    } else {
        Radio->Sleep ( false );
    }

    //Radio.Sleep ( );
    switch ( StateRadioProcess ) { 
        case RADIOSTATE_TXON :
            TimestampRtcIsr = RtcGetTimeMs ( ); //@info Timestamp only on txdone it
            StateRadioProcess = RADIOSTATE_TXFINISHED;
            break;
        
        case RADIOSTATE_TXFINISHED :
            StateRadioProcess = RADIOSTATE_RX1FINISHED;
            break;
        
       case RADIOSTATE_RX1FINISHED :
            StateRadioProcess = RADIOSTATE_IDLE;
            break;
        
        default :
            DEBUG_MSG ("receive It radio error\n");
            break;
    }
};

