//
// Created by Finn Carmichael on 4/19/26.
//

//
// AT24C02N EEPROM Driver
// HAL I2C — STM32F4xx
//

#include "at24c02.h"

// ─── Internal ───────────────────────────────────────────────

static AT24C02_Status wait_for_ready(I2C_HandleTypeDef *hi2c) {
    uint32_t timeout = HAL_GetTick();
    while (HAL_I2C_IsDeviceReady(hi2c, AT24C02_ADDR, 1, 10) != HAL_OK) {
        if (HAL_GetTick() - timeout > 20) return AT24C02_BUSY;
    }
    return AT24C02_OK;
}

// ─── Public API ─────────────────────────────────────────────

AT24C02_Status AT24C02_IsReady(I2C_HandleTypeDef *hi2c) {
    return wait_for_ready(hi2c);
}

AT24C02_Status AT24C02_WriteByte(I2C_HandleTypeDef *hi2c, uint8_t memAddr, uint8_t data) {
    if (memAddr > AT24C02_MAX_ADDR) return AT24C02_ERROR;

    if (wait_for_ready(hi2c) != AT24C02_OK) return AT24C02_BUSY;

    if (HAL_I2C_Mem_Write(hi2c, AT24C02_ADDR,
                          memAddr, I2C_MEMADD_SIZE_8BIT,
                          &data, 1,
                          HAL_MAX_DELAY) != HAL_OK) {
        return AT24C02_ERROR;
    }

    HAL_Delay(AT24C02_WRITE_DELAY);  // mandatory write cycle
    return AT24C02_OK;
}

AT24C02_Status AT24C02_WriteBuffer(I2C_HandleTypeDef *hi2c, uint8_t memAddr, uint8_t *buf, uint16_t len) {
    if (memAddr + len - 1 > AT24C02_MAX_ADDR) return AT24C02_ERROR;

    uint16_t remaining = len;
    uint8_t  currentAddr = memAddr;
    uint8_t  *ptr = buf;

    while (remaining > 0) {
        // How many bytes until end of current page
        uint8_t pageOffset  = currentAddr % AT24C02_PAGE_SIZE;
        uint8_t pageRemain  = AT24C02_PAGE_SIZE - pageOffset;
        uint8_t writeCount  = (remaining < pageRemain) ? remaining : pageRemain;

        if (wait_for_ready(hi2c) != AT24C02_OK) return AT24C02_BUSY;

        if (HAL_I2C_Mem_Write(hi2c, AT24C02_ADDR,
                              currentAddr, I2C_MEMADD_SIZE_8BIT,
                              ptr, writeCount,
                              HAL_MAX_DELAY) != HAL_OK) {
            return AT24C02_ERROR;
        }

        HAL_Delay(AT24C02_WRITE_DELAY);  // mandatory after each page write

        currentAddr += writeCount;
        ptr         += writeCount;
        remaining   -= writeCount;
    }

    return AT24C02_OK;
}

AT24C02_Status AT24C02_ReadByte(I2C_HandleTypeDef *hi2c, uint8_t memAddr, uint8_t *data) {
    if (memAddr > AT24C02_MAX_ADDR) return AT24C02_ERROR;

    if (wait_for_ready(hi2c) != AT24C02_OK) return AT24C02_BUSY;

    if (HAL_I2C_Mem_Read(hi2c, AT24C02_ADDR,
                         memAddr, I2C_MEMADD_SIZE_8BIT,
                         data, 1,
                         HAL_MAX_DELAY) != HAL_OK) {
        return AT24C02_ERROR;
    }

    return AT24C02_OK;
}

AT24C02_Status AT24C02_ReadBuffer(I2C_HandleTypeDef *hi2c, uint8_t memAddr, uint8_t *buf, uint16_t len) {
    if (memAddr + len - 1 > AT24C02_MAX_ADDR) return AT24C02_ERROR;

    if (wait_for_ready(hi2c) != AT24C02_OK) return AT24C02_BUSY;

    // Sequential read — EEPROM handles address auto-increment internally
    if (HAL_I2C_Mem_Read(hi2c, AT24C02_ADDR,
                         memAddr, I2C_MEMADD_SIZE_8BIT,
                         buf, len,
                         HAL_MAX_DELAY) != HAL_OK) {
        return AT24C02_ERROR;
    }

    return AT24C02_OK;
}