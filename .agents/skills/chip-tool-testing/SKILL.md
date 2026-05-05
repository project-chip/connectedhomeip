---
name: chip-tool-testing
description:
    Guidelines for building Matter examples and chip-tool, and testing examples
    using chip-tool.
---

# Testing Matter Examples with Chip-Tool

This skill provides guidelines for building Matter example applications and `chip-tool`, and using `chip-tool` to commission and interact with the examples in a simulated environment (like Linux POSIX).

> [!NOTE]
> While `chip-tool` and examples support multiple platforms, these guidelines focus on the Linux/POSIX environment, which is the standard environment for agent execution.

## Building from Source

Always build the `chip-tool` and the Matter example application from the same
revision of the repository to ensure compatibility.

### Building chip-tool

Use the build script to build `chip-tool` for Linux:

```bash
source scripts/activate.sh
./scripts/build/build_examples.py --target linux-x64-chip-tool-clang build
```

The binary will be located in `out/linux-x64-chip-tool-clang/chip-tool`.

### Building Examples

Use the build script to build examples (e.g., `all-devices-app`):

```bash
source scripts/activate.sh
./scripts/build/build_examples.py --target linux-x64-all-devices-boringssl build
```

The binary will be located in
`out/linux-x64-all-devices-boringssl/all-devices-app`.

## Testing Guidelines

### Running the Example

Run the example application (usually in a separate terminal or the background):

```bash
./out/linux-x64-all-devices-boringssl/all-devices-app
```

_Note: Use a clean KVS file (by specifying a new, non-existent file path with `--KVS`) or clear the existing one (by deleting the file) to ensure the application enters commissioning mode on startup._

### 1. Determine Pairing Credentials

Check the application logs on startup to find the setup PIN code and
discriminator. Example log:

```
[DL]   Setup Pin Code: 20202021
[DL]   Setup Discriminator: 3840 (0xF00)
```

### 2. Commissioning (Pairing)

Make sure the device is in commissioning mode. If using a persistent storage
(KVS), you may need to use a clean KVS file or clear it to force commissioning
mode on startup. On Linux, the default KVS file is `/tmp/chip_kvs`:

```bash
rm /tmp/chip_kvs
```

> [!NOTE]
> **Persistence Testing**: If you are performing a persistence test (verifying state across restarts), do NOT delete the KVS file. Reuse the same KVS file to preserve stored attributes.

#### Over IP (On-Network)

If the device is already on the same IP network (e.g., running locally on the
same host):

-   **With Long Discriminator**:
    ```bash
    chip-tool pairing onnetwork-long <node_id> <setup_pin> <discriminator>
    ```
-   **Without Discriminator** (looks for any commissionable device):
    ```bash
    chip-tool pairing onnetwork <node_id> <setup_pin>
    ```
-   **With Manual Pairing Code or QR Code**:
    ```bash
    chip-tool pairing code <node_id> <pairing_code_or_qrcode>
    ```
-   **Direct IP and Port** (useful if mDNS resolution fails):
    ```bash
    chip-tool pairing already-discovered <node_id> <setup_pin> <device_ip> <device_port> --bypass-attestation-verifier true
    ```

### 3. Interacting with Clusters

After successful commissioning, you can use `chip-tool` to interact with data
model clusters.

-   **Read Attribute**:

    ```bash
    chip-tool <cluster_name> read <attribute_name> <node_id> <endpoint_id>
    ```

    Example: `chip-tool basicinformation read vendor-id 0x8016 0`

-   **Invoke Command**:
    ```bash
    chip-tool <cluster_name> <command_name> [arguments] <node_id> <endpoint_id>
    ```
    Example: `chip-tool onoff toggle 0x8016 1`

## Troubleshooting

-   **Timeout**: If a command times out, you can increase it with
    `--timeout <seconds>`.
-   **Configuration Cache**: `chip-tool` caches state in `chip_tool_config.ini`
    (often in the current directory or `/tmp`). Deleting this file can resolve
    stale configuration issues. You can also specify a custom directory with
    `--storage-directory <path>`.

## Reporting Results

When completing a test task using this skill, you should output a structured
report to the user. Since every test may verify different features, do not force
a fixed sequence of steps. Instead, follow these general guidelines for
reporting each item you verified:

For each verified feature, command, or configuration item, provide:

1. **What was verified**: A clear description of the intent (e.g., "Verified
   that Endpoint 2 is a child of Endpoint 1").
2. **The command ran**: The exact `chip-tool` or shell command you used.
3. **The proof (logs)**: A snippet of the output or logs that proves the
   verification was successful (e.g., the attribute value read or the success
   status).

### General Guidelines for Content:

-   **Build Process**: Mention which targets were built and if any custom flags
    were used.
-   **Startup**: Include the command used to run the application, and the
    pairing credentials (Setup PIN and Discriminator) found in the logs.
-   **Commissioning**: Show the command used and the log snippet confirming
    success.
-   **Data Model**: If hierarchy or device composition was verified, show the
    relevant `Descriptor` reads.
-   **Cluster Interaction**: For any attribute read/write or command invocation,
    show the command and the response status.
