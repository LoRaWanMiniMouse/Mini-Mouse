#include "PointToPointReceiver.h"

#define CAD_DURATION_MS 1
#define ACK_LENGTH 2

PointToPointReceiver::PointToPointReceiver(RadioPLaner<SX1276>* radio_planner,
                                           const uint8_t hook_id)
  : radio_planner(radio_planner)
  , hook_id(hook_id)
  , state(STATE_INIT)
  , last_cad_ms(0)
  , next_cad_ms(0)
  , fragment_length(WAKE_UP_FRAGMENT_LENGTH)
  , ack_length(ACK_LENGTH)
{
  this->FrequencyList[0] = 868600000;
  this->FrequencyList[1] = 868200000;
  this->channel = this->FrequencyList[0];

  cad_task_param.Bw = BW125;
  cad_task_param.Sf = 7;
  cad_task_param.CodingRate = CR_4_5;
  cad_task_param.CrcMode = CRC_YES;
  cad_task_param.HeaderMode = IMPLICIT_HEADER;
  cad_task_param.IqMode = IQ_NORMAL;
  cad_task_param.Modulation = LORA;
  cad_task_param.Power = 14;
  cad_task_param.PreambuleLength = 32;
  cad_task_param.SyncWord = 0x34;
  cad_task_param.TimeOutMs = 0;

  cad_task.HookId = this->hook_id;
  cad_task.TaskDuration = CAD_DURATION_MS;
  cad_task.State = TASK_SCHEDULE;
  cad_task.TaskType = CAD;

  rx_wakeup_fragment_task_param.Bw = BW125;
  rx_wakeup_fragment_task_param.Sf = 7;
  rx_wakeup_fragment_task_param.CodingRate = CR_4_5;
  rx_wakeup_fragment_task_param.CrcMode = CRC_YES;
  rx_wakeup_fragment_task_param.HeaderMode = IMPLICIT_HEADER;
  rx_wakeup_fragment_task_param.IqMode = IQ_NORMAL;
  rx_wakeup_fragment_task_param.Modulation = LORA;
  rx_wakeup_fragment_task_param.Power = 14;
  rx_wakeup_fragment_task_param.PreambuleLength = 32;
  rx_wakeup_fragment_task_param.SyncWord = 0x34;
  rx_wakeup_fragment_task_param.TimeOutMs = 40;
  rx_wakeup_fragment_task_param.Snr = 0;
  rx_wakeup_fragment_task_param.Rssi = 0;

  rx_wakeup_fragment_task.HookId = this->hook_id;
  rx_wakeup_fragment_task.TaskDuration = WAKE_UP_FRAGMENT_DURATION_MS;
  rx_wakeup_fragment_task.State = TASK_SCHEDULE;
  rx_wakeup_fragment_task.TaskType = RX_LORA;

  rx_data_task_param.Bw = BW125;
  rx_data_task_param.Sf = 7;
  rx_data_task_param.CodingRate = CR_4_5;
  rx_data_task_param.CrcMode = CRC_YES;
  rx_data_task_param.HeaderMode = EXPLICIT_HEADER;
  rx_data_task_param.IqMode = IQ_NORMAL;
  rx_data_task_param.Modulation = LORA;
  rx_data_task_param.Power = 14;
  rx_data_task_param.PreambuleLength = 8;
  rx_data_task_param.SyncWord = 0x34;
  rx_data_task_param.TimeOutMs = 40;
  rx_data_task_param.Snr = 0;
  rx_data_task_param.Rssi = 0;

  rx_data_task.HookId = this->hook_id;
  rx_data_task.TaskDuration = 40;
  rx_data_task.State = TASK_SCHEDULE;
  rx_data_task.TaskType = RX_LORA;

  tx_ack_relay_task_param.Bw = BW125;
  tx_ack_relay_task_param.Sf = 7;
  tx_ack_relay_task_param.CodingRate = CR_4_5;
  tx_ack_relay_task_param.CrcMode = CRC_YES;
  tx_ack_relay_task_param.HeaderMode = EXPLICIT_HEADER;
  tx_ack_relay_task_param.IqMode = IQ_NORMAL;
  tx_ack_relay_task_param.Modulation = LORA;
  tx_ack_relay_task_param.Power = 14;
  tx_ack_relay_task_param.PreambuleLength = 8;
  tx_ack_relay_task_param.SyncWord = 0x34;
  tx_ack_relay_task_param.TimeOutMs = 10;
  tx_ack_relay_task_param.Rssi = 0;
  tx_ack_relay_task_param.Snr = 0;

  tx_ack_relay_task.HookId = this->hook_id;
  tx_ack_relay_task.TaskDuration = 58;
  tx_ack_relay_task.State = TASK_SCHEDULE;
  tx_ack_relay_task.TaskType = TX_LORA;
}

