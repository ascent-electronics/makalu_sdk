//
// Created by Finn Carmichael on 4/18/26.
//

#ifndef MAKALU_SDK_FDCAN_IDS_H
#define MAKALU_SDK_FDCAN_IDS_H

#include <stdint.h>

#define MAKALU_FDCAN_BUILD_ID(priority, src, type, index) \
    ((((uint32_t)(priority) & 0x07) << 26) | \
    (((uint32_t)(src) & 0xFF) << 16) | \
    (((uint32_t)(type) & 0xFF) << 8) | \
    (((uint32_t)(index) & 0xFF) << 0))

#endif //MAKALU_SDK_FDCAN_IDS_H
