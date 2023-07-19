# Matter Open IoT SDK Example Application

These examples are built using
[Open IoT SDK](https://gitlab.arm.com/iot/open-iot-sdk) and runs inside an
emulated target through the
[Arm FVP model for the Corstone-300 MPS3](https://developer.arm.com/downloads/-/arm-ecosystem-fvps).

The list of currently supported Matter examples:

```
shell
lock-app
tv-app
all-clusters-app
ota-requestor-app
```

You can use these examples as a reference for creating your own applications.

## Environment setup

The VSCode devcontainer has all the dependencies pre-installed. It is the
recommended way to build, run and develop with the Open IoT SDK port of the
Matter Project. Please read this
[VSCode development guide](../VSCODE_DEVELOPMENT.md) for more information.

Before building the examples, check out the Matter repository and sync Open IoT
SDK submodules using the following command:

```
scripts/checkout_submodules.py --shallow --recursive --platform openiotsdk
```

Next, bootstrap the source tree to install Pigweed (CIPD and Python packages)
components inside your environment (only once).

To bootstrap:

**using CLI**

```
$ bash scripts/bootstrap.sh
```

**using VSCode tasks**

-   Open the Command Palette: <kbd>F1</kbd>
-   Select `Tasks: Run Task`
-   Select `Bootstrap`

### Networking setup

Running ARM Fast Model with the TAP/TUN device networking mode requires the
setting up of proper network interfaces. Special scripts were designed to make
the setup easy. In the `scripts/setup/openiotsdk` directory you can find:

-   **network_setup.sh** - script to create the specific network namespace and
    Virtual Ethernet interface to connect with the host network. Both host and
    namespace sides have linked IP addresses. Inside the network namespace the
    TAP device interface is created and bridged with a Virtual Ethernet peer.
    There is also an option to enable an Internet connection in the namespace by
    forwarding traffic to the host default interface.

    To enable the Open IoT SDK networking environment:

    ```
    ${MATTER_ROOT}/scripts/setup/openiotsdk/network_setup.sh up
    ```

    To disable the Open IoT SDK networking environment:

    ```
    ${MATTER_ROOT}/scripts/setup/openiotsdk/network_setup.sh down
    ```

    To restart the Open IoT SDK networking environment:

    ```
    ${MATTER_ROOT}/scripts/setup/openiotsdk/network_setup.sh restart
    ```

    The default scripts settings are:

    -   `ARM` - network base name
    -   `current session user` - network namespace user
    -   `fe00::1` - host side IPv6 address
    -   `fe00::2` - namespace side IPv6 address
    -   `10.200.1.1` - host side IPv4 address
    -   `10.200.1.2` - namespace side IPv4 address
    -   no Internet connection support to network namespace

    Example of the `OIS` network environment settings:

    ```
    ARMns namespace configuration
    ARMbr: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
            inet 10.200.1.2  netmask 255.255.255.0  broadcast 0.0.0.0
            inet6 fe00::2  prefixlen 64  scopeid 0x0<global>
            inet6 fe80::1809:17ff:fe6c:f566  prefixlen 64  scopeid 0x20<link>
            ether 1a:09:17:6c:f5:66  txqueuelen 1000  (Ethernet)
            RX packets 1  bytes 72 (72.0 B)
            RX errors 0  dropped 0  overruns 0  frame 0
            TX packets 0  bytes 0 (0.0 B)
            TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

    ARMnveth: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
            ether 46:66:29:a6:91:4b  txqueuelen 1000  (Ethernet)
            RX packets 2  bytes 216 (216.0 B)
            RX errors 0  dropped 0  overruns 0  frame 0
            TX packets 3  bytes 270 (270.0 B)
            TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

    ARMtap: flags=4419<UP,BROADCAST,RUNNING,PROMISC,MULTICAST>  mtu 1500
            ether 1a:09:17:6c:f5:66  txqueuelen 1000  (Ethernet)
            RX packets 0  bytes 0 (0.0 B)
            RX errors 0  dropped 0  overruns 0  frame 0
            TX packets 0  bytes 0 (0.0 B)
            TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

    lo: flags=73<UP,LOOPBACK,RUNNING>  mtu 65536
            inet 127.0.0.1  netmask 255.0.0.0
            inet6 ::1  prefixlen 128  scopeid 0x10<host>
            loop  txqueuelen 1000  (Local Loopback)
            RX packets 0  bytes 0 (0.0 B)
            RX errors 0  dropped 0  overruns 0  frame 0
            TX packets 0  bytes 0 (0.0 B)
            TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

    Host configuration
    ARMhveth: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
            inet 10.200.1.1  netmask 255.255.255.0  broadcast 0.0.0.0
            inet6 fe80::147c:c9ff:fe4a:c6d2  prefixlen 64  scopeid 0x20<link>
            inet6 fe00::1  prefixlen 64  scopeid 0x0<global>
            ether 16:7c:c9:4a:c6:d2  txqueuelen 1000  (Ethernet)
            RX packets 3  bytes 270 (270.0 B)
            RX errors 0  dropped 0  overruns 0  frame 0
            TX packets 2  bytes 216 (216.0 B)
            TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
    ```

    Use `--help` to get more information about the script options.

    Open IoT SDK network setup is also supported via `VScode tasks`:

    -   Open the Command Palette: <kbd>F1</kbd>
    -   Select `Tasks: Run Task`
    -   Select `Setup Open IoT SDK network`
    -   Enter the network namespace name
    -   Choose command

    The VSCode task invokes `network_setup.sh` with the selected parameters.

-   **connect_if.sh** - script that connects specified network interfaces with
    the default route interface. It creates a bridge and links all interfaces to
    it. The bridge becomes the default interface.

    Example:

    ```
    ${MATTER_ROOT}/scripts/setup/openiotsdk/connect_if.sh ARMhveth
    ```

    Use `--help` to get more information about the script options.

Open IoT SDK network setup scripts contain commands that require root
permissions. Use `sudo` to run the scripts in a user account with root
privileges.

After setting up the Open IoT SDK network environment the user will be able to
run Matter examples on `FVP` in an isolated network namespace in TAP device
mode.

To execute a command in a specific network namespace use the helper script
`scripts/run_in_ns.sh`.

Example:

```
${MATTER_ROOT}/scripts/run_in_ns.sh ARMns <command to run>
```

Use `--help` to get more information about the script options.

> 💡 **Notes**:
>
> For Docker environment users it's recommended to use the
> [default bridge network](https://docs.docker.com/network/bridge/>#use-the-default-bridge-network)
> for a running container. This guarantees full isolation of the Open IoT SDK
> network from host settings.

### Debugging setup

Debugging the Matter application running on `FVP` model requires GDB Remote
Connection Plugin for Fast Model. More details
[GDBRemoteConnection](https://developer.arm.com/documentation/100964/1116/Plug-ins-for-Fast-Models/GDBRemoteConnection).

The `Fast Models FVP` add-on package can be downloaded from the ARM developer
website [Fast models](https://developer.arm.com/downloads/-/fast-models). After
login in to the `ARM developer` platform search for `Fast Models`, choose
`Fast Models (FM000A)` on the list of results, then choose the revision
`r11p16-16rel0` and download the
`Third Party Add-ons for Fast Models 11.16 (Linux)` package. Then unpack the
package in the selected location on the host machine.

Now you should add the GDB Remote Connection Plugin to your development
environment:

-   Linux host environment:

    -   install Fast Model Extension package by executing the command
        `./setup.bin`, and follow the installation instructions. After
        installation, the GDB Remote Connection Plugin should be visible in
        `<installation directory>/FastModelsPortfolio_11.16/plugins/Linux64_GCC-9.3`
        directory.
    -   add GDB plugin path to environment variable as
        `FAST_MODEL_PLUGINS_PATH`.

        Example:

        ```
        export FAST_MODEL_PLUGINS_PATH=<installation directory>/FastModelsPortfolio_11.16/plugins/Linux64_GCC-9.3
        ```

-   Docker container environment:

    -   pass the Fast Model Extension package to Docker container development
        environment by mounting it into the
        `/opt/FastModels_ThirdParty_IP_11-16_b16_Linux64` directory in the
        container. Add a volume bound to this directory
        [Add local file mount](https://code.visualstudio.com/remote/advancedcontainers/add-local-file-mount).

        You can edit the `.devcontainer/devcontainer.json` file, for example:

        ```
        ...
        "mounts": [ ...
        "source=/opt/FastModels_ThirdParty_IP_11-16_b16_Linux64,target=/opt/FastModels_ThirdParty_IP_11-16_b16_Linux64,type=bind,consistency=cached"
        ... ],
        ...
        ```

        Or if you launch the Docker container directly from CLI, use the above
        arguments with `docker run` command:

        ```
        docker run ... --mount type=bind,source=/opt/FastModels_ThirdParty_IP_11-16_b16_Linux64,target=/opt/FastModels_ThirdParty_IP_11-16_b16_Linux64 ...
        ```

    -   install the Fast Model Extension package via setup script inside Docker
        container:

        ```
        ${MATTER_ROOT}/scripts/setup/openiotsdk/debugging_setup.sh
        ```

    -   the GDB Remote Connection Plugin should be visible in
        `/opt/FastModelsPortfolio_11.16/plugins/Linux64_GCC-9.3` directory.
        -   For `VScode devcontainer` use the environment variable
            `FAST_MODEL_PLUGINS_PATH` to point to the correct directory.
        -   If the Docker container is directly launched remember to add the GDB
            Remote Connection Plugin path to the environment variable
            `FAST_MODEL_PLUGINS_PATH` inside the container:
            ```
            export FAST_MODEL_PLUGINS_PATH=/opt/FastModelsPortfolio_11.16/plugins/Linux64_GCC-9.3
            ```

### Testing setup

The Matter Python packages are required for the integration test suite. They are
not provided as part of the VSCode devcontainer. To install these run the
following command from the CLI:

```
${MATTER_ROOT}/scripts/run_in_build_env.sh \
    './scripts/build_python.sh --install_virtual_env out/venv'

source out/venv/bin/activate
```

More information about the Python tools you can find
[here](../../src/controller/python/README.md).

## Configuration

### Trusted Firmware-M

Matter examples support the [TF-M](https://tf-m-user-guide.trusted firmware.org)
by default.

This means the example is built as non-secure application in a Non-secure
Processing Environment (`NSPE`). The bootloader and the secure part are also
built from `TF-M` sources. All components are merged into a single executable
file at the end of the building process.

The project-specific configuration of `TF-M` can be provide by defining its own
header file for `TF-M` config and passing the path to it via the
`TFM_PROJECT_CONFIG_HEADER_FILE` variable.

```
set(TFM_PROJECT_CONFIG_HEADER_FILE "${CMAKE_CURRENT_SOURCE_DIR}/tf-m-config/TfmProjectConfig.h")
```

If the project-specific configuration is not provided the base `TF-M` settings
are used
[config_base.h](https://git.trustedfirmware.org/TF-M/trusted-firmware-m.git/tree/config/config_base.h).
It can be used as a pattern for the custom configuration header.

You can also provide your own version of a Matter example by setting the
`TFM_NS_APP_VERSION` variable.

```
set(TFM_NS_APP_VERSION "0.0.1")
```

### Storing persistent memory block in external files

The persistent storage is required to store key-value data of the Matter
examples.

Two storage types are supported:

-   Block device storage: The memory partition is located in `non-secure SRAM`
-   `TF-M` protected storage: The memory partition is located in
    `secure QSPI_RAM`

Fast models offers option to load and dump memory content. More details are
available
[here](./openiotsdk_platform_overview.md#fast-model-persistent-memory-via-files).
Depending on the storage implementation, different flags are used in the `FVP`
options.

For `TF-M` protected storage use:

```
--dump mps3_board.qspi_sram=<file-path>@0:0x660000,0x12000
--data mps3_board.qspi_sram=<file-path>@0:0x660000
```

> 💡 **Notes**:
>
> The `file-path` must exist to use the `--data` option.

[Open IoT SDK build script](../../scripts/examples/openiotsdk_example.sh)
provides the `-K,--kvsfile` option to use the persistence options listed above.

### Crypto backend

Open IoT SDK port supports two crypto backend implementations:

-   [Mbed TLS](../guides/openiotsdk_platform_overview.md#mbed-tls) - it's the
    default option
-   [PSA crypto service](https://tf-m-user-guide.trustedfirmware.org/integration_guide/services/tfm_crypto_integration_guide.html)
    from the
    [TrustedFirmware-M (TF-M)](../guides/openiotsdk_platform_overview.md#trusted-firmware-m)
    component

The CMake variable `CONFIG_CHIP_CRYPTO` controls how cryptographic operations
are implemented in Matter. It accepts two values:

-   `mbedtls`: use Mbed TLS for crypto operations.
-   `psa`: use
    [PSA Cryptography API](https://armmbed.github.io/mbed-crypto/html/) for
    crypto operations.

This variable can be set in the main application `CMakeLists.txt`:

```
set(CONFIG_CHIP_CRYPTO <mbedtls | psa>)
```

The variable can also be defined with CMake CLI:

```
cmake -G <...> -DCONFIG_CHIP_CRYPTO=<mbedtls | psa> <...>
```

> 💡 **Notes**:
>
> The `TF-M PSA crypto` option requires enabling [TF-M](#trusted-firmware-m)
> support.

### Device Firmware Update

Device Firmware Update (`DFU`) can be enabled in the application by setting the
`CONFIG_CHIP_OPEN_IOT_SDK_OTA_ENABLE` variable:

```
set(CONFIG_CHIP_OPEN_IOT_SDK_OTA_ENABLE YES)
```

This provides the proper service for Matter's `OTA Requestor` cluster. The
[TF-M Firmware Update Service](https://arm-software.github.io/psa-api/fwu/1.0/)
is the backend for all firmware update operations. The `DFU Manager` module is
attached to the application and allows full usage of the `OTA Requestor`
cluster.

You can also provide your own version of the Matter example to the Matter stack
by setting `CONFIG_CHIP_OPEN_IOT_SDK_SOFTWARE_VERSION` and
`CONFIG_CHIP_OPEN_IOT_SDK_SOFTWARE_VERSION_STRING` variables.

```
set(CONFIG_CHIP_OPEN_IOT_SDK_SOFTWARE_VERSION "1")
set(CONFIG_CHIP_OPEN_IOT_SDK_SOFTWARE_VERSION_STRING "0.0.1")
```

The default value for `CONFIG_CHIP_OPEN_IOT_SDK_SOFTWARE_VERSION_STRING` is set
to `TFM_NS_APP_VERSION`.

> 💡 **Notes**:
>
> The `DFU` option requires enabling [TF-M](#trusted-firmware-m) support.

## Building

You can build examples using the dedicated VSCode task or by calling directly
the build script from the command line.

### Building using the VSCode task

-   Open the Command Palette: <kbd>F1</kbd>
-   Select `Tasks: Run Task`
-   Select `Build Open IoT SDK example`
-   Decide on debug mode support
-   Decide on LwIP debug logs support
-   Choose crypto algorithm
-   Choose example name

This will call the script with the selected parameters.

### Building using CLI

You can call the script directly yourself.

```
${MATTER_ROOT}/scripts/examples/openiotsdk_example.sh <example name>
```

Use `--help` to get more information about the script options.

## Running

The application runs in the background and opens a telnet session. The telnet
client connects to the port used by the `FVP`. When the telnet process is
terminated it also terminates the `FVP` instance.

To exit the telnet session, type <kbd>CTRL + ]</kbd>. This changes the command
prompt to show as:

```
telnet>
```

Back in the terminal, type in the word 'close' to terminate the session.

```
telnet> close
```

You can run an example by using a VSCode task or by calling the run script
directly from the command line.

### Running using the VSCode task

-   Open the Command Palette: <kbd>F1</kbd>
-   Select `Tasks: Run Task`
-   Select `Run Open IoT SDK example`
-   Enter network namespace
-   Enter network interface
-   Choose example name

This will call the script with the selected example name.

### Running using CLI

You can call the script directly yourself.

```
${MATTER_ROOT}/scripts/examples/openiotsdk_example.sh -C run <example name>
```

Run example in specific network namespace with TAP device mode:

```
${MATTER_ROOT}/scripts/run_in_ns.sh ARMns ${MATTER_ROOT}/scripts/examples/openiotsdk_example.sh -C run -n ARMtap <example name>
```

## Testing

Run the Pytest integration test for the specific application.

The test result can be found in the
`src/test_driver/openiotsdk/integration-tests/<example name>/test_report.json`
file.

You can test an example by using a VSCode task or by calling the test script
directly from the command line.

### Testing using the VSCode task

-   Open the Command Palette: <kbd>F1</kbd>
-   Select `Tasks: Run Task`
-   Select `Test Open IoT SDK example`
-   Enter network namespace
-   Enter network interface
-   Choose example name

This will call the scripts with the selected example name.

### Testing using CLI

You can call the script directly yourself.

```
${MATTER_ROOT}/scripts/examples/openiotsdk_example.sh -C test <example name>
```

Testing an example in a specific network namespace with TAP device mode:

```
${MATTER_ROOT}/scripts/run_in_ns.sh ARMns ${MATTER_ROOT}/scripts/examples/openiotsdk_example.sh -C test -n ARMtap <example name>
```

## Debugging

Before debugging ensure the following:

1. The debug environment is correctly setup:
   [debugging setup](#debugging-setup).

2. The example is compiled with debug symbols enabled:

    For CLI:

    ```
    ${MATTER_ROOT}/scripts/examples/openiotsdk_example.sh -d true <example name>
    ```

    For the VSCode task:

    ```
    => Use debug mode (true)
    ```

3. The test network is correctly setup (if required): see
   [networking setup](#networking-setup).

### General instructions

-   Click `Run and Debug` from the primary side menu or press
    <kbd>Ctrl+Shift+D</kbd>
-   Select `Debug Open IoT SDK example application` from the drop down list
-   Click `Start Debugging`(green triangle) or press <kbd>F5</kbd>
-   Choose example name
-   Enter GDB target address
-   Enter network namespace
-   Enter network interface
-   Choose example name

As soon as a debugging session starts, the `DEBUG CONSOLE` panel is displayed
and shows the debugging output. Use debug controls to debug the current
application.

For debugging remote targets (i.e. run in other network namespaces) you need to
pass the hostname/IP address of the external GDB target that you want to connect
to (_GDB target address_).

In the case of using the [Open IoT SDK network environment](#networking-setup)
the GDB server runs inside a namespace and has the same IP address as the bridge
interface.

```
${MATTER_ROOT}/scripts/run_in_ns.sh <namespace_name> ifconfig <bridge_name>
```

The network namespace name and TAP interface name are also required then.

The application with GDB Remote Connection Plugin runs in the background and
opens a telnet session in terminal. The telnet client connects to the port used
by the `FVP`. When the telnet process is terminated it will also terminate the
`FVP` instance.

To exit the telnet session, type <kbd>CTRL + ]</kbd>. This changes the command
prompt to show as:

```
telnet>
```

Back in the terminal, type in the word 'close' to terminate the session.

```
telnet> close
```

> 💡 **Notes**:
>
> As you can see above, you will need to select the name of the example twice.
> This is because the debug task needs to launch the run task and currently VS
> code has no way of passing parameters between tasks.
>
> There are issues with debugging examples when the Docker container use the
> [network host](https://docs.docker.com/network/host/) and VPN connection is
> established. Changing routing negatively affects debugging process. It is
> recommended not to use VPN connections while debugging.

## Specific examples

### Build lock-app example with PSA crypto backend support and run it in the network namespace

**Using CLI**

```
${MATTER_ROOT}/scripts/examples/openiotsdk_example.sh -b psa lock-app

export TEST_NETWORK_NAME=OIStest

sudo ${MATTER_ROOT}/scripts/setup/openiotsdk/network_setup.sh -n $TEST_NETWORK_NAME restart

${MATTER_ROOT}/scripts/examples/scripts/run_in_ns.sh ${TEST_NETWORK_NAME}ns
${MATTER_ROOT}/scripts/examples/openiotsdk_example.sh -C run -n ${TEST_NETWORK_NAME}tap
lock-app
```

**Using the VSCode task**

Build example:

-   Open the Command Palette: <kbd>F1</kbd>
-   Select `Tasks: Run Task`
-   Select `Build Open IoT SDK example`
-   Deny debug mode support `false`
-   Deny LwIP debug logs support `false`
-   Choose crypto algorithm `psa`
-   Choose example name `lock-app`

Setup network environment:

-   Open the Command Palette: <kbd>F1</kbd>
-   Select `Tasks: Run Task`
-   Select `Setup Open IoT SDK network`
-   Enter the network namespace name `OIStest`
-   Choose command `restart`

Run example:

-   Open the Command Palette: <kbd>F1</kbd>
-   Select `Tasks: Run Task`
-   Select `Run Open IoT SDK example`
-   Enter network namespace `OIStestns`
-   Enter network interface `OIStesttap`
-   Choose example name `lock-app`

The example output should be seen in the terminal window.

### Build lock-app example with mbedtls crypto backend support and execute its test in the network namespace

**Using CLI**

```
${MATTER_ROOT}/scripts/examples/openiotsdk_example.sh -b mbedtls lock-app

export TEST_NETWORK_NAME=OIStest

sudo ${MATTER_ROOT}/scripts/setup/openiotsdk/network_setup.sh -n $TEST_NETWORK_NAME restart

${MATTER_ROOT}/scripts/examples/scripts/run_in_ns.sh ${TEST_NETWORK_NAME}ns
${MATTER_ROOT}/scripts/examples/openiotsdk_example.sh -C test -n ${TEST_NETWORK_NAME}tap
lock-app
```

**Using the VSCode task**

Build example:

-   Open the Command Palette: <kbd>F1</kbd>
-   Select `Tasks: Run Task`
-   Select `Build Open IoT SDK example`
-   Deny debug mode support `false`
-   Deny LwIP debug logs support `false`
-   Choose crypto algorithm `mbedtls`
-   Choose example name `lock-app`

Setup network environment:

-   Open the Command Palette: <kbd>F1</kbd>
-   Select `Tasks: Run Task`
-   Select `Setup Open IoT SDK network`
-   Enter the network namespace name `OIStest`
-   Choose command `restart`

Test example:

-   Open the Command Palette: <kbd>F1</kbd>
-   Select `Tasks: Run Task`
-   Select `Test Open IoT SDK example`
-   Enter network namespace `OIStestns`
-   Enter network interface `OIStesttap`
-   Choose example name `lock-app`

### Build lock-app example with mbedtls crypto backend support in debug mode and debug it in the network namespace using the VSCode task

Build example:

-   Open the Command Palette: <kbd>F1</kbd>
-   Select `Tasks: Run Task`
-   Select `Build Open IoT SDK example`
-   Confirm debug mode support `true`
-   Deny LwIP debug logs support `false`
-   Choose crypto algorithm `mbedtls`
-   Choose example name `lock-app`

Setup network environment:

-   Open the Command Palette: <kbd>F1</kbd>
-   Select `Tasks: Run Task`
-   Select `Setup Open IoT SDK network`
-   Enter the network namespace name `OIStest`
-   Choose command `restart`

Debug example:

-   Click `Run and Debug` from the primary side menu or press
    <kbd>Ctrl+Shift+D</kbd>
-   Select `Debug Open IoT SDK example application` from the drop down list
-   Click `Start Debugging`(green triangle) or press <kbd>F5</kbd>
-   Choose example name `lock-app`
-   Enter GDB target address `10.200.1.2`
-   Enter network namespace `OIStestns`
-   Enter network interface `OIStesttap`
-   Choose example name `lock-app`

Use debug controls to debug the application.

## Add new example

This chapter describes how to add a new Matter example based on Open IoT SDK
platform.

In the description below we use the placeholder `example_name` as the name of
the example to create. Replace it with the name of your example.

> 💡 **Notes**:
>
> Remember to update the list of currently supported Matter examples at the top
> of this document.

### Files structure

A new example should be put into `examples/<example_name>/openiotsdk` directory.
It should contain:

-   application source files and headers in the `main` sub-directory
-   application `CMakeLists.txt` file
-   `.gitignore` file with with all sources to skip
-   `README.md` file with example description
-   additional directories with required configuration for used components. Use
    `component_name-config` pattern, e.g `freertos-config`

### Target name

A new application target name should be created with
`chip-openiotsdk-<example_name>-example(_ns)` pattern. The `_ns` suffix is
required for [TF-M applications](#trusted-firmware-m).

Example:

```
set(APP_TARGET chip-openiotsdk-new-example-example_ns)
```

### Example tools

Add a new example name to the list in the
`examples/platform/openiotsdk/supported_examples.txt` file. After that the new
example is available in all necessary tools such as helper script
`scripts/examples/openiotsdk_example.sh` or VSCode tasks.

Example:

```
...
example_name
...
```

### CI

To add a new example to the Matter CI edit the
`.github/workflows/examples-openiotsdk.yaml` file and add the next step for
`openiotsdk` job step that build this example.

Example:

```
...
- name: Build new-example example
    id: build_new_example
    run: |
        scripts/examples/openiotsdk_example.sh -b ${{ matrix.cryptoBackend }} new-example
        .environment/pigweed-venv/bin/python3 scripts/tools/memory/gh_sizes.py \
        openiotsdk release new-example \
        examples/new-example/openiotsdk/build/chip-openiotsdk-new-example-example.elf \
        /tmp/bloat_reports/
...
```
