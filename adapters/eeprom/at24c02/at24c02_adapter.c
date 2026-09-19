//
// AT24C02 → eeprom_driver_t adapter
// Type translation only — no hardware access in this file.
//

#include "at24c02_adapter.h"

#include <string.h>

#include "at24c02.h"

static eeprom_status_t _is_ready(void *ctx) {
    AT24C02_Status s = AT24C02_IsReady((I2C_HandleTypeDef *)ctx);
    if (s == AT24C02_OK)   return EEPROM_OK;
    if (s == AT24C02_BUSY) return EEPROM_BUSY;
    return EEPROM_ERROR;
}

static eeprom_status_t _read_byte(void *ctx, uint16_t addr, uint8_t *data) {
    return (AT24C02_ReadByte((I2C_HandleTypeDef *)ctx, (uint8_t)addr, data) == AT24C02_OK)
           ? EEPROM_OK : EEPROM_ERROR;
}

static eeprom_status_t _write_byte(void *ctx, uint16_t addr, uint8_t data) {
    return (AT24C02_WriteByte((I2C_HandleTypeDef *)ctx, (uint8_t)addr, data) == AT24C02_OK)
           ? EEPROM_OK : EEPROM_ERROR;
}

static eeprom_status_t _read(void *ctx, uint16_t addr, uint8_t *buf, uint16_t len) {
    return (AT24C02_ReadBuffer((I2C_HandleTypeDef *)ctx, (uint8_t)addr, buf, len) == AT24C02_OK)
           ? EEPROM_OK : EEPROM_ERROR;
}

static eeprom_status_t _write(void *ctx, uint16_t addr, const uint8_t *buf, uint16_t len) {
    return (AT24C02_WriteBuffer((I2C_HandleTypeDef *)ctx, (uint8_t)addr, (uint8_t *)buf, len) == AT24C02_OK)
           ? EEPROM_OK : EEPROM_ERROR;
}

static eeprom_status_t _clear(void *ctx) {
    uint8_t zero[AT24C02_SIZE] = {0};
    return (AT24C02_WriteBuffer((I2C_HandleTypeDef *)ctx, 0x00, zero, AT24C02_SIZE) == AT24C02_OK)
               ? EEPROM_OK : EEPROM_ERROR;
}

void at24c02_adapter_init(eeprom_driver_t *drv, I2C_HandleTypeDef *hi2c) {
    drv->is_ready   = _is_ready;
    drv->read_byte  = _read_byte;
    drv->write_byte = _write_byte;
    drv->read       = _read;
    drv->write      = _write;
    drv->clear      = _clear;
    drv->ctx        = hi2c;
}