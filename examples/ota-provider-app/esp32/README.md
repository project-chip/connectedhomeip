# CHIP ESP32 OTA Provider Example

A prototype application that demonstrates OTA provider capabilities.

Please
[setup ESP-IDF and CHIP Environment](../../../docs/guides/esp32/setup_idf_chip.md)

---

-   [Building and Commissioning](#building-and-commissioning)
-   [Access control list requirements](#access-control-list-requirements)
-   [Set delayed action time (Optional)](#set-delayed-action-time-optional)

---

### Building and Commissioning

Generate the OTA image as described
[here](../../../docs/guides/esp32/ota.md#generate-chip-ota-image)

#### Configure OTA image to serve

-   Copy the binary file which you are going to send to the OTA Requestor to
    `./spiffs_image` path and make sure the `OTA_IMAGE_NAME` is the same as the
    name of the image file you placed in `./spiffs_image`.

-   Please set the below configuration after setting the target

    Edit the `OTA_IMAGE_NAME` through `Demo`->`OTA image file name`.

    ```
    idf.py menuconfig
    ```

    Length of `OTA_IMAGE_NAME` SHALL NOT be more than 30 bytes excluding NULL
    terminator.

Follow
[ESP32 Application Usage Guide](../../../docs/guides/esp32/build_app_and_commission.md)
to Build, Flash, Monitor, and Commission the device.

Once device is commissioned successfully, then please try below steps.

### Access control list requirements

Commissioner or Administrator should install necessary ACL entries at
commissioning time or later to enable processing of QueryImage commands from OTA
Requestors on their fabric, otherwise that OTA Provider will not be usable by
OTA Requestors.

Since the ACL attribute contains a list of entries, we cannot append a single
entry to it. So, read the ACL entries and append an entry which grants operate
privileges to all nodes for the OTA Provider cluster (0x0029) on every endpoint.

-   Read the ACL attribute

```
./out/debug/chip-tool accesscontrol read acl <PROVIDER NODE ID> 0
```

-   Write the ACL attribute

```
out/chip-tool accesscontrol write acl '[{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [112233], "targets": null}, {"fabricIndex": 1, "privilege": 3, "authMode": 2, "subjects": null, "targets": [{"cluster": 41, "endpoint": null, "deviceType": null}]}]' <PROVIDER NODE ID>0
```

First entry created as part of commissioning which grants administer privilege
to the node ID 112233 (default controller node ID) for all clusters on every
endpoint. Seconds entry is the new entry being added which grants operate
privileges to all nodes for the OTA Provider cluster (0x0029) on every endpoint

### Set delayed action time (Optional)

-   Set delayed action time in the Query Image Response and Apply Update
    Response, default is zero.

```
> matter OTAProvider delay <delay seconds>
```

---

Once OTA provider is commissioned then head over to
[OTA Requestor Example](../../ota-requestor-app/esp32).
