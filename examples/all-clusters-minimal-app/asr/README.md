# Matter ASR All Clusters Example

A prototype application that demonstrates device commissioning and cluster
control on ASR platform.

---

-   [Matter ASR All Clusters Example](#matter-asr-all-clusters-example)
    -   [Supported Chips](#supported-chips)
    -   [Building and Commissioning](#building-and-commissioning)
    -   [Cluster Control](#cluster-control)
    -   [Light switch press button and light status LED](#light-switch-press-button-and-light-status-led)

---

## Supported Chips

The Matter demo application is supported on:

-   ASR582X
-   ASR595X

## Building and Commissioning

Please refer
[Building and Commissioning](../../../docs/guides/asr_getting_started_guide.md#building-the-example-application)
guides to get started

```
./scripts/build/build_examples.py --target asr-$ASR_BOARD-all-clusters-minimal build
```

## Cluster Control

After successful commissioning, use `chip-tool` to control the board

For example,control the OnOff Cluster attribute:

```
./chip-tool onoff read on-off <NODE ID> 1
./chip-tool onoff on <NODE ID> 1
./chip-tool onoff off <NODE ID> 1
./chip-tool onoff toggle <NODE ID> 1
```

## Light switch press button and light status LED

This demo uses button to test changing the light states and LED to show the
state of these changes.

|  Name  |  Pin  |
| :----: | :---: |
|  LED   | PAD6  |
| BUTTON | PAD12 |
