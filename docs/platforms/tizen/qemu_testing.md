# Testing on QEMU Emulator

Matter provides a QEMU-based testing environment for Tizen that allows running
unit tests and integration tests without physical hardware.

The QEMU environment uses a 32-bit ARM image. Tests are built and executed
automatically by `build_examples.py`.

For more details on the QEMU test infrastructure, see the
[test driver README](../../../src/test_driver/tizen/README.md).

## Prerequisites

Enter the Tizen QEMU environment using the `tizen-qemu` Docker service (see
[Docker Compose Environment](./building.md#docker-compose-environment)):

```bash
docker compose run --rm tizen-qemu bash
source scripts/activate.sh
```

## Running Tests

Build and run the Tizen tests using `build_examples.py`. The `build` command
for the `tizen-arm-tests` target automatically compiles and runs both unit tests
and integration tests in QEMU:

```bash
./scripts/build/build_examples.py --target tizen-arm-tests-no-thread-no-ble build
```

The test runner will:

1. Boot Tizen in QEMU
2. Mount the build output directory with test binaries
3. Discover and run all unit test executables
4. Install the lighting TPK and run integration tests (pairing, on/off control)
5. Report pass/fail status for each test

:::{note}
The QEMU image is 32-bit ARM, so the `tizen-arm-tests` target must be used
(not `tizen-arm64-tests`).
:::

## Coverage Reports

To generate code coverage reports, build with the `--enable-coverage` flag:

```bash
./scripts/build/build_examples.py --target tizen-arm-tests-no-thread-no-ble-coverage build
```

Coverage data (`.gcda` files) will be collected during the test run. The build
system will generate an HTML coverage report in
`out/tizen-arm-tests-no-thread-no-ble-coverage/coverage/html/`.
