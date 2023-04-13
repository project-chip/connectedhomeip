# Matter Open IoT SDK Example Application

These examples are built using
[Open IoT SDK](https://gitlab.arm.com/iot/open-iot-sdk) and runs inside an
emulated target through the
[Arm FVP model for the Corstone-300 MPS3](https://developer.arm.com/downloads/-/arm-ecosystem-fvps).

You can use these example as a reference for creating your own applications.

## Environment setup

Before building the examples, check out the Matter repository and sync
submodules using the following command:

```
$ git submodule update --init
```

The VSCode devcontainer has all dependencies pre-installed. Using the VSCode
devcontainer is the recommended way to interact with Open IoT SDK port of the
Matter Project. Please read this [README.md](../VSCODE_DEVELOPMENT.md) for more
information.

### Networking setup

Running ARM Fast Model with TAP/TUN device networking mode requires setup proper
network interfaces. Special scripts were designed to make setup easy. In
`scripts/setup/openiotsdk` directory you can find:

-   **network_setup.sh** - script to create the specific network namespace and
    Virtual Ethernet interface to connect with host network. Both host and
    namespace sides have linked IP addresses. Inside the network namespace the
    TAP device interface is created and bridged with Virtual Ethernet peer.
    There is also option to enable Internet connection in namespace by
    forwarding traffic to host default interface.

    To enable Open IoT SDK networking environment:

    ```
    ${MATTER_ROOT}/scripts/setup/openiotsdk/network_setup.sh up
    ```

    To disable Open IoT SDK networking environment:

    ```
    ${MATTER_ROOT}/scripts/setup/openiotsdk/network_setup.sh down
    ```

    Use `--help` to get more information about the script options.

-   **connect_if.sh** - script that connects specified network interfaces with
    the default route interface. It creates a bridge and links all interfaces to
    it. The bridge becomes the default interface.

    Example:

    ```
    ${MATTER_ROOT}/scripts/setup/openiotsdk/connect_if.sh ARMhveth
    ```

    Use `--help` to get more information about the script options.

Open IoT SDK network setup scripts contain commands that require root
permissions. Use `sudo` to run the scripts in user account with root privileges.

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

**NOTE**

For Docker environment users it's recommended to use the
[default bridge network](https://docs.docker.com/network/bridge/#use-the-default-bridge-network)
for a running container. This guarantees full isolation of the Open IoT SDK
network from host settings.

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

## Configuration

### Trusted Firmware-M

To add [TF-M](https://tf-m-user-guide.trustedfirmware.org) support to Matter
example you need to set `TFM_SUPPORT` variable inside main application
`CMakeLists.txt` file.

```
set(TFM_SUPPORT YES)
```

This causes the Matter example to be built as non-secure application in
Non-secure Processing Environment (`NSPE`). The bootloader and the secure part
are also built from `TF-M` sources. All components are merged into a single
executable file at the end of the building process.

You can also provide the own version of Matter example by setting
`TFM_NS_APP_VERSION` variable.

```
set(TFM_NS_APP_VERSION "0.0.1")
```

### Trusted Firmware-M Protected Storage

By default, the
[Block Device storage](../guides/openiotsdk_platform_overview.md#storage) is
used for storing Matter key-value data.

There is an option to add
[TF-M Protected Storage Service](https://tf-m-user-guide.trustedfirmware.org/integration_guide/services/tfm_ps_integration_guide.html)
support for `key-value` storage component in the Matter examples. Set the
variable `CONFIG_CHIP_OPEN_IOT_SDK_USE_PSA_PS` to `YES` to add
`TF-M Protected Storage` support to you application. You can put it inside the
main application `CMakeLists.txt` file:

```
set(CONFIG_CHIP_OPEN_IOT_SDK_USE_PSA_PS YES)
```

or add as a Cmake command-line parameter:

```
cmake -G <...> -DCONFIG_CHIP_OPEN_IOT_SDK_USE_PSA_PS=YES <...>
```

This option causes `key-value` objects will be stored in a secure part of flash
memory and the Protected Storage Service takes care of their encryption and
authentication.

**NOTE**

The `TF-M Protected Storage` option requires enabling
[TF-M](#trusted-firmware-m) support.

The `-k/--kvsstore` option in
[Open IoT SDK build script](../../scripts/examples/openiotsdk_example.sh)
selects key-value storage implementation for the Matter's examples. It
demonstrates how to use the `CONFIG_CHIP_OPEN_IOT_SDK_USE_PSA_PS` variable.

### Storing persistent memory block in external files

The persistent storage is required to store key-value data of the Matter
examples.

Two storage types are supported:

-   Block device storage: The memory partition is located in `non-secure SRAM`
-   `TF-M` protected storage: The memory partition is located in
    `secure QSPI_RAM`

Fast models offers option to load and dump memory content. More details are
available
[here](../guides/openiotsdk_platform_overview.md#fast-model-persistent-memory-via-files).
Depending on the storage implementation, different flags are used in the `FVP`
options.

For block device storage use:

```
--dump mps3_board.sram=<file-path>@0:0x0,0x100000
--data mps3_board.sram=<file-path>@0:0x0
```

For `TF-M` protected storage use:

```
--dump mps3_board.qspi_sram=<file-path>@0:0x660000,0x12000
--data mps3_board.qspi_sram=<file-path>@0:0x660000
```

_Note_: The `file-path` must exist to use the `--data` option.

[Open IoT SDK build script](../../scripts/examples/openiotsdk_example.sh)
provides the `-K,--kvsfile` option to use the persistence options listed above.

## Building

You build using a vscode task or call the script directly from the command line.

### Building using vscode task

```
Command Palette (F1) => Run Task... => Build Open IoT SDK example => (debug on/off) => <example name>
```

This will call the scripts with the selected parameters.

### Building using CLI

You can call the script directly yourself.

```
${MATTER_ROOT}/scripts/examples/openiotsdk_example.sh <example name>
```

Use `--help` to get more information about the script options.

## Running

The application runs in the background and opens a telnet session. The script
will open telnet for you and connect to the port used by the `FVP`. When the
telnet process is terminated it will also terminate the `FVP` instance.

You can run the application script from a vscode task or call the script
directly.

### Running using vscode task

```
Command Palette (F1) => Run Task... => Run Open IoT SDK example => (network namespace) => (network interface) => <example name>
```

This will call the scripts with the selected example name.

### Running using CLI

You can call the script directly yourself.

```
${MATTER_ROOT}/scripts/examples/openiotsdk_example.sh -C run <example name>
```

Run example in specific network namespace with TAP device mode:

```
${MATTER_ROOT}/scripts/run_in_ns.sh ARMns ${MATTER_ROOT}/scripts/examples/openiotsdk_example.sh -C run -n ARMtap <example name>
```

### Commissioning

Once booted the application can be commissioned, please refer to
[docs/guides/openiotsdk_commissioning.md](../guides/openiotsdk_commissioning.md)
for further instructions.

## Testing

Run the Pytest integration test for specific application.

The test result can be found in
`src/test_driver/openiotsdk/integration-tests/<example name>/test_report.json`
file.

You run testing using a vscode task or call the script directly from the command
line.

### Testing using vscode task

```
Command Palette (F1) => Run Task... => Test Open IoT SDK example => (network namespace) => (network interface) => <example name>
```

This will call the scripts with the selected example name.

### Testing using CLI

You can call the script directly yourself.

```
${MATTER_ROOT}/scripts/examples/openiotsdk_example.sh -C test <example name>
```

Test example in specific network namespace with TAP device mode:

```
${MATTER_ROOT}/scripts/run_in_ns.sh ARMns ${MATTER_ROOT}/scripts/examples/openiotsdk_example.sh -C test -n ARMtap <example name>
```

## Debugging

Debugging can be started using a VS code launch task:

```
Run and Debug (Ctrl+Shift+D) => Debug Open IoT SDK example application => Start
Debugging (F5) => <example name> => (GDB target address) => (network namespace) => (network interface) => <example name>
```

For debugging remote targets (i.e. run in other network namespaces) you need to
pass hostname/IP address of external GDB target that you want to connect to
(_GDB target address_). In case of using the
[Open IoT SDK network environment](#networking-setup) the GDB server runs inside
a namespace and has the same IP address as bridge interface.

```
${MATTER_ROOT}/scripts/run_in_ns.sh <namespace_name> ifconfig <bridge_name>
```

**NOTE**

As you can see above, you will need to select the name of the example twice.
This is because the debug task needs to launch the run task and currently VS
code has no way of passing parameters between tasks.
