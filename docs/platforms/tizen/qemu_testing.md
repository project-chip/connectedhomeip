# Testing on QEMU Emulator

Matter provides a QEMU-based testing environment for Tizen that allows running
unit tests and integration tests without physical hardware.

## Prerequisites

Enter the Tizen QEMU environment using the `tizen-qemu` Docker service (see
[Docker Compose Environment](./building.md#docker-compose-environment)):

```bash
docker compose run --rm tizen-qemu bash
source scripts/activate.sh
```

The `TIZEN_SDK_ROOT` environment variable must be set and point to a valid
Tizen SDK installation that contains the QEMU kernel and rootfs images:

- `$TIZEN_SDK_ROOT/iot-qemu-virt-zImage` – QEMU kernel image
- `$TIZEN_SDK_ROOT/iot-rootfs.img` – Root filesystem image
- `$TIZEN_SDK_ROOT/iot-sysdata.img` – System data image

Also install `qemu-system-arm` on your host:

```bash
sudo apt install qemu-system-arm
```

## Running Unit Tests

### Building the Test Target

Build the Tizen unit tests using `build_examples.py`:

```bash
./scripts/build/build_examples.py --target tizen-arm64-tests build
```

### Running Tests in QEMU

Use the `tizen_qemu.py` script to run the tests in the QEMU emulator:

```bash
python3 third_party/tizen/tizen_qemu.py \
    --share out/tizen-arm64-tests \
    --runner chip_tests/runner.sh
```

The script will:

1. Boot Tizen in QEMU
2. Mount the shared directory with test binaries
3. Execute the `runner.sh` script which discovers and runs all test executables
4. Report pass/fail status for each test

### Interactive QEMU Session

For debugging, you can launch an interactive QEMU session:

```bash
python3 third_party/tizen/tizen_qemu.py --interactive
```

This drops you into a root shell on the Tizen emulator.

## Running Integration Tests

### Building the Integration Test Target

Build the lighting app with TPK for integration testing:

```bash
./scripts/build/build_examples.py --target tizen-arm64-light-no-thread-no-ble --enable-flashbundle build
```

### Running Integration Tests in QEMU

```bash
python3 third_party/tizen/tizen_qemu.py \
    --share out/tizen-arm64-light-no-thread-no-ble \
    --runner integration_tests/lighting-app/runner.sh \
    --virtio-net
```

The `--virtio-net` flag enables network access in QEMU, which is required for
Matter commissioning tests.

The integration test runner will:

1. Install the lighting TPK on the emulated Tizen system
2. Launch the lighting application
3. Use `chip-tool` to pair and control the application
4. Verify on/off commands work correctly

## QEMU Options

The `tizen_qemu.py` script supports the following options:

| Option | Default | Description |
|--------|---------|-------------|
| `--interactive` | off | Run QEMU in interactive mode with root shell |
| `--smp NUM` | 2 | Number of CPUs available in QEMU |
| `--memory SIZE` | 512 | RAM size in MB assigned to QEMU |
| `--virtio-net` | off | Enable external network access via virtio-net |
| `--kernel PATH` | `$TIZEN_SDK_ROOT/iot-qemu-virt-zImage` | Path to kernel image |
| `--image-root IMAGE` | `$TIZEN_SDK_ROOT/iot-rootfs.img` | Path to root image |
| `--image-data IMAGE` | `$TIZEN_SDK_ROOT/iot-sysdata.img` | Path to system data image |
| `--share DIR` | none | Host directory to share with the guest |
| `--runner SCRIPT` | none | Runner script to execute after boot (relative to shared dir) |
| `--output FILE` | `/dev/null` | Store QEMU output in a file |

## Coverage Reports

To generate code coverage reports, build with the `--enable-coverage` flag:

```bash
./scripts/build/build_examples.py --target tizen-arm64-tests-coverage build
```

Then run the tests in QEMU. Coverage data (`.gcda` files) will be written to the
shared directory. The build system will generate an HTML coverage report in
`out/tizen-arm64-tests-coverage/coverage/html/`.
