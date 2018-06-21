
/*
  __  __ _       _                                 
 |  \/  (_)     (_)                                
 | \  / |_ _ __  _ _ __ ___   ___  _   _ ___  ___  
 | |\/| | | '_ \| | '_ ` _ \ / _ \| | | / __|/ _ \
 | |  | | | | | | | | | | | | (_) | |_| \__ \  __/ 
 |_|  |_|_|_| |_|_|_| |_| |_|\___/ \__,_|___/\___| 
                                                   
                                                   
Description       : LoraWan Timer ISR Routine.  
Note              : the isr routine isn't a global function , it is a method of RadioContainer template class. 
                  : It could be inside the PhyLayer.cpp file but for more readibility , it is choose to create a .cpp file for this method
License           : Revised BSD License, see LICENSE.TXT file include in the project

Maintainer        : Fabien Holin (SEMTECH)
*/

#include "PhyLayer.h"
#include "Define.h"
#include "MacLayer.h"
#include "utilities.h"
#define FileId 7
template class LoraWanContainer <16,SX1276>;
template class LoraWanContainer <16,SX126x>;
template class LoraWanContainer <72,SX1276>;
template class LoraWanContainer <72,SX126x>;
template class LoraWanContainer <16,SX1272>;
template class LoraWanContainer <72,SX1272>;
template <int NBCHANNEL, class R> void LoraWanContainer<NBCHANNEL, R>::IsrTimerRx1( void ) {
    StateTimer = TIMERSTATE_SLEEP;
	  InsertTrace ( __COUNTER__, FileId );
    ConfigureRadioForRx1 ( );
};

template <int NBCHANNEL, class R> void LoraWanContainer<NBCHANNEL, R>::IsrTimerRx2( void ) {
    StateTimer = TIMERSTATE_SLEEP;
	  InsertTrace ( __COUNTER__, FileId );
    ConfigureRadioForRx2 ( );
};
