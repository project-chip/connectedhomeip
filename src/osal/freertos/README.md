# FreeRTOS port of CHIP OSAL

The FreeRTOS port implements the CHIP OSAL abstractions, always using the
equivalent native FreeRTOS module with the goal of mapping to the native
FreeRTOS module in the most minimal and direct way.

The FreeRTOS port provides implementations of the following CHIP OSAL
abstractions:

-   [tasks](#Task) (aka threads)
-   [mutex](#Mutex)
-   [semaphores](#Semaphore)
-   [queues](#Queue)
-   [timers](#Timer)
-   [time](#Time)

## Install FreeRTOS

To test the FreeRTOS port, use Nordic nrf52840dk.

1. Install FreeRTOS included in nRF5 SDK

-   Download and install the
    [Nordic nRF5 SDK for Thread and Zigbee](https://www.nordicsemi.com/Software-and-Tools/Software/nRF5-SDK-for-Thread-and-Zigbee)
    ([Direct download link](https://www.nordicsemi.com/-/media/Software-and-other-downloads/SDKs/nRF5-SDK-for-Thread/nRF5-SDK-for-Thread-and-Zigbee/nRF5SDKforThreadandZigbeev400dc7186b.zip))

2. Install `arm-none-eabi-gcc` toolchain.

3. Set these environment variables

    ```bash
    export NRF5_SDK_ROOT=${HOME}/tools/nRF5_SDK_for_Thread_and_Zigbee_v3.1.0
    export NRF5_TOOLS_ROOT=${HOME}/tools/nRF-Command-Line-Tools
    ```

4. Build the osal and tests

    ```
    cd src/osal/tests
    DEBUG=1 PLATFORM=nrf52840 make -f Makefile.mk
    ```

---

## Reference

### Task

The port uses native [FreeRTOS Task](https://www.freertos.org/a00019.html):
`TaskHandle_t` using `xTaskCreate()`.

### Mutex

The port uses native
[FreeRTOS Recursive Mutex](https://www.freertos.org/a00113.html):
`SemaphoreHandle_t` using `xSemaphoreCreateRecursiveMutex()`.

### Semaphore

The port uses native
[FreeRTOS Counting Semaphore](https://www.freertos.org/a00113.html):
`SemaphoreHandle_t` using `xSemaphoreCreateCounting()`.

### Queue

The port uses native [FreeRTOS Queue](https://www.freertos.org/a00018.html):
`QueueHandle_t` using `xQueueCreate`.

### Timer

The port uses native
[FreeRTOS Timer](https://www.freertos.org/FreeRTOS-Software-Timer-API-Functions.html):
`TimerHandle_t` using `xTimerCreate`.

### Time

The port uses native [FreeRTOS Time](https://www.freertos.org/a00021.html):
`xTaskGetTickCountFromISR`.

## Port Details

### Support Status

| Module | FreeRTOS |
| ------ | -------- |
| Task   | PROTO    |
| Mutex  | PROTO    |
| Sem    | PROTO    |
| Timer  | PROTO    |
| Time   | PROTO    |
| Queue  | PROTO    |

### FreeRTOS Port Layout

```
src/osal/freertos
├── chip
│   └── os_port.h         - Primary port-specific header to hook into public src/include/chip/osal.h
├── os_hw.h               - HW specific ISR helpers (ARM cortex-M specific)
├── os_mutex.c            - Implementation of chip_os_mutex
├── os_queue.c            - Implementation of chip_os_queue
├── os_sem.c              - Implementation of chip_os_sem
├── os_time.c             - Implementation of chip_os_time
├── os_timer.c            - Implementation of chip_os_timer
└── README.md             - This README
```
