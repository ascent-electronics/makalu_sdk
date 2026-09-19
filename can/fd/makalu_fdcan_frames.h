//
// Created by Finn Carmichael on 4/26/26.
//

#ifndef MAKALU_PDM_MAKALU_FDCAN_FRAMES_H
#define MAKALU_PDM_MAKALU_FDCAN_FRAMES_H
#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint8_t  fw_major;      /* firmware major version    */
    uint8_t  fw_minor;      /* firmware minor version    */
    uint8_t  fw_patch;      /* firmware patch version    */
    uint8_t  bl_version;    /* bootloader version        */
    uint8_t  op_state;      /* current operation state   */
    uint8_t  hw_revision;   /* hardware revision         */
    uint8_t  node_id;       /* this module's node ID     */
    uint8_t  reserved;      /* reserved for future use   */
} makalu_status_frame_t;

typedef struct __attribute__((packed)) {
    uint8_t node_id;    /* sender node ID                */
    uint8_t op_state;   /* current operation state       */
} makalu_heartbeat_frame_t;

typedef struct __attribute__((packed)) {
    uint8_t configCommissioned;    /* sender node ID                */
    uint8_t codingCommissioned;   /* current operation state       */
} makalu_commissioned_frame_t;

/* ── Operation states ─────────────────────────────────────────────────────── */
typedef enum {
    MAKALU_STATE_BOOT      = 0x00,  /* booting up              */
    MAKALU_STATE_NORMAL    = 0x01,  /* normal operation        */
    MAKALU_STATE_SLEEP_1   = 0x02,  /* sleep state 1           */
    MAKALU_STATE_SLEEP_2   = 0x03,  /* sleep state 2           */
    MAKALU_STATE_OTA       = 0x04,  /* OTA update in progress  */
    MAKALU_STATE_FAULT     = 0x05,  /* fault condition         */
} makalu_op_state_t;


#endif //MAKALU_PDM_MAKALU_FDCAN_FRAMES_H
