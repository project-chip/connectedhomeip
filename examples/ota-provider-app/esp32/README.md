# ota-provider-app (ESP32)

### Build and flash OTA apps
- Build and flash [ESP32 OTA provider app](.) and [ESP32 OTA Requestor app](../../ota-requestor-app)
```
$ cd examples/ota-provider-app/esp32
$ idf.py -p <RequestorSerialPort> build flash

$ cd examples/ota-requestor-app/esp32
$ idf.py -p <ProviderSerialPort> build flash
```

- Flash hello-world OTA binary in providor app. Please find hello-world.bin [here](http://shubhamdp.github.io/esp_ota/esp32/hello-world-flash-in-ota-provider-partition.bin)
```
esptool.py -p <ProviderSerialPort> write_flash 0x206400 hello-world-flash-in-ota-provider-partition.bin
```

- Run IDF monitor for both applications in separate terminals
```
idf.py -p <RequestorSerialPort> monitor
idf.py -p <ProviderSerialPort> monitor
```

### Commission both apps
```
# Commission OTA Provider
$ ./chip-tool pairing ble-wifi 12345 <ssid> <passphrase> 0 20202021 3841

# Commission OTA Requestor
$ ./chip-tool pairing ble-wifi 123456 <ssid> <passphrase> 0 20202021 3840
```

## QueryImage from requestor console
```
esp32> QueryImage <ip> 12345 // node id should be in decimal
```

When the image download is complete device waits for an ApplyUpdate command, so fire following command from Requestor app

```
esp32> ApplyUpdateRequest <ip> 12345 // node id should be in decimal
```

After this Requestor will run hello-world application.
