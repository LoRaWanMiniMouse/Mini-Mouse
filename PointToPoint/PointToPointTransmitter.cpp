

#define TX_POWER (-5)

#include "UserDefine.h"
#include "PointToPointTransmitter.h"

PointToPointTransmitter::PointToPointTransmitter(RadioPLaner<SX1276> *radio_planner, const uint8_t hook_id) : radio_planner(radio_planner), state(STATE_INIT), hook_id(hook_id), WakeUpSequenceDelay(0),
                                                                                                              WakeUpSequenceLength(WAKE_UP_SEQUENCE_LENGTH_MAX),
                                                                                                              ChannelIndex(0), count_wus_tx_attempt(0), count_wuf_tx_attempt(0), count_data_tx_attempt(0),
                                                                                                              count_ack_rx_attempt(0), count_ack_rx_success(0), ack_length(2)
{
    mcu.InitGpioOut(RX_INDICATOR_PIN);
    mcu.InitGpioOut(TX_INDICATOR_PIN);

    mcu.SetValueDigitalOutPin(TX_INDICATOR_PIN, 0);
    mcu.SetValueDigitalOutPin(RX_INDICATOR_PIN, 0);

    fragment_length = WAKE_UP_FRAGMENT_LENGTH;

    FrequencyList[0] = 868600000;
    FrequencyList[1] = 868200000;

    wakeup_fragment_task_param.Bw = BW125;
    wakeup_fragment_task_param.Sf = 7;
    wakeup_fragment_task_param.CodingRate = CR_4_5;
    wakeup_fragment_task_param.CrcMode = CRC_YES;
    wakeup_fragment_task_param.HeaderMode = IMPLICIT_HEADER;
    wakeup_fragment_task_param.IqMode = IQ_NORMAL;
    wakeup_fragment_task_param.Modulation = LORA;
    wakeup_fragment_task_param.Power = 14;
    wakeup_fragment_task_param.PreambuleLength = 32;
    wakeup_fragment_task_param.SyncWord = 0x34;
    wakeup_fragment_task_param.TimeOutMs = 0;

    wakeup_fragment_task.HookId = this->hook_id;
    wakeup_fragment_task.TaskDuration = WAKE_UP_FRAGMENT_DURATION_MS;
    wakeup_fragment_task.State = TASK_SCHEDULE;
    wakeup_fragment_task.TaskType = TX_LORA;

    ack_relay_rx_task_param.Bw = BW125;
    ack_relay_rx_task_param.Sf = 7;
    ack_relay_rx_task_param.CodingRate = CR_4_5;
    ack_relay_rx_task_param.CrcMode = CRC_YES;
    ack_relay_rx_task_param.HeaderMode = EXPLICIT_HEADER;
    ack_relay_rx_task_param.IqMode = IQ_NORMAL;
    ack_relay_rx_task_param.Modulation = LORA;
    ack_relay_rx_task_param.Power = 14;
    ack_relay_rx_task_param.PreambuleLength = 32;
    ack_relay_rx_task_param.SyncWord = 0x34;
    ack_relay_rx_task_param.TimeOutMs = 20;
    ack_relay_rx_task_param.Rssi = 0;
    ack_relay_rx_task_param.Snr = 0;

    ack_relay_rx_task.HookId = this->hook_id;
    ack_relay_rx_task.TaskDuration = 20;
    ack_relay_rx_task.State = TASK_SCHEDULE;
    ack_relay_rx_task.TaskType = RX_LORA;

    data_send_task_param.Bw = BW125;
    data_send_task_param.Sf = 7;
    data_send_task_param.CodingRate = CR_4_5;
    data_send_task_param.CrcMode = CRC_YES;
    data_send_task_param.HeaderMode = EXPLICIT_HEADER;
    data_send_task_param.IqMode = IQ_NORMAL;
    data_send_task_param.Modulation = LORA;
    data_send_task_param.Power = 14;
    data_send_task_param.PreambuleLength = 8;
    data_send_task_param.SyncWord = 0x34;
    data_send_task_param.TimeOutMs = 50;

    data_send_task.HookId = this->hook_id;
    data_send_task.TaskDuration = 40;
    data_send_task.State = TASK_SCHEDULE;
    data_send_task.TaskType = TX_LORA;

    NextSendSlot = mcu.RtcGetTimeMs();
    last_ack_success_received_ms = NextSendSlot - 1000000 ;// init very far in the past 
    Ftype      = 0;
    //DevAddr    = 0x26011B67;
    DevAddr   = 0x26011D16;
    CntDnw     = WAKE_UP_FRAGMENT_LENGTH;
    Fcount     = 0;
    Channel_Dr = 0;
    for (int i = 0 ; i < WAKE_UP_FRAGMENT_LENGTH ; i++ ) {
        Mic [ i ] = 0x1234;
    }
}

