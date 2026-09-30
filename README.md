# Makalu SDK

Shared firmware SDK for the **Makalu** family of CAN-networked automotive modules from Ascent Electronics. It holds the pieces every Makalu module needs: the CAN driver and protocol definitions, EEPROM drivers behind a portable interface, and (planned) OTA updates, diagnostics, flash management, and CRC utilities.

Module firmware repos, such as [`makalu_pdm`](https://github.com/ascent-electronics/makalu_pdm), include this repo as a git submodule and link against it as a static library. Protocol changes and driver fixes are made once here and picked up by every module.

> **Status:** Early development. The bxCAN driver, CAN ID/protocol definitions, and the AT24C02 EEPROM stack are working and in use on the PDM. The OTA, diagnostics, flash, config, CRC, and CAN-FD modules are scaffolded but not yet built.

---

## What's inside

| Module                     | Path                          | Status      | Purpose                                                    |
| -------------------------- | ----------------------------- | ----------- | ---------------------------------------------------------- |
| bxCAN driver               | `can/bx/`                     | ✅ Working  | Interrupt-driven RX ring buffer and callback dispatch      |
| CAN protocol definitions   | `can/fd/makalu_fdcan_ids.h`   | ✅ Working  | 29-bit ID builder/parser, node IDs, message types          |
| CAN frame layouts          | `can/fd/makalu_fdcan_frames.h`| ✅ Working  | Packed payload structs (heartbeat, status, commissioning)  |
| EEPROM interface           | `drivers/eeprom/eeprom_driver.h` | ✅ Working | Chip-agnostic function-pointer interface               |
| AT24C02 driver             | `drivers/eeprom/at24c02/`     | ✅ Working  | Low-level I²C driver with page-aware writes                |
| AT24C02 adapter            | `adapters/eeprom/at24c02/`    | ✅ Working  | Maps the AT24C02 driver onto `eeprom_driver_t`             |
| CAN-FD driver              | `can/fd/makalu_fdcan.c`       | 🚧 Stub     | For FDCAN-capable MCUs (e.g. higher-end module variants)   |
| OTA                        | `ota/`                        | 🚧 Stub     | Firmware updates over CAN                                  |
| Flash                      | `flash/`                      | 🚧 Stub     | Internal flash layout and access                           |
| Diagnostics                | `diag/`                       | 🚧 Stub     | Diagnostic trouble codes (DTCs)                            |
| Config                     | `config/`                     | 🚧 Stub     | Runtime config store                                       |
| CRC                        | `util/makalu_crc.c`           | 🚧 Stub     | CRC32 for firmware images and stored data                  |

---

## Architecture

```
            ┌───────────────────────────────────────────┐
            │         Module firmware (e.g. PDM)        │
            │   registers CAN callbacks, uses EEPROM    │
            └──────────────┬──────────────┬─────────────┘
                           │              │
          ┌────────────────▼───┐   ┌──────▼──────────────────┐
          │  can/bx            │   │  eeprom_driver_t        │  ← generic interface
          │  makalu_bxcan      │   └──────▲──────────────────┘
          │  ring buffer +     │          │
          │  dispatch table    │   ┌──────┴──────────────────┐
          └────────┬───────────┘   │  adapters/…/at24c02     │  ← type translation only
                   │               └──────▲──────────────────┘
          ┌────────▼───────────┐   ┌──────┴──────────────────┐
          │  can/fd  IDs +     │   │  drivers/…/at24c02      │  ← talks to hardware
          │  frame layouts     │   └──────┬──────────────────┘
          └────────┬───────────┘          │
                   ▼                      ▼
              STM32 HAL CAN         STM32 HAL I²C
```

---

## CAN

### Protocol: 29-bit extended IDs

Every Makalu message uses an extended ID that carries its own routing metadata:

```
 28  26 25 24 23          16 15           8 7            0
┌──────┬─────┬──────────────┬──────────────┬──────────────┐
│ prio │ rsv │  source node │   msg type   │    index     │
└──────┴─────┴──────────────┴──────────────┴──────────────┘
```

```c
uint32_t id = MAKALU_FDCAN_BUILD_ID(MAKALU_FDCAN_PRIO_LOW,
                                    MAKALU_FDCAN_N_PDM_B_VR0,
                                    MAKALU_MSG_HEARTBEAT,
                                    0x00);

uint8_t src  = MAKALU_FDCAN_GET_SRC_NODE(id);
uint8_t type = MAKALU_FDCAN_GET_MSG_TYPE(id);
```

**Priorities:** `0x0` critical, `0x1` high, `0x2` normal, `0x3` low. A lower value wins bus arbitration.

**Nodes:**

| ID     | Node                                   |
| ------ | -------------------------------------- |
| `0x01` | PC interface / gateway                 |
| `0x02` | PDM K2 MINI (Beta 1, Variant 0)        |
| `0x03` | PDM K2 SUMMIT (Beta 1, Variant 1)      |

**Message types:**

| Range         | Group          | Messages                                                        |
| ------------- | -------------- | --------------------------------------------------------------- |
| `0x01–0x06`   | System         | Heartbeat, Ping, Pong, Status                                   |
| `0x04–0x05`   | PDM            | Channel on / off                                                |
| `0x57–0x59`   | Faults         | Critical, standard, low-severity                                |
| `0x60`        | Info           | Informational message                                           |
| `0x6C`        | System         | Commissioning status                                            |
| `0x71–0x73`   | Config         | Read, write, ACK                                                |
| `0x81–0x87`   | Coding (CDG)   | Announce, handshake, read, write, ACK, NACK, chunked write/end  |

The full list is in `can/fd/makalu_fdcan_ids.h`, and payload layouts are in `can/fd/makalu_fdcan_frames.h`.

### bxCAN driver

The driver separates the interrupt from message handling:

1. When a frame lands in RX FIFO0, the ISR copies it into a 32-slot ring buffer and returns. The hardware FIFO only holds 3 frames, so it has to be drained fast.
2. A task or main loop calls `makalu_bxcan_process()`, which pops each frame, extracts its message type from the ID, and calls every callback registered for that type.

```c
// Receive handler: any function with this signature
static void on_ping(const makalu_bxcan_frame_t *frame) { /* ... */ }

makalu_bxcan_init(&hcan1);                        // pass-all filter, start, enable RX IRQ
makalu_bxcan_register(MAKALU_MSG_PING, on_ping);  // up to 16 callbacks

// In a task / loop:
makalu_bxcan_process();

// Sending:
makalu_bxcan_frame_t f = { .id = id, .len = 2, .data = { 0x02, 0x01 } };
makalu_bxcan_send(&f);
```

Hook the ISR into the HAL callback in your CubeMX `main.c`:

```c
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    makalu_bxcan_rx_isr();
}
```

| Setting                       | Default | Location          |
| ----------------------------- | ------- | ----------------- |
| `MAKALU_BXCAN_RX_BUF_SIZE`    | 32      | `makalu_bxcan.h`  |
| `MAKALU_BXCAN_MAX_CALLBACKS`  | 16      | `makalu_bxcan.h`  |

If the ring buffer is full, new frames are dropped.

---

## EEPROM

Application code talks to storage only through `eeprom_driver_t`, a struct of function pointers plus a context pointer:

```c
typedef struct {
    eeprom_status_t (*is_ready)  (void *ctx);
    eeprom_status_t (*read_byte) (void *ctx, uint16_t addr, uint8_t *data);
    eeprom_status_t (*write_byte)(void *ctx, uint16_t addr, uint8_t data);
    eeprom_status_t (*read)      (void *ctx, uint16_t addr, uint8_t *buf, uint16_t len);
    eeprom_status_t (*write)     (void *ctx, uint16_t addr, const uint8_t *buf, uint16_t len);
    eeprom_status_t (*clear)     (void *ctx);
    void *ctx;
} eeprom_driver_t;
```

```c
eeprom_driver_t eeprom;
at24c02_adapter_init(&eeprom, &hi2c1);

uint8_t node_id;
eeprom.read_byte(eeprom.ctx, 0x03, &node_id);
```

### AT24C02 driver

- 256 bytes, I²C address `0x50` (A2 = A1 = A0 = GND)
- Buffer writes are split on 8-byte page boundaries, so a write never wraps within a page
- Polls `IsDeviceReady` before each operation (20 ms timeout) and waits the 5 ms write cycle after each write
- Bounds-checked against the 256-byte address space

### Adding a new storage chip

1. Write a low-level driver under `drivers/eeprom/<chip>/`.
2. Write an adapter under `adapters/eeprom/<chip>/` that fills in an `eeprom_driver_t`.
3. Add both paths to `CMakeLists.txt`.

No application code has to change.

---

## Using the SDK in a module

### 1. Add the submodule

```bash
git submodule add https://github.com/ascent-electronics/makalu_sdk.git sdk
```

### 2. Provide the expected host files

The SDK is portable across STM32 families because it doesn't include a specific HAL header itself. The host project must provide:

- **`app/makalu_hal.h`**, one level above the SDK directory, which includes your MCU's HAL:
  ```c
  #ifndef MAKALU_HAL_H
  #define MAKALU_HAL_H
  #include "stm32f4xx_hal.h"
  #endif
  ```
- **A `stm32cubemx` CMake target**, which STM32CubeMX's CMake generator creates. The SDK links against it for HAL headers and sources.

### 3. Link it

Add the SDK before the CubeMX project so the `makalu_sdk` target exists:

```cmake
add_subdirectory(sdk ${CMAKE_BINARY_DIR}/makalu_sdk)
add_subdirectory(cubemx/<your_project>)

target_link_libraries(${CMAKE_PROJECT_NAME} makalu_sdk)
```

### 4. Wire the CAN ISR

Add the `HAL_CAN_RxFifo0MsgPendingCallback` hook shown above.

### Requirements

- CMake ≥ 3.22
- `arm-none-eabi-gcc` (or `starm-clang`)
- STM32 HAL generated by STM32CubeMX (tested on STM32F446 / bxCAN)

---

## Design decisions

- **One SDK, many modules.** The CAN protocol is a contract between every node on the bus. Keeping the ID layout and message types in a single shared header means two modules can't drift out of sync on what `0x83` means.
- **Metadata in the CAN ID.** Packing priority, source, type, and index into the ID gives priority-based arbitration for free, and lets the driver route frames by type without parsing the payload.
- **Minimal ISR, deferred dispatch.** The interrupt only copies a frame and advances an index. All protocol logic runs outside interrupt context, which keeps latency low and avoids dropped frames when messages arrive in bursts.
- **Callback table instead of a big switch.** Modules register only the messages they care about, and the SDK needs no knowledge of module-specific handlers.
- **Driver / adapter / interface split for storage.** The driver owns the hardware quirks (page boundaries, write delays), the adapter only translates types, and the application sees a generic interface. Moving to a larger EEPROM or SPI flash is a two-file addition.
- **HAL supplied by the host.** Using a host-provided `makalu_hal.h` means the same SDK can target different STM32 families.

---

## Roadmap

- [ ] CRC32 implementation (`util/`)
- [ ] OTA firmware update over CAN, with CRC-checked images (`ota/`, `flash/`)
- [ ] DTC storage and reporting (`diag/`)
- [ ] Runtime config store backed by `eeprom_driver_t` (`config/`)
- [ ] FDCAN driver for FD-capable MCUs (`can/fd/`)
- [ ] Hardware ID filtering in place of the pass-all filter
- [ ] Unit tests for ID packing and the ring buffer (host-side)

---

## Contributors

<!-- Edit to reflect who built what -->
- **Finn Carmichael** ([@finnc0](https://github.com/finnc0)): CAN driver, protocol definitions, EEPROM driver and adapter
- **[@ceriddenn](https://github.com/ceriddenn)**: SDK scaffolding and build structure
