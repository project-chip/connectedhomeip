# Matter ASR Lock Example

The ASR Lock Example demonstrates how to remotely control a door lock device
with one basic bolt. It uses buttons to test changing the lock and device states
and LEDs to show the state of these changes. You can use this example as a
reference for creating your own application.

---

-   [Matter ASR Lock Example](#matter-asr-lock-example)
    -   [Building and Commissioning](#building-and-commissioning)
    -   [Cluster Control](#cluster-control)
    -   [Lock press button and lock status led](#lock-press-button-and-lock-status-led)

---

## Building and Commissioning

Please refer
[Building and Commissioning](../../../docs/guides/asr_getting_started_guide.md#building-the-example-application)
guides to get started

```
./scripts/build/build_examples.py --target asr-$ASR_BOARD-lock build
```

## Cluster Control

After successful commissioning, use `chip-tool` to control the board

-   OnOff Cluster

```
./chip-tool onoff read on-off <NODE ID> 1
./chip-tool onoff on <NODE ID> 1
./chip-tool onoff off <NODE ID> 1
./chip-tool onoff toggle <NODE ID> 1
```

## Lock press button and lock status led

This demo uses button to test changing the lock and device states and LED to
show the state of these changes.

|    Name     |  Pin  |
| :---------: | :---: |
| LOCK-STATE  | PAD7  |
| LOCK-BUTTON | PAD12 |
