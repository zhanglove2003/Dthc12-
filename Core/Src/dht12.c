#include "dht12.h"

extern I2C_HandleTypeDef hi2c1;

static uint8_t sht_crc(const uint8_t *data, size_t len)
{
  uint8_t crc = 0xFF;
  for (size_t i = 0; i < len; i++)
  {
    crc ^= data[i];
    for (uint8_t b = 0; b < 8; b++)
      crc = (crc & 0x80) ? (crc << 1) ^ 0x31 : (crc << 1);
  }
  return crc;
}

uint8_t DHT12_Read(DHT12_Data *data)
{
  data->err = 1;
  uint8_t buf[6];

  /* 发送 DHTC12 测量命令 0x2C10 */
  uint8_t cmd[] = {0x2C, 0x10};
  if (HAL_I2C_Master_Transmit(&hi2c1, DHT12_ADDR, cmd, 2, DHT12_TIMEOUT) != HAL_OK)
    return 2;

  /* 直接延时 100ms 确保传感器测量完成，避免 NACK 触发 STM32 I2C 硬件死锁 */
  HAL_Delay(100);

  HAL_StatusTypeDef ret = HAL_I2C_Master_Receive(&hi2c1, DHT12_ADDR, buf, 6, DHT12_TIMEOUT);
  if (ret != HAL_OK) return 2;

  for (int i = 0; i < 6; i++) data->raw[i] = buf[i];

  /* CRC 校验 */
  if (sht_crc(buf, 2) != buf[2] || sht_crc(buf + 3, 2) != buf[5])
    return 3;

  /* 温度：有符号16bit，T(°C) = 40 + raw_signed / 256 */
  uint16_t t_u = ((uint16_t)buf[0] << 8) | buf[1];
  float t_f = (t_u & 0x8000) ? (float)t_u - 65536.0f : (float)t_u;
  data->temperature = 40.0f + t_f / 256.0f;

  /* 湿度：无符号16bit，线性转换 + 温漂补偿 */
  uint16_t h_u = ((uint16_t)buf[3] << 8) | buf[4];
  data->humidity = (float)h_u * 100.0f / 65535.0f;
  data->humidity += 0.25f * (data->temperature - 25.0f);
  if (data->humidity > 100.0f) data->humidity = 100.0f;
  if (data->humidity <   0.0f) data->humidity =   0.0f;

  data->err = 0;
  return 0;
}
