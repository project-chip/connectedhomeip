# CHIP ESP32 OTA Requestor Example

A prototype application that demonstrates OTA Requestor capabilities.

Please
[setup ESP-IDF and CHIP Environment](../../../docs/guides/esp32/setup_idf_chip.md)
and refer
[building and commissioning](../../../docs/guides/esp32/build_app_and_commission.md)
guides to get started.

---

-   [Prerequisite](#prerequisite)
-   [Query for an OTA Image](#query-for-an-ota-image)
-   [ESP32 OTA Requestor with Linux OTA Provider](#esp32-ota-requestor-with-linux-ota-provider)
-   [RPC console and Device Tracing](../../../docs/guides/esp32/rpc_console.md)

---

## Prerequisite

Before moving ahead, make sure you have
[OTA Provider](../../ota-provider-app/esp32) is commissioned and running.

### Query for an OTA Image

After commissioning is successful, announce OTA provider's presence using
chip-tool. On receiving this command OTA requestor will query for OTA image.

```
./out/debug/chip-tool otasoftwareupdaterequestor announce-otaprovider <PROVIDER NODE ID> 0 0 0 <REQUESTOR NODE ID> 0
```

Once the transfer is complete, OTA requestor sends ApplyUpdateRequest command to
OTA provider for applying the image. Device will restart on successful
application of OTA image.

### ESP32 OTA Requestor with Linux OTA Provider

-   Build the [Linux OTA Provider](../../ota-provider-app/linux/README.md)
-   Run the Linux OTA Provider with OTA image.

```
./out/debug/chip-ota-provider-app -f hello-world.bin
```

-   Provision the Linux OTA Provider using chip-tool

```
./out/debug/chip-tool pairing onnetwork 12345 20202021
```

### Note

While trying out example ota-requestor-app bump the software version from
`CMakeList.txt` and not from `idf.py menuconfig`. And software version of the
image which is being ota should be greater than current software version.