PointToPointTransmitter::~PointToPointTransmitter() {}
void PointToPointTransmitter::SetChannelDr ( uint32_t Channel, uint8_t DataRate ) { 
    DEBUG_PRINTF ( "channel = %d dr = %d\n",Channel,DataRate);
    switch  (Channel ) {
        case 868100000 :
            Channel_Dr =  DataRate  & 0xf ; 
            break;
        case 868300000 :
            Channel_Dr = ( 1<<4 ) +  ((DataRate)  & 0xf) ; 
            break; 
        case 868500000 :
            Channel_Dr = ( 2<<4 ) +  ( (DataRate)  & 0xf) ; 
            break ;
        case 867100000 :
            Channel_Dr = ( 3<<4 ) +  ( (DataRate)  & 0xf) ; 
            break;
        case 867300000 :
            Channel_Dr = ( 4<<4 ) +  ((DataRate)  & 0xf) ; 
            break; 
        case 867500000 :
            Channel_Dr = ( 5<<4 ) +  ( (DataRate)  & 0xf) ; 
            break ;
        case 867700000 :
            Channel_Dr = ( 6<<4 ) +  ((DataRate)  & 0xf) ; 
            break; 
        case 867900000 :
            Channel_Dr = ( 7<<4 ) +  ( (DataRate)  & 0xf) ; 
            break ;
            
        default        :
            Channel_Dr =DataRate  & 0xf ; 
            break;
    }
} ; 
uint32_t  PointToPointTransmitter::Start(uint8_t *data_payload, const uint8_t data_payload_length)
{
    if (this->state != STATE_INIT)
    {
        // DEBUG_PRINTF("Refuse to start: already running (in state 0x%x)\n", this->state);
        return (0);
    }
    DEBUG_PRINTFRP ("start and wk up length = %d\n",WakeUpSequenceLength );
    this->data_payload = data_payload;
    this->data_payload_length = data_payload_length;

    PointToPointTransmitter::GetNextSendSlotTimeAndChannel(
        mcu.RtcGetTimeMs(),
        WakeUpSequenceDelay,
        last_ack_success_received_ms,
        &WakeUpSequenceLength,
        &NextSendSlot,
        &ChannelIndex);

    this->fragment_index = WakeUpSequenceLength;
    PrepareNextWakeUpFragment(&this->fragment, this->fragment_index);

    wakeup_fragment_task_param.Frequency = this->FrequencyList[ChannelIndex];
    data_send_task_param.Frequency       = this->FrequencyList[ChannelIndex];
    ack_relay_rx_task_param.Frequency    = this->FrequencyList[ChannelIndex];

    wakeup_fragment_task.StartTime = NextSendSlot;

    eHookStatus hookStatus = this->radio_planner->EnqueueTask(wakeup_fragment_task, (uint8_t *)&fragment, &this->fragment_length, wakeup_fragment_task_param);

    DEBUG_PRINTFRP(
        "Go!\n"
        "  --> WakeUp #Fragments: %i\n"
        "  --> NextSendSlot: %i\n"
        "  --> Actual Time : %i\n"
        "  --> Next channel index: %i\n",
        WakeUpSequenceLength,
        NextSendSlot,
        mcu.RtcGetTimeMs(),
        ChannelIndex);

    this->state = STATE_SEND_WAKEUP_SEQUENCE_FRAGMENT;
    count_wus_tx_attempt++;
    Fcount++;
    return (NextSendSlot + ( ( WakeUpSequenceLength )* WAKE_UP_FRAGMENT_DURATION_MS ) + 3  );
}

