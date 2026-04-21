//
// Created by Finn Carmichael on 4/18/26.
//

#ifndef MAKALU_SDK_FDCAN_IDS_H
#define MAKALU_SDK_FDCAN_IDS_H
#include <stdint.h>

// FDCAN extended 29bit id builder, excludes bits 25-24 (reserved value)
#define MAKALU_FDCAN_BUILD_ID(priority, src, type, index) \
    ((((uint32_t)(priority) & 0x07) << 26) | \
    (((uint32_t)(src) & 0xFF) << 16) | \
    (((uint32_t)(type) & 0xFF) << 8) | \
    (((uint32_t)(index) & 0xFF) << 0))

// ID destructuring, shift first, then bit mask
#define MAKALU_FDCAN_GET_PRIORITY(id) (((id) >> 26) & 0x07)
#define MAKALU_FDCAN_GET_SRC_NODE(id) (((id) >> 16) & 0xFF)
#define MAKALU_FDCAN_GET_MSG_TYPE(id) (((id) >> 8) & 0xFF)
#define MAKALU_FDCAN_GET_MSG_INDEX(id) (((id) >> 0) & 0xFF)

// ID priority levels
#define MAKALU_FDCAN_PRIO_CRITICAL 0x0
#define MAKALU_FDCAN_PRIO_HIGH  0x1
#define MAKALU_FDCAN_PRIO_NORMAL  0x2
#define MAKALU_FDCAN_PRIO_LOW 0x3

// Define Node IDS
#define MAKALU_FDCAN_N_INTERFACEORGATEWAY 0x01 // Computer interface
#define MAKALU_FDCAN_N_PDM_B_VR0 0x02 // K2 MINI Beta 1 Variant 0
#define MAKALU_FDCAN_N_PDM_B_VR1 0x03 // K2 SUMMIT Beta 1 Variant 1

// Define message IDS

//system
#define MAKALU_MSG_BROADCAST 0x01 // Message that all modules respond to with their node id
#define MAKALU_MSG_PING 0x02 // Message response request from interface to module
#define MAKALU_MSG_PONG 0x03 // Module responds with PONG when receives PING
#define MAKALU_MSG_STATU 0x06 // Module responds with SW/FW/BL version, operation state, etc

// PDM SPECIFIC
#define MAKALU_MSG_PDM_SW_S1 0x04 // Switch on PDM channel (state 1)
#define MAKALU_MSG_PDM_SW_S0 0x05 // Switch off PDM channel (state 0)

// faults
#define MAKALU_MSG_FAULT_TYPE1 0x57 // Critical fault
#define MAKALU_MSG_FAULT_TYPE2 0x58 // Standard fault error
#define MAKALU_MSG_FAULT_TYPE3 0x59 // Low severity fault warning

#define MAKALU_MSG_INFO 0x60 // An information message

// Configuration
#define MAKALU_MSG_CFG_READ 0x71 // Config read register
#define MAKALU_MSG_CFG_WRITE 0x72 // Config write register
#define MAKALU_MSG_CFG_ACK 0x73 // Config operation successful
#define MAKALU_MSG_CFG_NACK // Config operation failure

// Coding
#define MAKALU_MSG_CDG_ANNOUNCE 0x81 // Interface broadcasts to start coding session
#define MAKALU_MSG_CDG_HANDSHAKE 0x82 // Module has started coding session successfully, otherwise would omit NACK
#define MAKALU_MSG_CDG_READ 0x82 // Coding read value - can be done without an ann/handshake
#define MAKALU_MSG_CDG_WRITE 0x83 // Coding write value
#define MAKALU_MSG_CDG_ACK 0x84 // Successful operation
#define MAKALU_MSG_CDG_NACK 0x85 // Unsuccessful operation




#endif //MAKALU_SDK_FDCAN_IDS_H
