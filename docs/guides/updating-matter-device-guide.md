# Guide: Updating a Matter Device Using the OTA Provider Sample App

This guide details the end-to-end process of updating a Matter device using the
standard OTA Provider and `chip-tool` applications.

## Prerequisites

1.  A working Matter build environment.
2.  A Matter device (e.g., an example app running on a dev kit) that you can
    commission.
3.  An OTA image package file (`.ota`) for your device.
4.  The `software_version` number associated with your OTA image.

### Node IDs

|                         | Node ID |
| ----------------------- | ------- |
| Device (OTA Requestor)  | 1000    |
| OTA Provider Sample App | 2000    |

### Default from SDK

|                        | Node ID |
| ---------------------- | ------- |
| chip-tool (controller) | 112233  |

---

## Step 1: Build the Required Tools

First, build the `chip-tool` and the `ota-provider-app`. These commands build
the tools from the `connectedhomeip` root directory and place the output in the
`out/` directory.

```bash
# Activate the build environment
source scripts/activate.sh

# Build chip-tool
./scripts/examples/gn_build_example.sh examples/chip-tool out/chiptool

# Build the Linux OTA Provider App
./scripts/examples/gn_build_example.sh examples/ota-provider-app/linux out/debug chip_config_network_layer_ble=false
```

---

## Step 2: Commission Your Matter Device

Before an OTA can be performed, the target device must be commissioned into a
Matter fabric.

1.  **Commission the Device:** Use `chip-tool` to pair the device. This example
    uses a manual pairing code. Replace `${DEVICE_NODE_ID}` with a unique Node
    ID for your device (e.g., `1000`) and `${MANUAL_PAIRING_CODE}` with the code
    from your device.

    ```bash
    # Command to pair a device using its manual pairing code
    ./out/chiptool/chip-tool pairing code ${DEVICE_NODE_ID} ${MANUAL_PAIRING_CODE}

    # Example:
    ./out/chiptool/chip-tool pairing code 1000 3497-011-2332
    ```

    -   **Finding the Pairing Code:** The device will either print a QR code
        link or a numeric code to its console/log output upon startup.
    -   **QR Code:** If you have a QR code, you can use:
        `./out/chiptool/chip-tool pairing code ${DEVICE_NODE_ID} '${QR_CODE_PAYLOAD}'`,
        for example:
        `./out/chiptool/chip-tool pairing code 1000 'MT:EXAMPLE-QR-CODE-PAYLOAD'`

    After successful commissioning, your device is part of the fabric and ready
    for the next steps. Let's assume you assigned the device **Node ID 1000**.

---

## Step 3: Prepare and Run the OTA Update

This process involves running the provider app and then using `chip-tool` to
command the device to check for an update.

### 3.1. Start the OTA Provider App

Open a **new terminal** and start the OTA Provider application. It needs to be
running to serve the OTA image file to the device when requested.

-   The provider will be assigned its own Node ID on the fabric. By default, it
    will use **Node ID \${PROVIDER_NODE_ID}**.
-   You must provide the path to your `.ota` image file using the `-f` flag.

```bash
# In a new terminal, activate the environment
source scripts/activate.sh

# Run the OTA provider app
./out/debug/chip-ota-provider-app --filepath ${OTA_IMAGE_PATH} -q updateAvailable --KVS ${OTA_PROVIDER_KVS_FILE} --secured-device-port ${OTA_PROVIDER_PORT} --discriminator ${OTA_PROVIDER_DISCRIMINATOR} --passcode ${OTA_PROVIDER_MANUAL_PAIRING_CODE}

# Example:
./out/debug/chip-ota-provider-app --filepath image.ota -q updateAvailable --KVS /tmp/chip_kvs_provider --secured-device-port 5541 --discriminator 1111 --passcode 1234-567-8901
```

The provider is now running and waiting for devices to query it.

### 3.2. Configure the Device to Use the OTA Provider

To ensure the device immediately checks for an update, we will manually
configure it to trust our running OTA provider and then trigger an announcement.

**Perform these `chip-tool` commands in your original terminal.**

