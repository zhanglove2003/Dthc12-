#ifndef __DHT12_H__
#define __DHT12_H__

#include "main.h"

#define DHT12_ADDR      (0x44 << 1)
#define DHT12_TIMEOUT   100

typedef enum {
  DHTC12_OK = 0,
  DHTC12_ERR_NO_ACK = 1,
  DHTC12_ERR_TX = 2,
  DHTC12_ERR_RX = 3,
  DHTC12_ERR_CRC = 4,
  DHTC12_ERR_CALIB = 5
} DHTC12_Status;

typedef struct {
  float temperature;
  float humidity;
  uint8_t err;
  uint8_t raw[6];
  uint32_t hal_error;
  uint8_t used_calibration;
} DHT12_Data;

DHTC12_Status DHTC12_Init(void);
DHTC12_Status DHT12_Read(DHT12_Data *data);
const char *DHTC12_StatusString(DHTC12_Status status);
uint32_t DHTC12_LastHalError(void);
uint8_t DHTC12_UsingCalibration(void);

#endif