void PointToPointTransmitter::ExecuteStateMachine()
{
    DEBUG_PRINTFRP ( "state =  %d\n",this->state);
    switch (this->state)
    {
    case STATE_INIT:
    {
        break;
    }

    case STATE_SEND_WAKEUP_SEQUENCE_FRAGMENT:
    {
        if (this->fragment_index > 1)
        {
            this->fragment_index--;
            // DEBUG_PRINTF("Tx WuF: %i\n", this->fragment_index);
            PrepareNextWakeUpFragment(&this->fragment, this->fragment_index);
            wakeup_fragment_task.StartTime = mcu.RtcGetTimeMs() + 1;
            uint8_t size_of_fragment = WAKE_UP_FRAGMENT_LENGTH;
            this->radio_planner->EnqueueTask(wakeup_fragment_task, (uint8_t *)&this->fragment, &this->fragment_length, wakeup_fragment_task_param);
            count_wuf_tx_attempt++;
        }
        else
        {
             DEBUG_MSGRP("Send data \n");
            data_send_task.StartTime = mcu.RtcGetTimeMs() + 16;
            eHookStatus hookStatus = this->radio_planner->EnqueueTask(data_send_task, this->data_payload, &this->data_payload_length, data_send_task_param);
            count_data_tx_attempt++;
            if (hookStatus == HOOK_ID_ERROR)
            {
                DEBUG_MSG("Cannot enqueue: abort\n");
                this->Abort();
            }
            state = STATE_WAIT_RELAY_ACK;
        }
        break;
    }
    case STATE_WAIT_RELAY_ACK:
    {
        ack_relay_rx_task.StartTime = mcu.RtcGetTimeMs() + 3;
        this->radio_planner->EnqueueTask(ack_relay_rx_task, this->rx_buffer, &this->ack_length, ack_relay_rx_task_param);
        this->rxSuccess = false;
        count_ack_rx_attempt++;
        state = ACK_RECEIVED;
        break;
    }

    case ACK_RECEIVED:
    {
        if (this->rxSuccess)
        {
            this->last_ack_success_received_ms = mcu.RtcGetTimeMs();
            this->rxSuccess = false;
            count_ack_rx_success++;
            WakeUpSequenceDelay = this->rx_buffer[0] + (this->rx_buffer[1] << 8);
            DEBUG_PRINTFRP("Delay: %i\n", WakeUpSequenceDelay);
        }
        else
        {
            DEBUG_MSG("Missed ack!\n");
        }
        this->state = STATE_INIT;
        break;
    }

    default:
    {
        DEBUG_MSG("Forgot break?");
    }
    }
}

void PointToPointTransmitter::Abort()
{
    this->state = STATE_INIT;
    this->last_ack_success_received_ms = 0;
    this->WakeUpSequenceLength = WAKE_UP_SEQUENCE_LENGTH_MAX;
    this->NextSendSlot = mcu.RtcGetTimeMs();
    this->ChannelIndex = 0;
    this->TxChannel = this->FrequencyList[this->ChannelIndex];
    this->RxDone = false;
    this->TxDone = false;
    this->rxSuccess = false;
}

void PointToPointTransmitter::GetStatistics(StatisticCounters_t *counters){
    counters->ack_rx_attempt = this->count_ack_rx_attempt;
    counters->ack_rx_success = this->count_ack_rx_success;
    counters->wuf_tx_attempt = this->count_wuf_tx_attempt;
    counters->wus_tx_attempt = this->count_wus_tx_attempt;
    counters->data_tx_attempt = this->count_data_tx_attempt;
}

void PointToPointTransmitter::Callback(void *self)
{
    PointToPointTransmitter *me = reinterpret_cast<PointToPointTransmitter *>(self);
    // DEBUG_PRINTF("  --> State = %i\n", me->state);

    ePlanerStatus planner_status;

    uint32_t irq_ts_ms;
    me->radio_planner->GetStatusPlaner(me->hook_id, irq_ts_ms, planner_status);
    me->irq_timestamp_ms = irq_ts_ms;
    // DEBUG_PRINTF("  --> Planner status: %i\n  --> IRQ timestamp: %i\n", planner_status, irq_timestamp_ms);

    switch (planner_status)
    {
    case PLANER_RX_PACKET:
    {
        me->rxSuccess = true;
        break;
    }
    case PLANER_RX_CRC_ERROR:
    case PLANER_RX_TIMEOUT:
    {
        me->rxSuccess = false;
        break;
    }
    case PLANER_TASK_ABORTED:
    {   //if ( me->fragment_index > 1) { 
           // DEBUG_PRINTF("PtP aborted, stop all %d\n",me->fragment_index);
           //me->Abort();
        //}
        break;
    }
    case PLANER_TX_DONE:
    {
        break;
    }
    default:
        DEBUG_MSG("Forgot break?\n");
    }
    me->ExecuteStateMachine();
}

