# Matter ASR Temperature Measurement Example

The ASR Temperature Measurement Example demonstrates getting simulated data from
temperature sensor.

---

-   [Matter ASR Temperature Measurement Example](#matter-asr-temperature-measurement-example)
    -   [Building and Commissioning](#building-and-commissioning)
    -   [Cluster Control](#cluster-control)

---

## Building and Commissioning

Please refer
[Building and Commissioning](../../../docs/guides/asr_getting_started_guide.md#building-the-example-application)
guides to get started

```
./scripts/build/build_examples.py --target asr-$ASR_BOARD-temperature-measurement build
```

## Cluster Control

After successful commissioning, use `chip-tool` to control the board

For example,read temperature sensor measured value:

```
./chip-tool temperaturemeasurement read measured-value <NODE ID> 1
```