PointToPointReceiver::~PointToPointReceiver() {}

void
PointToPointReceiver::Start(uint8_t* data_payload, uint8_t* data_payload_length)
{
  this->cad_success = false;
  this->state = STATE_WAIT_CAD_COMPLETION;
  this->data_received = data_payload;
  this->data_received_length = data_payload_length;
  this->last_cad_ms = mcu.RtcGetTimeMs();
  this->ConfigureAndEnqueueNextCad();
}

void
PointToPointReceiver::ExecuteStateMachine()
{
  switch (this->state) {
    case STATE_INIT: {
      break;
    }

    case STATE_ENQUEUE_CAD: {
      this->ConfigureAndEnqueueNextCad();
      break;
    }

    case STATE_WAIT_CAD_COMPLETION: {
      if (this->cad_success) {
        this->state = STATE_WAIT_RX_WUF_COMPLETION;
        this->rx_wakeup_fragment_task_param.Frequency = this->channel;
        this->rx_success = false;
        this->rx_wakeup_fragment_task.StartTime = mcu.RtcGetTimeMs() + 1;
        this->radio_planner->EnqueueTask(
          this->rx_wakeup_fragment_task, (uint8_t*)&this->fragment.buffer,
          &this->fragment_length, this->rx_wakeup_fragment_task_param);
          DEBUG_MSGRP("CAD SUCESS\n");
      } else {
        this->ConfigureAndEnqueueNextCad();
      }
      break;
    }

    case STATE_WAIT_RX_WUF_COMPLETION: {
      if (this->rx_success) {
        this->rx_success = false;
        this->wake_up_id = this->fragment.index;
        data_rx_ms = rx_done_timestamp +
                     (int)((wake_up_id - 1) * WAKE_UP_FRAGMENT_DURATION_MS) + 3;
        this->rx_data_task.StartTime = data_rx_ms;
        this->rx_data_task_param.Frequency = this->channel;
        this->radio_planner->EnqueueTask(this->rx_data_task, this->rx_buffer,
                                         &this->rx_buffer_length,
                                         this->rx_data_task_param);
        this->state = STATE_WAIT_RX_DATA_COMPLETION;
        // DEBUG_PRINTF("Rx frag. #%i\n", wake_up_id);
      } else {
        this->ConfigureAndEnqueueNextCad();
        DEBUG_MSGRP("Rx Wuf failed\n");
      }
      break;
    }

    case STATE_WAIT_RX_DATA_COMPLETION: {
      if (this->rx_success) {
        this->rx_success = false;
        this->delay_indicator =
          this->GetDelayIndicatorMs(last_cad_ms, data_rx_ms);
        this->ack.delay = delay_indicator;
        this->tx_ack_relay_task.StartTime = mcu.RtcGetTimeMs() + 1;
        this->tx_ack_relay_task_param.Frequency = this->channel;
        this->radio_planner->EnqueueTask(this->tx_ack_relay_task,
                                         this->ack.buffer, &this->ack_length,
                                         this->tx_ack_relay_task_param);
        this->state = STATE_WAIT_TX_ACK_COMPLETION;
      } else {
        DEBUG_PRINTFRP("Wakeup id: %i\n", this->wake_up_id);
        DEBUG_MSGRP("Missed data\n");
        this->ConfigureAndEnqueueNextCad();
      }
      break;
    }

    case STATE_WAIT_TX_ACK_COMPLETION: {
      // DEBUG_PRINTF("Wake Up id: %i\n", this->wake_up_id);
      DEBUG_PRINTFRP("Wakeup id: %i\n", this->wake_up_id);
      DEBUG_PRINTFRP("Ack Sent: %i\n", this->delay_indicator);
      this->ConfigureAndEnqueueNextCad();
      break;
    }

    default: {
      DEBUG_MSG("1-> Forgot break?\n");
      this->ConfigureAndEnqueueNextCad();
    }
  }
}

