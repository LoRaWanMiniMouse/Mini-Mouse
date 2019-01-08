#ifndef __POINT_TO_POINT_BASE_H__
#define __POINT_TO_POINT_BASE_H__

#include <stdint.h>

#define WAKE_UP_FRAGMENT_LENGTH 11
#define WAKE_UP_FRAGMENT_DURATION_MS 62.2
#define CAD_BEAT_MS 250
#define CAD_BEAT_PER_CHANNEL_MS (2 * CAD_BEAT_MS)
#define NBR_FREQUENCIES 2

typedef struct
{
  uint32_t wus_tx_attempt;
  uint32_t wuf_tx_attempt;
  uint32_t data_tx_attempt;
  uint32_t ack_rx_attempt;
  uint32_t ack_rx_success;
} StatisticCounters_t;

typedef union
{
  struct
  {
    int16_t delay;
  };
  uint8_t buffer[2];
} Acknowledges_t;

typedef union
{
  struct
  {
    uint8_t blob[10];
    uint8_t index;
  };
  uint8_t buffer[WAKE_UP_FRAGMENT_LENGTH];
} WakeUpFragments_t;

typedef enum { 
    ERROR_PTP = -1,
    OK_PTP    = 0,
}eStatusPtP;

#endif // __POINT_TO_POINT_BASE_H__