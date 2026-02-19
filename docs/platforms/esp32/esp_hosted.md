# ESP-Hosted Instruction

For SoCs that do not include a built-in Wi-Fi radio (such as the ESP32-P4), it
is still possible to run Matter-over-Wi-Fi by using the
[esp-hosted](https://github.com/espressif/esp-hosted) component together with a
slave Wi-Fi and Bluetooth co-processor.

The following guide describes how to setup esp-hosted and configure the slave
co-processor for
[ESP32-P4 Function_EV_Board](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32p4/esp32-p4-function-ev-board/index.html).

## Setup the slave co-processor

Please refer to the
[Flash ESP32-C6](https://github.com/espressif/esp-hosted-mcu/blob/main/docs/esp32_p4_function_ev_board.md#5-flashing-esp32-c6)
section for detailed instructions on flashing the slave co-processor firmware to
the ESP32-C6 on the ESP32-P4 Function_EV_Board.

## Build Matter firmware for Host MCU

The [all-clusters-app](../../../examples/all-clusters-app/esp32/README.md)
example can be used as the Matter firmware for ESP32-P4. After flashing the
slave co-processor, set the target, build and flash the firmware as shown below:

```
idf.py set-target esp32p4
idf.py build
idf.py -p <port> erase-flash flash monitor
```

Once flashed, the device can be commissioned by a Matter commissioner.
