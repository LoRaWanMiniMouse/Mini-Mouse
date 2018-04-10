
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
#include "ApiTimers.h"
#include "Define.h"
#include "MacLayer.h"

template class LoraWanContainer <16,SX1276>;
template class LoraWanContainer <16,SX126x>;

template <int NBCHANNEL, class R> void LoraWanContainer<NBCHANNEL, R>::IsrTimerRx1( void ) {
    StateTimer = TIMERSTATE_SLEEP;
    ConfigureRadioForRx1 ( );
};

template <int NBCHANNEL, class R> void LoraWanContainer<NBCHANNEL, R>::IsrTimerRx2( void ) {
    StateTimer = TIMERSTATE_SLEEP;
    ConfigureRadioForRx2 ( );
};
