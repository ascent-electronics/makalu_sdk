//
// Created by Finn Carmichael on 4/18/26.
//
#include "makalu_bxcan.h"

// callback entry, stores one registered callback
typedef struct {
 uint8_t msg_type; // makalu msg type
 makalu_bxcan_rx_cb_t cb; // function to call when matched with incoming frame
} makalu_bxcan_cb_t;

// module state, all state is static.auto

// pointer to the HAL CAN handle passed in init
static CAN_HandleTypeDef *s_hcan = NULL;

// ring buffer to hold received frames waiting to be processed (ie called their cb)
static makalu_bxcan_frame_t s_rx_buf[MAKALU_BXCAN_RX_BUF_SIZE];

// where the ISR (hardware) writes the incoming frames to (pointer to idx in buf)
static volatile uint32_t s_rx_head = 0;

// where process reads the next frame to dispatch (pointer to idx in buf)
static volatile uint32_t s_rx_tail = 0;

// table of registered message types mapped to callbacks
static makalu_bxcan_cb_t s_callbacks[MAKALU_BXCAN_MAX_CALLBACKS];

// how many callbacks currently registered
static uint32_t s_cb_count = 0;

// ring buffer helpers
static uint32_t buf_next(uint32_t i) {
 return (i+1) % MAKALU_BXCAN_RX_BUF_SIZE;
}

static bool buf_full(void) {
 return buf_next(s_rx_head) == s_rx_tail;
}

static bool buf_empty(void) {
 return s_rx_head == s_rx_tail;
}

// configures bxcan peripheral via HAL
// implements a filter with all msg passthrough
// impl HAL_CAN_Start() - moves periph to normal operating mode
// Enabled FIFO0 message pending int, when a frame arrives in FIFO0,
// nvic fires the irq which HAL routes to RxFifo0MsgPendingCallback which calls isr.auto
makalu_bxcan_status_t makalu_bxcan_init(CAN_HandleTypeDef *hcan) {
 if (!hcan) return MAKALU_BXCAN_ERR_INVALID;
 s_hcan = hcan;

 CAN_FilterTypeDef f = {0};
 f.FilterActivation = CAN_FILTER_ENABLE;
 f.FilterBank = 0;
 f.FilterFIFOAssignment = CAN_FILTER_FIFO0;
 f.FilterMode = CAN_FILTERMODE_IDMASK; // mask mode = pass all when mask=0
 f.FilterScale = CAN_FILTERSCALE_32BIT;
 // filter id and filter mask default to 0 - pass all ids

 if (HAL_CAN_ConfigFilter(s_hcan, &f) != HAL_OK) return MAKALU_BXCAN_ERR_INIT;
 if (HAL_CAN_Start(s_hcan) != HAL_OK) return MAKALU_BXCAN_ERR_INIT;
 if (HAL_CAN_ActivateNotification(s_hcan, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) {
  return MAKALU_BXCAN_ERR_INIT;
 }
 return MAKALU_BXCAN_OK;
}

// bxcan has 3 tx mailboxes. addtxmsg picks the first free one
// if all three are busy it returns hal_error and return err_tx
// ide = can_id_ext tells the periph this is a 29-bit extended ID frame
// rtr = can_rtr_data means this is a data frame not a remote req
// dlc is clamped to 8 -- bxcan cannot send more than 8 bytes

makalu_bxcan_status_t makalu_bxcan_send(const makalu_bxcan_frame_t *frame) {
 if (!frame) return MAKALU_BXCAN_ERR_INVALID;

 CAN_TxHeaderTypeDef h = {0};
 h.ExtId = frame->id;
 h.IDE = CAN_ID_EXT;
 h.RTR = CAN_RTR_DATA;
 h.DLC = frame->len > 8 ? 8 : frame->len;
 h.TransmitGlobalTime = DISABLE;

 uint32_t mailbox;
 if (HAL_CAN_AddTxMessage(s_hcan, &h, frame->data, &mailbox) != HAL_OK) {
  return MAKALU_BXCAN_ERR_TX;
 }
 return MAKALU_BXCAN_OK;
}

// registers a callback for a specific message type
// when a frame arrives whose ID contains msg_type in 15-8 bits
// the callback is called from bxcan_process().

makalu_bxcan_status_t makalu_bxcan_register(uint8_t msg_type, makalu_bxcan_rx_cb_t cb) {
 if (!cb || s_cb_count >= MAKALU_BXCAN_MAX_CALLBACKS) return MAKALU_BXCAN_ERR_INVALID;

 // adds to top of callback map based on count pointer
 s_callbacks[s_cb_count].msg_type = msg_type;
 s_callbacks[s_cb_count].cb = cb;
 // increment cb counter
 s_cb_count++;

 return MAKALU_BXCAN_OK;
}

// called from int context when a frame arrives in RX FIFO0
// runs inside Hal_CAN_RxFifo0MsgPendingCallback()
// we read frame out of fifo immediately because fifo only holds 3 frames
// if not drained fast enough hardware will overwrite old frames.

// frame is copied into the ring buffer at head, then the head advances.
// if the buffer is full we drop the frame silently.

void makalu_bxcan_rx_isr(void) {
 if (buf_full()) return; // drops frame buffer full
 CAN_RxHeaderTypeDef h;
 // since s_rx_head only increments after a frame has been recieved in here, this will
 // always point to a frame
 makalu_bxcan_frame_t *f = &s_rx_buf[s_rx_head];

 // since frame data is passed in as pointer, no need to assign it below if statement
 if (HAL_CAN_GetRxMessage(s_hcan, CAN_RX_FIFO0, &h, f->data) != HAL_OK) return;
 f->id = h.ExtId;
 f->len = h.DLC; // data bytes
 s_rx_head = buf_next(s_rx_head);
}

// this method gets called from the main loop.
// drains the ring buffer and dispatches frames to their mapped callbacks.
//For each frame:
//1. Extract the message type from the ID using MAKALU_CAN_GET_TYPE()
//2. Walk the callback table looking for a matching msg_type
//3. Call every matching callback with a pointer to the frame

void makalu_bxcan_process(void) {
  while (!buf_empty()) {
   // get least recently added frame from buffer
   makalu_bxcan_frame_t f = s_rx_buf[s_rx_tail];

   // increment tail pointer by one as we have copied the frame so its saved
   // we are about to process it below
   s_rx_tail = buf_next(s_rx_tail);

   uint8_t type = MAKALU_FDCAN_GET_MSG_TYPE(f.id);

   // loop until reach max cb count and look for a callback map for incoming frame msg type
   for (int i = 0; i<s_cb_count; i++) {
    if (s_callbacks[i].msg_type == type) {
     s_callbacks[i].cb(&f);
    }
   }
  }
}