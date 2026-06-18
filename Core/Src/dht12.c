#include "dht12.h"

#include <string.h>

extern I2C_HandleTypeDef hi2c1;

#define DHTC12_CMD_SOFT_RESET_HIGH   0x30
#define DHTC12_CMD_SOFT_RESET_LOW    0xA2
#define DHTC12_CMD_MEASURE_HIGH      0x2C
#define DHTC12_CMD_MEASURE_LOW       0x10
#define DHTC12_CMD_READ_REG_HIGH     0xD2

static uint16_t hum_a = 0;
static uint16_t hum_b = 0;
static uint8_t has_calibration = 0;
static uint32_t last_hal_error = 0;

static uint8_t sht_crc(const uint8_t *data, size_t len)
{
  uint8_t crc = 0xFF;
  for (size_t i = 0; i < len; i++)
  {
    crc ^= data[i];
    for (uint8_t b = 0; b < 8; b++)
    {
      crc = (crc & 0x80) ? (uint8_t)((crc << 1) ^ 0x31) : (uint8_t)(crc << 1);
    }
  }
  return crc;
}

static DHTC12_Status capture_hal_error(DHTC12_Status status)
{
  last_hal_error = HAL_I2C_GetError(&hi2c1);
  return status;
}

static DHTC12_Status dhtc12_probe(void)
{
  if (HAL_I2C_IsDeviceReady(&hi2c1, DHT12_ADDR, 2, DHT12_TIMEOUT) != HAL_OK)
  {
    return capture_hal_error(DHTC12_ERR_NO_ACK);
  }
  last_hal_error = HAL_I2C_GetError(&hi2c1);
  return DHTC12_OK;
}

static DHTC12_Status dhtc12_send_command(uint8_t high, uint8_t low)
{
  uint8_t cmd[2] = { high, low };
  if (HAL_I2C_Master_Transmit(&hi2c1, DHT12_ADDR, cmd, sizeof(cmd), DHT12_TIMEOUT) != HAL_OK)
  {
    return capture_hal_error(DHTC12_ERR_TX);
  }
  last_hal_error = HAL_I2C_GetError(&hi2c1);
  return DHTC12_OK;
}

static DHTC12_Status dhtc12_read_register(uint8_t reg, uint8_t *value)
{
  uint8_t cmd[2] = { DHTC12_CMD_READ_REG_HIGH, reg };
  uint8_t rx[3] = {0};

  if (HAL_I2C_Master_Transmit(&hi2c1, DHT12_ADDR, cmd, sizeof(cmd), DHT12_TIMEOUT) != HAL_OK)
  {
    return capture_hal_error(DHTC12_ERR_TX);
  }

  if (HAL_I2C_Master_Receive(&hi2c1, DHT12_ADDR, rx, sizeof(rx), DHT12_TIMEOUT) != HAL_OK)
  {
    return capture_hal_error(DHTC12_ERR_RX);
  }

  if (sht_crc(rx, 2) != rx[2])
  {
    return capture_hal_error(DHTC12_ERR_CRC);
  }

  *value = rx[0];
  last_hal_error = HAL_I2C_GetError(&hi2c1);
  return DHTC12_OK;
}

DHTC12_Status DHTC12_Init(void)
{
  uint8_t hum_a_high;
  uint8_t hum_a_low;
  uint8_t hum_b_high;
  uint8_t hum_b_low;

  has_calibration = 0;
  hum_a = 0;
  hum_b = 0;

  DHTC12_Status status = dhtc12_probe();
  if (status != DHTC12_OK)
  {
    return status;
  }

  status = dhtc12_send_command(DHTC12_CMD_SOFT_RESET_HIGH, DHTC12_CMD_SOFT_RESET_LOW);
  if (status != DHTC12_OK)
  {
    return status;
  }
  HAL_Delay(5);

  if (dhtc12_read_register(0x08, &hum_a_high) == DHTC12_OK &&
      dhtc12_read_register(0x09, &hum_a_low) == DHTC12_OK &&
      dhtc12_read_register(0x0A, &hum_b_high) == DHTC12_OK &&
      dhtc12_read_register(0x0B, &hum_b_low) == DHTC12_OK)
  {
    hum_a = ((uint16_t)hum_a_high << 8) | hum_a_low;
    hum_b = ((uint16_t)hum_b_high << 8) | hum_b_low;
    has_calibration = (hum_a != hum_b);
  }

  return has_calibration ? DHTC12_OK : DHTC12_ERR_CALIB;
}

DHTC12_Status DHT12_Read(DHT12_Data *data)
{
  uint8_t buf[6] = {0};

  memset(data, 0, sizeof(*data));
  data->err = 1;

  DHTC12_Status status = dhtc12_probe();
  if (status != DHTC12_OK)
  {
    data->hal_error = last_hal_error;
    return status;
  }

  status = dhtc12_send_command(DHTC12_CMD_MEASURE_HIGH, DHTC12_CMD_MEASURE_LOW);
  if (status != DHTC12_OK)
  {
    data->hal_error = last_hal_error;
    return status;
  }

  HAL_Delay(100);

  if (HAL_I2C_Master_Receive(&hi2c1, DHT12_ADDR, buf, sizeof(buf), DHT12_TIMEOUT) != HAL_OK)
  {
    data->hal_error = HAL_I2C_GetError(&hi2c1);
    return capture_hal_error(DHTC12_ERR_RX);
  }

  memcpy(data->raw, buf, sizeof(buf));

  if (sht_crc(buf, 2) != buf[2] || sht_crc(buf + 3, 2) != buf[5])
  {
    data->hal_error = HAL_I2C_GetError(&hi2c1);
    return capture_hal_error(DHTC12_ERR_CRC);
  }

  uint16_t t_u = ((uint16_t)buf[0] << 8) | buf[1];
  float t_f = (t_u & 0x8000) ? (float)t_u - 65536.0f : (float)t_u;
  data->temperature = 40.0f + t_f / 256.0f;

  uint16_t h_u = ((uint16_t)buf[3] << 8) | buf[4];
  if (has_calibration)
  {
    data->humidity = ((float)((int32_t)h_u - (int32_t)hum_b) * 60.0f) /
                     (float)((int32_t)hum_a - (int32_t)hum_b) + 30.0f;
    data->used_calibration = 1;
  }
  else
  {
    data->humidity = (float)h_u * 100.0f / 65535.0f;
    data->used_calibration = 0;
  }

  data->humidity += 0.25f * (data->temperature - 25.0f);
  if (data->humidity > 100.0f) data->humidity = 100.0f;
  if (data->humidity < 0.0f) data->humidity = 0.0f;

  data->err = 0;
  data->hal_error = HAL_I2C_GetError(&hi2c1);
  return DHTC12_OK;
}

const char *DHTC12_StatusString(DHTC12_Status status)
{
  switch (status)
  {
    case DHTC12_OK: return "OK";
    case DHTC12_ERR_NO_ACK: return "NO_ACK";
    case DHTC12_ERR_TX: return "TX_ERR";
    case DHTC12_ERR_RX: return "RX_ERR";
    case DHTC12_ERR_CRC: return "CRC_ERR";
    case DHTC12_ERR_CALIB: return "CALIB_FALLBACK";
    default: return "UNKNOWN";
  }
}

uint32_t DHTC12_LastHalError(void)
{
  return last_hal_error;
}

uint8_t DHTC12_UsingCalibration(void)
{
  return has_calibration;
}
