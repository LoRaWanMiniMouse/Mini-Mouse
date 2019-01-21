#ifndef __POINT_TO_POINT_RECEIVER_H__
#define __POINT_TO_POINT_RECEIVER_H__

#include "PointToPointBase.h"
#include "RadioPlaner.h"
#include "sx1276.h"

#define MAX_RX_BUFFER_LENGTH 255

class PointToPointReceiver
{
public:
  explicit PointToPointReceiver(RadioPLaner<SX1276>* radio_planner,
                                const uint8_t hook_id);
  ~PointToPointReceiver();

  void Start(uint8_t* data_payload, uint8_t* data_payload_length);
  void Abort();
  void GetStatistics(StatisticCounters_t* counters);
  void GetRxPayload ( uint8_t * RxPayload, uint8_t * PayloadLength, uint32_t *RxTime );
  static void Callback(void*);
  SRadioParam Tx4Rx3Param;
protected:
  void ExecuteStateMachine();
  eStatusPtP DecodeWakeUpSequence ( void ) ;
  uint32_t GetNextCadStartMs(const uint32_t lastCadMs);
  uint32_t GetNextFreqency(const uint32_t nextCadMs);
  void ConfigureAndEnqueueNextCad();
  static int32_t GetDelayIndicatorMs(
  const uint32_t lastCadMs, const uint32_t expectedEndLastWakeUpFragment);

private:
  RadioPLaner<SX1276>* radio_planner;
  const uint8_t hook_id;

  typedef enum
  {
    STATE_INIT,
    STATE_ENQUEUE_CAD,
    STATE_WAIT_CAD_COMPLETION,
    STATE_WAIT_RX_WUF_COMPLETION,
    STATE_WAIT_DATA_PACKET,
    STATE_WAIT_RX_DATA_COMPLETION,
    STATE_WAIT_TX_ACK_COMPLETION,
  } State_t;

  State_t state;

  volatile bool cad_success;
  volatile bool rx_success;
  volatile bool tx_success;

  uint8_t  rx_buffer[MAX_RX_BUFFER_LENGTH];
  uint8_t  rx_buffer_length;
  uint8_t  RxBufferApp[MAX_RX_BUFFER_LENGTH];
  uint8_t  RxBufferAppLength;
  uint32_t RxBufferAppTime;
  uint32_t FrequencyList[NBR_FREQUENCIES];

  uint32_t last_cad_ms;
  uint32_t next_cad_ms;
  uint32_t data_rx_ms;
  volatile uint32_t rx_done_timestamp;
  uint32_t channel;

  SRadioParam cad_task_param;
  STask cad_task;
  SRadioParam rx_wakeup_fragment_task_param;
  STask rx_wakeup_fragment_task;
  SRadioParam tx_ack_relay_task_param;
  STask tx_ack_relay_task;
  SRadioParam rx_data_task_param;
  STask rx_data_task;
  int16_t RssiRxDataTask;
  int16_t SnrRxDataTask;
  int16_t RssiRxFragmentTask;
  int16_t SnrRxFragmentTask;
  uint8_t* data_received;
  uint8_t* data_received_length;
  WakeUpFragments_t fragment;
  uint8_t fragment_length;
  Acknowledges_t ack;
  uint8_t ack_length;

  uint8_t wake_up_id;
  int32_t delay_indicator;
  void* Lp;
  uint8_t  PtPKey[16];
  uint32_t AddKey ;
};

#endif // __POINT_TO_POINT_RECEIVER_H__