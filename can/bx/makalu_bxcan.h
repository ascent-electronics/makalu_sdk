//
// Created by Finn Carmichael on 4/18/26.
//

#ifndef MAKALU_PDM_MAKALU_BXCAN_H
#define MAKALU_PDM_MAKALU_BXCAN_H

#include <stdint.h>
#include <stdbool.h>
#include "../../../app/makalu_hal.h"
#include "../fd/makalu_fdcan_ids.h"

// def buffer and callback limits
#define MAKALU_BXCAN_RX_BUF_SIZE 32
#define MAKALU_BXCAN_MAX_CALLBACKS 16

// def can frame structure
typedef struct {
    uint32_t id;
    uint8_t data[8];
    uint8_t len;
} makalu_bxcan_frame_t;

// def RX callback type, function pointer
// any function that takes a constant frame pointer and returns void
// can be registered as callback

typedef void (*makalu_bxcan_rx_cb_t)(const makalu_bxcan_frame_t *frame);

// status codes
typedef enum {
    MAKALU_BXCAN_OK = 0,
    MAKALU_BXCAN_ERR_INIT = 1,
    MAKALU_BXCAN_ERR_TX = 2,
    MAKALU_BXCAN_ERR_FULL = 3,
    MAKALU_BXCAN_ERR_INVALID = 4,
} makalu_bxcan_status_t;


// def public api methods below

// calls at startup pass hal can handle as memory address.
makalu_bxcan_status_t makalu_bxcan_init(CAN_HandleTypeDef *hcan1);

// sends one frame onto the canbus. hal puts frame into a tx mailbox, and the hardware
// handles transmission.
makalu_bxcan_status_t makalu_bxcan_send(const makalu_bxcan_frame_t *frame);

// registers a callback for a certain incoming msg. basically associates X can msg
// , with x function to run when received
makalu_bxcan_status_t makalu_bxcan_register(uint8_t msg_type, makalu_bxcan_rx_cb_t cb);

// calls from the HAL CAN RX FIFO0 callback, reads the frame from hardware,
// pushes it into the ring buffer, runs in int context.
void makalu_bxcan_rx_isr(void);

// called from main loop, drains the ring buffer and dispatches each incoming frame to
// its associated callback. if incoming frame doesnt match a msg type in the buffer,
// the frame is ignored.
void makalu_bxcan_process(void);

#endif //MAKALU_PDM_MAKALU_BXCAN_H
