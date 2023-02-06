# CHIP Tests on QEMU

Tizen runs mostly on ARM architecture. In order to run tests on Tizen, we need
to use QEMU. This document describes how to build and run CHIP tests on QEMU.

## Obtaining Tizen QEMU Docker Image

All tools and dependencies required to build and run tests on Tizen on QEMU are
included in the `chip-build-tizen-qemu` docker image. One can pull the docker
image from hub.docker.com or build it locally using the provided Dockerfile in
`integrations/docker/images/chip-build-tizen-qemu` directory.

```sh
# Pull the image from hub.docker.com
docker pull connectedhomeip/chip-build-tizen-qemu:latest
```

## Building and Running Tests on QEMU

All steps described below should be done inside the docker container.

```sh
docker run -it --rm --name chip-tizen-qemu \
    connectedhomeip/chip-build-tizen-qemu:latest /bin/bash
```

### Clone the connectedhomeip repository

```sh
git clone https://github.com/project-chip/connectedhomeip.git
```

### Activate the environment

```sh
cd connectedhomeip
source scripts/activate.sh
```

### Build application variants that you would like to test

```sh
# Build chip-tool without BLE support (BLE is not supported on QEMU)
./scripts/build/build_examples.py --target tizen-arm-chip-tool-no-ble build
# Build lighting-app without BLE and WiFi support
./scripts/build/build_examples.py --target tizen-arm-light-no-ble-no-wifi \
    --enable-flashbundle build
```

### Generate and run test target

```sh
# Generate test target
gn gen --check --fail-on-unused-args -root="$PWD/src/test_driver/tizen" --args="
    target_os=\"tizen\" target_cpu=\"arm\"
    tizen_chip_tool=\"$PWD/out/tizen-arm-chip-tool-no-ble/chip-tool\"
    tizen_lighting_app_tpk=\"$PWD/out/tizen-arm-light-no-ble-no-wifi/package/out/org.tizen.matter.example.lighting-1.0.0.tpk\"
" out/tizen-check
# Run Time QEMU-based tests
ninja -C out/tizen-check check
```
