//
// AT24C02 → eeprom_driver_t adapter
//

#ifndef MAKALU_SDK_AT24C02_ADAPTER_H
#define MAKALU_SDK_AT24C02_ADAPTER_H

#include "eeprom_driver.h"
#include "makalu_hal.h"

void at24c02_adapter_init(eeprom_driver_t *drv, I2C_HandleTypeDef *hi2c);

#endif /* MAKALU_SDK_AT24C02_ADAPTER_H */