# Chef App Tests

Tests written using
[Python CHIP Controller ](https://github.com/project-chip/connectedhomeip/blob/master/src/controller/python/README.md)
used for gating chef device functionality.

The `chef_device_functional_test` job in
[chef.yaml](https://github.com/sxb427/connectedhomeip/blob/master/.github/workflows/chef.yaml)
shows all chef tests run in CI.

## Running Locally (Linux, Docker)

### Setup Docker

1. Install [Docker](https://www.docker.com/get-started/)
2. [Setup ipv6 for default bridge network](https://docs.docker.com/engine/daemon/ipv6/#use-ipv6-for-the-default-bridge-network)

### Run Tests In a Docker Container

1. Start a bash shell in a
   [chip-build-vscode](https://github.com/project-chip/connectedhomeip/pkgs/container/chip-build-vscode)
   container.

```
docker run -it --rm ghcr.io/project-chip/chip-build:181 /bin/bash
```

Run the remaining commands inside the container.

1. Clone repo (or a fork) -

```
git clone https://github.com/project-chip/connectedhomeip.git
```

3. Setup env -

```
cd connectedhomeip
git config --global --add safe.directory "*"
python scripts/checkout_submodules.py --shallow --recursive --platform linux
source ./scripts/bootstrap.sh
scripts/run_in_build_env.sh './scripts/build_python.sh --install_virtual_env out/python_env'
```

4. Build chef app -

```
./examples/chef/chef.py -b -d {app_name} -t linux
```

5. Run test file (fill in `app_name` and `test_file`) -

```
scripts/run_in_python_env.sh out/python_env './scripts/tests/run_python_test.py --app examples/chef/linux/out/{app_name} --factory-reset --script ./examples/chef/tests/{test_file}.py --script-args "--commissioning-method on-network --discriminator 3840 --passcode 20202021" --app-stdin-pipe /tmp/chef_stdin.txt'
```

## Libraries

-   [cluster objects](https://github.com/project-chip/connectedhomeip/blob/master/src/controller/python/matter/clusters/Objects.py)
-   [matter_testing_infrastructure](https://github.com/project-chip/connectedhomeip/tree/master/src/python_testing/matter_testing_infrastructure)
    Python test utilities
    -   [matter_testing.py](https://github.com/project-chip/connectedhomeip/blob/master/src/python_testing/matter_testing_infrastructure/matter/testing/matter_testing.py)
        MatterBaseTest base class for tests.
-   [Python Matter controller libraries](https://github.com/project-chip/connectedhomeip/tree/master/src/controller/python/matter)
    -   [Matter Device Controller interface](https://github.com/project-chip/connectedhomeip/blob/master/src/controller/python/matter/ChipDeviceCtrl.py)
-   PwRPC:
    -   [Protos](https://github.com/project-chip/connectedhomeip/tree/master/examples/common/pigweed/protos)
    -   [Device connection utils](https://github.com/google/pigweed/tree/1baea6bc0ee4afe955a834d17177d7be5418900a/pw_system/py/pw_system)