void PointToPointTransmitter::GetNextSendSlotTimeAndChannel(const uint32_t actual_time, const int16_t delay_rx, const uint32_t last_ack_success_time, uint8_t *wake_up_sequence_length, uint32_t *next_send_slot, uint8_t *channel_index)
{
    const uint8_t previous_wake_up_sequence_length = *wake_up_sequence_length;
    const uint32_t last_tx_time = (*next_send_slot);
    const uint32_t t_cad_rx = last_tx_time + (previous_wake_up_sequence_length - 1) * WAKE_UP_FRAGMENT_DURATION_MS + 12 - delay_rx;

    // Search next send opportunity
    // const uint8_t next_wake_up_sequence_length = previous_wake_up_sequence_length%18 + 1;
    uint8_t next_wake_up_sequence_length;
    ComputeNextWakeUpLength(&next_wake_up_sequence_length, actual_time, last_ack_success_time);
    const uint32_t next_wake_up_sequence_window = (next_wake_up_sequence_length - 1) * WAKE_UP_FRAGMENT_DURATION_MS + 24;
    const uint8_t delta_t_ms = 10;
    uint32_t t_cad_next = t_cad_rx;
    uint8_t channel_index_temp = (*channel_index);
    while ( (int) ( (actual_time + (next_wake_up_sequence_window / 2) + delta_t_ms - t_cad_next ) ) > 0) 
    {
        channel_index_temp = !channel_index_temp;
        t_cad_next += CAD_BEAT_MS;
    }
    *next_send_slot = t_cad_next - next_wake_up_sequence_window / 2;
    *wake_up_sequence_length = next_wake_up_sequence_length;
    
    *channel_index = channel_index_temp;
}

void PointToPointTransmitter::PrepareNextWakeUpFragment(WakeUpFragments_t *fragment, const uint8_t fragment_index)
{
    /*for (uint8_t index = 0; index < 10; index++)
    {
        fragment->blob[index] = 0;
    }
    fragment->index = fragment_index;
    */
   fragment->buffer[0]  = Ftype;
   fragment->buffer[1]  = ( DevAddr >> 24 ) & 0xFF;
   fragment->buffer[2]  = ( DevAddr >> 16 ) & 0xFF;
   fragment->buffer[3]  = ( DevAddr >> 8 ) & 0xFF;
   fragment->buffer[4]  =   DevAddr & 0xFF;
   fragment->buffer[5]  = fragment_index ;
   fragment->buffer[6]  = ( Fcount >> 8 ) & 0xFF ; 
   fragment->buffer[7]  = Fcount & 0xFF ;
   fragment->buffer[8]  = Channel_Dr ;
   fragment->buffer[9]  = 0x12;//Mic[fragment_index] >> 8 ;
   fragment->buffer[10] = 0x34;//Mic[fragment_index] & 0xFF ;
}

void PointToPointTransmitter::ComputeNextWakeUpLength(uint8_t *nextWakeUpLength, const uint32_t actual_time, const uint32_t last_ack_success_time)
{
    const uint8_t multiplicator_ppm = 7; // 4;
    uint8_t number_of_fragments = 1;
    uint16_t uncertainty_window = 24 + (number_of_fragments * WAKE_UP_FRAGMENT_DURATION_MS);
    while ((int)(((actual_time - last_ack_success_time) >> multiplicator_ppm) - uncertainty_window ) > 0 )
    {
        number_of_fragments++;
        uncertainty_window += WAKE_UP_FRAGMENT_DURATION_MS;
        if (number_of_fragments >= WAKE_UP_SEQUENCE_LENGTH_MAX)
        {
            break;
        }
    }
    *nextWakeUpLength = number_of_fragments;
}