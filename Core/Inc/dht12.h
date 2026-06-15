#ifndef __DHT12_H__
#define __DHT12_H__

#include "main.h"

#define DHT12_ADDR      (0x44 << 1)
#define DHT12_TIMEOUT   100

typedef struct {
  float temperature;
  float humidity;
  uint8_t err;
  uint8_t raw[6];
} DHT12_Data;

void DHTC12_Init(void);
uint8_t DHT12_Read(DHT12_Data *data);

#endif
