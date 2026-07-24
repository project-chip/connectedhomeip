# Testing Native Apps on Raspberry Pi

This section documents how to cross-compile the Matter lighting application as a
native Linux ARM64 binary, deploy it to a Raspberry Pi 4 running Tizen OS, and
test it using `chip-tool`.

Unlike the TPK approach, this method runs the application as a standalone
executable without the Tizen package manager. This is useful for rapid
development iteration.

## Prerequisites

-   Raspberry Pi 4 running Tizen OS with Wi-Fi configured (see
    [Installing Tizen on Raspberry Pi](./raspberry_pi_install.md))
-   `sdb` tool — available inside the Tizen Docker container (see
    [Building for Tizen](./building.md#docker-compose-environment))
-   `chip-tool` built for your host PC (see
    [Building chip-tool](./building.md#building-chip-tool-for-the-host-pc))

## Building

Use the `crosscompile` Docker service to build the Linux ARM64 variant:

```bash
docker compose run --rm crosscompile bash
source scripts/activate.sh
./scripts/build/build_examples.py --target linux-arm64-light-no-thread-no-ble-clang build
exit
```

The output binary will be in `out/linux-arm64-light-no-thread-no-ble-clang/`.

:::{note} All Docker services share the same workspace volume. Artifacts built
in the `crosscompile` container (e.g.,
`out/linux-arm64-light-no-thread-no-ble-clang/`) are also accessible from the
`tizen` container, and vice versa. :::

## Installing the Binary via SDB

Use `sdb` from the Tizen Docker container. The build output from `crosscompile`
is available in the shared workspace:

```bash
docker compose run --rm tizen bash
sdb connect RASPBERRY_PI_IP
sdb root on

# Ensure the target directory exists on the Raspberry Pi
sdb shell mkdir -p /opt/matter/

# Push the native executable binary to the target deployment path
sdb push out/linux-arm64-light-no-thread-no-ble-clang/chip-lighting-app /opt/matter/

# Mark the binary as an executable on the device
sdb shell chmod +x /opt/matter/chip-lighting-app
```

## Running the Application

Run the application on the Raspberry Pi with target network parameters:

```bash
sdb shell /opt/matter/chip-lighting-app --wifi true --discriminator 1234 --passcode 11223344
```

## Commissioning and Control via chip-tool

Once the application is running on the Raspberry Pi, use `chip-tool` on your
host PC to pair and control the device.

### Network Commissioning

Pair the target device over the network. We assign `Node ID = 1` using the
configured passcode:

```bash
./out/linux-x64-chip-tool/chip-tool pairing onnetwork 1 11223344
```

Verify that the log output displays successful CASE session establishment.

### Controlling the Cluster State (Toggle)

Send an On/Off toggle action command to the commissioned lighting application at
`Node ID = 1` on endpoint `1`:

```bash
./out/linux-x64-chip-tool/chip-tool onoff toggle 1 1
```

## Verification

-   **Raspberry Pi Logs:** The terminal should register an incoming ZCL
    interaction model action frame and transition the light attribute state
    (`OFF_ACTION` / `ON_ACTION`).
-   **PC Terminal Status:** The `chip-tool` output must return a successful
    command transmission status.

## Cleaning Up

To stop the application on the Raspberry Pi:

```bash
sdb shell killall chip-lighting-app
```

To remove the deployed binary:

```bash
sdb shell rm -rf /opt/matter/
```

## Profiling Memory Leaks with AddressSanitizer

To profile memory leaks in the application, use AddressSanitizer (ASan) during cross-compilation.

### Step 1: Build with ASan Enabled

Compile the application with the ASan target inside the `crosscompile` Docker container:

```bash
docker compose run --rm crosscompile bash
source scripts/activate.sh
./scripts/build/build_examples.py --target linux-arm64-light-no-thread-no-ble-asan-clang build
exit
```

The ASan-enabled binary will be in `out/linux-arm64-light-no-thread-no-ble-asan-clang/`.

### Step 2: Deploy the ASan Binary

Push the ASan-enabled binary to the Raspberry Pi:

```bash
docker compose run --rm tizen bash
sdb connect RASPBERRY_PI_IP
sdb root on
sdb shell mkdir -p /opt/matter/
sdb push out/linux-arm64-light-no-thread-no-ble-asan-clang/chip-lighting-app /opt/matter/
sdb shell chmod +x /opt/matter/chip-lighting-app
```

### Step 3: Run with ASan Verbosity

Run the application on the Raspberry Pi with the `ASAN_OPTIONS` environment variable to get verbose output:

```bash
sdb shell ASAN_OPTIONS='verbosity=1' /opt/matter/chip-lighting-app --wifi true --discriminator 1234 --passcode 11223344
```

### Step 4: Introduce a Memory Leak (for Testing)

Since the application code typically does not contain memory leaks, you can introduce one for testing purposes. Add the following code to `examples/lighting-app/linux/main.cpp`:

```cpp
int main(int argc, char * argv[])
{
#if 1
    // Memory leak test - allocates array without freeing
    int *leak_test = new int[500];
    leak_test[0] = 1;
    leak_test[1] = 1;

    for (int i = 2; i < 500; i++) {
        leak_test[i] = leak_test[i-1] + leak_test[i-2];  // Fibonacci sequence
    }
#endif
    // ... rest of main
```

:::{note} The Fibonacci calculation ensures the allocated memory is actually used, preventing compiler warnings about unused variables that could cause build failures. :::

### Step 5: Recompile and Run with the Memory Leak

Rebuild the application with the memory leak code:

```bash
docker compose run --rm crosscompile bash
source scripts/activate.sh
./scripts/build/build_examples.py --target linux-arm64-light-no-thread-no-ble-asan-clang build
exit
```

Deploy the new binary:

```bash
docker compose run --rm tizen bash
sdb push out/linux-arm64-light-no-thread-no-ble-asan-clang/chip-lighting-app /opt/matter/
```

Run the application and observe the ASan leak report:

```bash
sdb shell /opt/matter/chip-lighting-app --wifi true --discriminator 1234 --passcode 11223344
```

ASan will output a memory leak report similar to:

```
==60452==ERROR: LeakSanitizer: detected memory leaks

Direct leak of 2000 byte(s) in 1 object(s) allocated from:
    #0 0xaaaab622a86c  (/opt/matter/chip-lighting-app+0x30a86c) (BuildId: a7f1faf24f851d1106d1c3e27c3be0dd7298a019)
    #1 0xaaaab623d13c  (/opt/matter/chip-lighting-app+0x31d13c) (BuildId: a7f1faf24f851d1106d1c3e27c3be0dd7298a019)
    #2 0xffff97575f58  (/lib64/libc.so.6+0x25f58) (BuildId: 940fe8e11a8d820bbca2e78f4f4a2b26150643f6)
    #3 0xffff97576034  (/lib64/libc.so.6+0x26034) (BuildId: 940fe8e11a8d820bbca2e78f4f4a2b26150643f6)
    #4 0xaaaab6150b2c  (/opt/matter/chip-lighting-app+0x230b2c) (BuildId: a7f1faf24f851d1106d1c3e27c3be0dd7298a019)

SUMMARY: AddressSanitizer: 2000 byte(s) leaked in 1 allocation(s).
```

### Step 6: Map Leak Output to Code Paths

Convert the hexadecimal addresses from the ASan report to source code locations using `llvm-symbolizer` on the cross-compile machine:

```bash
llvm-symbolizer --obj=out/linux-arm64-light-no-thread-no-ble-asan-clang/chip-lighting-app 0x30a86c 0x31d13c 0x230b2c
```

Example output:

```
operator new[](unsigned long)
../../../../../../../../../llvm-llvm-project/compiler-rt/lib/asan/asan_new_delete.cpp:111:37

main
/workspace/connectedhomeip/out/linux-arm64-light-no-thread-no-ble-asan-clang/../../examples/lighting-app/linux/main.cpp:129:18

_start
??:0:0
```

This output shows the exact file and line number where the memory leak occurred (e.g., `main.cpp:129`). The first address (`0x30a86c`) points to the `operator new[]` call, the second address (`0x31d13c`) points to the `main` function where the allocation occurs, and the third address (`0x230b2c`) points to the `_start` entry point.

:::{note} The `llvm-symbolizer` command must be run on the machine where the cross-compilation was performed, as it requires access to the build artifacts and debug symbols. :::
