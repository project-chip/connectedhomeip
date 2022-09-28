# CHIP Tests on QEMU

An application that runs CHIP's unit tests on QEMU.

---

- [CHIP Tests on QEMU](#chip-tests-on-qemu)
  - [Building the Docker Image](#building-the-docker-image)
    - [Run the Docker Image](#run-the-docker-image)
      - [Clone connectedhomeip repository](#clone-connectedhomeip-repository)
    - [Building the Application](#building-the-application)
    - [Run tests using qemu](#run-tests-using-qemu)

---

## Building the Docker Image

Building and testing the application requires the use of tizen SDK. The easiest
way is doing this in a dedicated Tizen docker container. In order to build tizen-qemu
docker image, use this command in the root of the project directory:

```sh
./integrations/docker/images/chip-build-tizen-qemu/build.sh
```

### Run the Docker Image

Best way is to do every next step inside a docker container. Run and go to
container you can use this command.

```sh
docker run -it --rm --name tizen-qemu \
    connectedhomeip/chip-build-tizen-qemu:latest /bin/bash
```

Optional you can mount your local repository in the container. This will allow you
to edit files on your host machine and build them inside the container. To do this
use this command instead of the previous one:

```sh
docker run -it --rm --name tizen-qemu -v "$PWD":/home/tizen/connectedhomeip \
    connectedhomeip/chip-build-tizen-qemu:latest /bin/bash
```

#### Clone connectedhomeip repository

You can skip this step if you use docker with mount volume.

```sh
git clone https://github.com/project-chip/connectedhomeip.git
```

### Building the Application

All these steps should be done inside the docker container.

Activate the environment:

```sh
cd connectedhomeip
source scripts/activate.sh
```

Build testing target application. Target list:
tizen-arm-qemu-tests- | no-ble | enable-wifi | use-asan

```sh
./scripts/build/build_examples.py --target tizen-arm-qemu-tests-no-ble --enable-flashbundle build
```

This test uses chip-tool, so you need build this target as well:

```sh
./scripts/build/build_examples.py --target tizen-arm-chip-tool-no-ble build
```

### Run tests using qemu

The default target in the runner script is set to tizen-arm-qemu-tests-no-ble. You
can use the `--target` flag to set other target for your test. Also, you can set
target for chip-tool using `--chip-tool-target`.

```sh
./src/test_driver/tizen/run_qemu_test.py --verbose
```

The `--verbose` flag is optional. It will print the output of the test to the
console.
