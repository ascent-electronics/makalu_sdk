//
// Created by Finn Carmichael on 4/19/26.
//

#ifndef MAKALU_PDM_AT24C02_H
#define MAKALU_PDM_AT24C02_H

#include "makalu_hal.h"

// Device address — A2=A1=A0=GND
#define AT24C02_ADDR  (0x50 << 1)  // 0xA0 in 8-bit format
#define AT24C02_PAGE_SIZE   8             // AT24C02N page size in bytes
#define AT24C02_MAX_ADDR    255           // 256 bytes total (0x00–0xFF)
#define AT24C02_WRITE_DELAY 5             // ms — mandatory after every write

typedef enum {
    AT24C02_OK    = 0,
    AT24C02_ERROR = 1,
    AT24C02_BUSY  = 2,
} AT24C02_Status;

AT24C02_Status AT24C02_WriteByte  (I2C_HandleTypeDef *hi2c, uint8_t memAddr, uint8_t data);
AT24C02_Status AT24C02_WriteBuffer(I2C_HandleTypeDef *hi2c, uint8_t memAddr, uint8_t *buf, uint16_t len);
AT24C02_Status AT24C02_ReadByte   (I2C_HandleTypeDef *hi2c, uint8_t memAddr, uint8_t *data);
AT24C02_Status AT24C02_ReadBuffer (I2C_HandleTypeDef *hi2c, uint8_t memAddr, uint8_t *buf, uint16_t len);
AT24C02_Status AT24C02_IsReady    (I2C_HandleTypeDef *hi2c);


#endif //MAKALU_PDM_AT24C02_H
