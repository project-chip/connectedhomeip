# Matter ASR Thermostat Example

The ASR Thermostat Example demonstrates controlling a thermostat and getting
temperature from local sensor.

---

-   [Matter ASR Thermostat Example](#matter-asr-thermostat-example)
    -   [Building and Commissioning](#building-and-commissioning)
    -   [Cluster Control](#cluster-control)

---

## Building and Commissioning

Please refer
[Building and Commissioning](../../../docs/guides/asr_getting_started_guide.md#building-the-example-application)
guides to get started

```
./scripts/build/build_examples.py --target asr-$ASR_BOARD-thermostat build
```

## Cluster Control

After successful commissioning, use `chip-tool` to control the board

For example,read local-temperature value:

```
./chip-tool thermostat read local-temperature <NODE ID> 1
```

increases the temperature by sending a SetpointRaiseLower command:

```
./chip-tool thermostat setpoint-raise-lower 0 10 <NODE ID> 1
```

decreases the temperature by sending a SetpointRaiseLower command:

```
./chip-tool thermostat setpoint-raise-lower 0 -10 <NODE ID> 1
```
