# Matter ESP32 Thread Border Router Example

A prototype application that demonstrates OpenThread Border Router on ESP32-S3 + ESP32-H2
Thread Border Router DevKit Board.

Please
[setup ESP-IDF and CHIP Environment](../../../docs/guides/esp32/setup_idf_chip.md)
and refer
[building and commissioning](../../../docs/guides/esp32/build_app_and_commission.md)
guides to get started.

---

-   [OpenThread Border Router DevKit Board](#openthread-border-router-board)
-   [OpenThread RCP](#openthread-rcp)
-   [OpenThread CLI](#openthread-cli)
-   [Setup Thread Network](#setup-thread-network)
-   [Commissioning Thread End Devices](#commissioning-thread-end-devices)

---

### Thread Border Router Board

The ESP Thread border router board provides an integrated module of an ESP32-S3 and an ESP32-H2.

![br_dev_kit](./image/esp-thread-border-router-board.png)

### OpenThread RCP

We need to flash an OpenThread RCP(Radio Co-Processor) on ESP32-H2 of the Border Router Board before setting
up the Thread Border example. Connect the USB1 port of the Border Router Board to your host machine. Then
build and flash the RCP firmware

```
cd $IDF_PATH/examples/openthread/ot_rcp
idf.py set-target esp32-h2
idf.py build
idf.py -p {port} erase-flash flash
```

### OpenThread CLI

After you build this example and flash it to the ESP32-S3 of Border Router Board, you can access a standard
OpenThread CLI via the device console with a `matter otcli` prefix.

For instance, you can get the state:
```
> matter otcli state
Detached
Done
```

### Setup Thread Network

You can choose to setup Thread network with the [OpenThread CLI](#openthread-cli).

```
> matter otcli dataset set active <dataset-tlvs>
> matter otcli dataset commit active
> matter otcli ifconfig up
> matter otcli thread start
```

Or you can send SetActiveDatasetRequest command to the Thread BR after commissioning it as a
Matter-Over-Wi-Fi device to setup the Thread network.

```
./chip-tool pairing ble-wifi 1 <ssid> <password> 20202021 3840
./chip-tool generalcommissioning arm-fail-safe 180 1 1 0
./chip-tool threadborderroutermanagement set-active-dataset-request hex:<dataset-tlvs> 1 1 1
```

The Thread BR with enable the Thread network interface and start Thread network after it receives
SetActiveDatasetRequest command.

### Commissioning Thread End Devices

After setting up the Thread network, you can commission a Thread End-device to the Thread network.

```
./chip-tool pairing ble-wifi 2 hex:<dataset_tlvs> <pincode> <discriminator>
```
