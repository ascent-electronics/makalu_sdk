//
// Created by Finn Carmichael on 4/24/26.
//

#ifndef MAKALU_SDK_EEPROM_DRIVER_H
#define MAKALU_SDK_EEPROM_DRIVER_H

#include <stdint.h>

typedef enum {
    EEPROM_OK    = 0,
    EEPROM_ERROR = 1,
    EEPROM_BUSY  = 2,   /* Mirrors AT24C02_BUSY */
} eeprom_status_t;

/*
 * Any new driver (M24C64, SPI flash, etc.) implements these five functions.
 *
 *  is_ready    →  AT24C02_IsReady
 *  read_byte   →  AT24C02_ReadByte
 *  write_byte  →  AT24C02_WriteByte
 *  read        →  AT24C02_ReadBuffer
 *  write       →  AT24C02_WriteBuffer
 */
typedef struct {
    eeprom_status_t (*is_ready)  (void *ctx);
    eeprom_status_t (*read_byte) (void *ctx, uint16_t addr, uint8_t *data);
    eeprom_status_t (*write_byte)(void *ctx, uint16_t addr, uint8_t data);
    eeprom_status_t (*read)      (void *ctx, uint16_t addr, uint8_t *buf, uint16_t len);
    eeprom_status_t (*write)     (void *ctx, uint16_t addr, const uint8_t *buf, uint16_t len);
    eeprom_status_t (*clear)     (void *ctx);
    void *ctx;
} eeprom_driver_t;

#endif //MAKALU_SDK_EEPROM_DRIVER_H