void
PointToPointReceiver::Callback(void* self)
{
  PointToPointReceiver* me = reinterpret_cast<PointToPointReceiver*>(self);
  // DEBUG_PRINTF("  --> State = %i\n", me->state);

  uint32_t irq_timestamp_ms;
  ePlanerStatus planner_status;

  me->radio_planner->GetStatusPlaner(me->hook_id, irq_timestamp_ms,
                                     planner_status);

  switch (planner_status) {
    case PLANER_CAD_POSITIVE: {
      me->cad_success = true;
      break;
    }

    case PLANER_CAD_NEGATIVE: {

      break;
    }

    case PLANER_RX_PACKET: {
      me->rx_done_timestamp = irq_timestamp_ms;
      me->rx_success = true;

      break;
    }

    case PLANER_RX_TIMEOUT:
    case PLANER_RX_CRC_ERROR: {
      me->rx_success = false;
      break;
    }

    case PLANER_TX_DONE: {
      break;
    }

    case PLANER_TASK_ABORTED: {
      DEBUG_MSGRP("CA\n");
      me->state = STATE_ENQUEUE_CAD;

      break;
    }

    default: {
      DEBUG_PRINTF("2-> Forgot break?: 0x%x\n", planner_status);
    }
  }
  me->ExecuteStateMachine();
}

uint32_t
PointToPointReceiver::GetNextCadStartMs(const uint32_t lastCadMs)
{
  uint32_t delay_ms = 5;
  uint32_t actual_ms = mcu.RtcGetTimeMs() + delay_ms;
  uint32_t next_cad_start_ms =
    ((uint32_t)(lastCadMs / CAD_BEAT_MS)) * CAD_BEAT_MS; // Warning: overflow
  while (next_cad_start_ms <= actual_ms) {
    next_cad_start_ms += CAD_BEAT_MS;
  }
  return next_cad_start_ms;
}

uint32_t
PointToPointReceiver::GetNextFreqency(const uint32_t nextCadMs)
{
  uint8_t frequency_index = ((nextCadMs) / 250) % 2;
  return this->FrequencyList[frequency_index];
}

void
PointToPointReceiver::ConfigureAndEnqueueNextCad()
{
    cad_task_param.Bw = BW125;
  cad_task_param.Sf = 7;
  cad_task_param.CodingRate = CR_4_5;
  cad_task_param.CrcMode = CRC_YES;
  cad_task_param.HeaderMode = IMPLICIT_HEADER;
  cad_task_param.IqMode = IQ_NORMAL;
  cad_task_param.Modulation = LORA;
  cad_task_param.Power = 14;
  cad_task_param.PreambuleLength = 32;
  cad_task_param.SyncWord = 0x34;
  cad_task_param.TimeOutMs = 0;
  
  this->cad_success = false;
  this->state = STATE_WAIT_CAD_COMPLETION;
  this->next_cad_ms = this->GetNextCadStartMs(this->last_cad_ms);
  this->last_cad_ms = this->next_cad_ms;
  this->channel = this->GetNextFreqency(this->next_cad_ms);
  this->cad_task.StartTime = this->next_cad_ms;
  this->cad_task_param.Frequency = this->channel;
  this->radio_planner->EnqueueTask(this->cad_task, NULL, NULL,
                                   this->cad_task_param);
  //  DEBUG_PRINTF("next_cad_ms: %i\n"
  //               "channel: %d\n",
  //               this->next_cad_ms, this->channel);
  //DEBUG_MSG(".");
}

int32_t
PointToPointReceiver::GetDelayIndicatorMs(
  const uint32_t lastCadMs, const uint32_t expectedEndLastWakeUpFragment)
{
  static uint32_t correction_end_packet_middle_preamble_ms = 51;
  int32_t WakeUpSequenceDelay = expectedEndLastWakeUpFragment - lastCadMs -
                                correction_end_packet_middle_preamble_ms;
  while (WakeUpSequenceDelay > CAD_BEAT_PER_CHANNEL_MS) {
    WakeUpSequenceDelay -= CAD_BEAT_PER_CHANNEL_MS;
  }
  return WakeUpSequenceDelay;
}