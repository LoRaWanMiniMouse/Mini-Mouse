
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
#include "Define.h"
#include "ApiMcu.h"
#include "utilities.h"
#define FileId 6


template class RadioContainer<SX1276>;
template class RadioContainer<SX1272>;
template class RadioContainer<SX126x>;
template <class R> void RadioContainer <R>::IsrRadio( void ) {
    
 
    int status = OKLORAWAN;
    uint32_t tCurrentMillisec;
    LastItTimeFailsafe = mcu.RtcGetTimeSecond ( );
    if( this->CurrentMod == LORA ) {
        RegIrqFlag = Radio->GetIrqFlagsLora( );
        Radio->ClearIrqFlagsLora( );
    } else {
        RegIrqFlag = Radio->GetIrqFlagsFsk( );
        Radio->ClearIrqFlagsFsk( );
    }
    switch ( RegIrqFlag ) {
        case SENT_PACKET_IRQ_FLAG :
            break;

        case RECEIVE_PACKET_IRQ_FLAG :
            InsertTrace ( __COUNTER__, FileId );
            tCurrentMillisec =  mcu.RtcGetTimeMs( );
            DEBUG_PRINTF( "Receive a packet %d ms after tx done\n",tCurrentMillisec-TimestampRtcIsr);
            status = DumpRxPayloadAndMetadata ( );
            if ( status != OKLORAWAN ) { // Case receive a packet but it isn't a valid packet 
                InsertTrace ( __COUNTER__, FileId );
                tCurrentMillisec =  mcu.RtcGetTimeMs( );
                uint32_t timeoutMs = LastTimeRxWindowsMs - tCurrentMillisec ;
                if (( (int)( LastTimeRxWindowsMs - tCurrentMillisec - 5 * SymbolDuration ) > 0 ) || (StateRadioProcess == RADIOSTATE_RXC)) {
                    if ( RxMod == LORA ) {
                      if ( StateRadioProcess == RADIOSTATE_RXC ) {
                          Radio->RxLora( RxBw, RxSf, RxFrequency, 10000);
                      } else {
                          Radio->RxLora( RxBw, RxSf, RxFrequency, timeoutMs );
                      }
                    } else {
                        Radio->RxFsk( RxFrequency, timeoutMs );
                    }
                    DEBUG_MSG( "Receive a packet But rejected\n");
                    DEBUG_PRINTF( "tcurrent %u timeout = %d, end time %u \n ", tCurrentMillisec, timeoutMs, LastTimeRxWindowsMs);
                    return;
                }
                DEBUG_MSG( "Receive a packet But rejected and too late to restart\n");
                RegIrqFlag = RXTIMEOUT_IRQ_FLAG;
            } 
            break;

        case RXTIMEOUT_IRQ_FLAG :
            if ( StateRadioProcess == RADIOSTATE_RXC ) {
                Radio->RxLora( RxBw, RxSf, RxFrequency, 10000);
                DEBUG_MSG( "  **************************\n " );
                DEBUG_MSG( " *      RXC  Timeout       *\n " );
                DEBUG_MSG( " ***************************\n " );
                return;
            }
    
        case BAD_PACKET_IRQ_FLAG :
            break;
        
        default :
            DEBUG_MSG ("receive It radio error\n");
            break;
    }
    Radio->Sleep ( true );
    switch ( StateRadioProcess ) { 
        case RADIOSTATE_TXON : 
            InsertTrace ( __COUNTER__, FileId );
            TimestampRtcIsr = mcu.RtcGetTimeMs ( ); //@info Timestamp only on txdone it
            StateRadioProcess = RADIOSTATE_TXFINISHED;
            break;
        
        case RADIOSTATE_TXFINISHED :
            InsertTrace ( __COUNTER__, FileId );
            StateRadioProcess = RADIOSTATE_RX1FINISHED;
            break;
        
       case RADIOSTATE_RX1FINISHED :
            InsertTrace ( __COUNTER__, FileId ); 
            StateRadioProcess = RADIOSTATE_IDLE;
            break;
        case RADIOSTATE_RXC :
            StateRadioProcess = RADIOSTATE_IDLE;
            break;
        
        default :
            InsertTrace ( __COUNTER__, FileId ); 
            DEBUG_MSG ("receive It radio error\n");
            break;
    }
};

