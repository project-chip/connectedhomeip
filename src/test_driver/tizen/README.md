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

### Generate and run test target

As for now, Tizen QEMU-based test driver does not support BLE. In order to
disable BLE, one needs to pass `chip_config_network_layer_ble=false` to the args
argument of the `gn gen` command.

```sh
# Generate test target
gn gen --check --fail-on-unused-args \
    --root="$PWD/src/test_driver/tizen" \
    --args="target_os=\"tizen\" target_cpu=\"arm\" chip_config_network_layer_ble=false" \
    out/tizen-check
# Run Tizen QEMU-based tests
ninja -C out/tizen-check check
```