1.  **Pair OTA Provider using chip-tool on the same fabric:**

    ```bash
    # Command to pair a provider using its manual pairing code
    ./out/chiptool/chip-tool pairing code ${PROVIDER_NODE_ID} ${OTA_PROVIDER_MANUAL_PAIRING_CODE}

    # Example:
    ./out/chiptool/chip-tool pairing onnetwork 2000 0451-082-8541
    ```

2.  **Set ACL on Provider:** Write to the `ACL` attribute on the device's
    "AccessControl" cluster in order to let the Device (DUT) request the
    Provider about available updates.

    -   **Chip-tool (controller) Node ID:** `${ADMIN_NODE_ID}`, defaults to
        `112233`
    -   **DUT (Requestor) Node ID:** `${DEVICE_NODE_ID}`
    -   **OTA Provider Node ID:** `${PROVIDER_NODE_ID}`

    ```bash
    # Write ACL to the Provider:
    ./out/chiptool/chip-tool accesscontrol write acl '[{"privilege": 5, "authMode": 2, "subjects": [${ADMIN_NODE_ID}], "targets": null}, {"fabricIndex": 1, "privilege": 3, "authMode": 2, "subjects": [${DEVICE_NODE_ID}], "targets": [{"cluster": 41, "endpoint": null, "deviceType": null}]}]' ${PROVIDER_NODE_ID} 0

    # Example:
    ./out/chiptool/chip-tool accesscontrol write acl '[{"privilege": 5, "authMode": 2, "subjects": [112233], "targets": null}, {"fabricIndex": 1, "privilege": 3, "authMode": 2, "subjects": [1000], "targets": [{"cluster": 41, "endpoint": null, "deviceType": null}]}]' 2000 0
    ```

3.  **Trigger the Update (Announce OTA Provider):** This is the most effective
    command to force the device to check for an update. It simulates the
    provider announcing its presence to the device.

    -   **Provider Node ID:** for example, `2000`
    -   **Vendor/Product ID:** `0` (can be used as a wildcard)
    -   **Announcement Reason:** `2` for `kUrgentUpdateAvailable` (advisory).
    -   **Target Device Node ID:** e.g., `1000`

    ```bash
    # Announce OTA Provider
    ./out/chiptool/chip-tool otasoftwareupdatemanagement announce-ota-provider ${PROVIDER_NODE_ID} 0 2 0 ${DEVICE_NODE_ID} 0

    # Example:
    ./out/chiptool/chip-tool otasoftwareupdaterequestor announce-otaprovider 2000 0 2 0 1000 0
    ```

### 3.3. Monitor the Update

-   **In the OTA Provider terminal**, you will see log output showing the device
    connecting and downloading the image. Key messages include:

    -   Progress indicators for the image download (`[BDX] ...`).

-   **On the device's console/log**, you will see it receiving the command,
    querying the provider, downloading the new image, and applying the update
    before rebooting.

---

## Step 4: Verify the Update

After the device reboots, you can use `chip-tool` to verify that its software
version has changed.

1.  **Verify the OTA Provider Logs:** Verify that the transfer of the software
    image happens all the way until the last Block is acknowledged from OTA
    Provider to DUT and below is the sample log provided for the raspi platform:

    ```
    [1645748688025] [99779:20370762] CHIP: [BDX] OutputEvent type: AckEOFReceived
    [1645748688025] [99779:20370762] CHIP: [BDX] Transfer completed, got AckEOF
    ```

2.  **Read the SoftwareVersion attribute:** Check the "Basic Information"
    cluster on the device.

    ```bash
    # Replace ${DEVICE_NODE_ID} with your device's ID
    ./out/chiptool/chip-tool basicinformation read software-version ${DEVICE_NODE_ID} 0

    # Example:
    ./out/chiptool/chip-tool basicinformation read software-version 1000 0
    ```

3.  **Read the SoftwareVersionString attribute:**

    ```bash
    # Replace ${DEVICE_NODE_ID} with your device's ID
    ./out/chiptool/chip-tool basicinformation read software-version-string ${DEVICE_NODE_ID} 0

    # Example:
    ./out/chiptool/chip-tool basicinformation read software-version-string 1000 0
    ```

The output should now show the new `software_version` that corresponds to the
OTA image you provided. Congratulations, you have successfully updated your
Matter device!
