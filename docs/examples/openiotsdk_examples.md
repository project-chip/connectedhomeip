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

Debugging Matter application running on `FVP` model requires GDB Remote
Connection Plugin for Fast Model. More details
[GDBRemoteConnection](https://developer.arm.com/documentation/100964/1116/Plug-ins-for-Fast-Models/GDBRemoteConnection).

The Third-Party IP add-on package can be downloaded from ARM developer website
[Fast models](https://developer.arm.com/downloads/-/fast-models). Currently
required version is `11.16`.

To install Fast Model Third-Party IP package:

-   unpack the installation package in a temporary location
-   execute the command `./setup.bin` (Linux) or `Setup.exe` (Windows), and
    follow the installation instructions.

After installation the GDB Remote Connection Plugin should be visible in
`FastModelsPortfolio_11.16/plugins` directory.

Then add the GDB plugin to your development environment:

-   host environment - add GDB plugin path to environment variable as
    FAST_MODEL_PLUGINS_PATH.

    Example

    ```
    export FAST_MODEL_PLUGINS_PATH=/opt/FastModelsPortfolio_11.16/plugins/Linux64_GCC-9.3
    ```

-   Docker container environment - mount the Fast Model Third-Party IP directory
    into the `/opt/FastModelsPortfolio_11.16` directory in container.

    The Vscode devcontainer users should add a volume bound to this directory
    [Add local file mount](https://code.visualstudio.com/remote/advancedcontainers/add-local-file-mount).

    You can edit the `.devcontainer/devcontainer.json` file, for example:

    ```
    ...
    "mounts": [
        ...
        "source=/opt/FastModelsPortfolio_11.16,target=/opt/FastModelsPortfolio_11.16,type=bind,consistency=cached"
        ...
    ],
    ...
    ```

    In this case, the FAST MODEL PLUGINS PATH environment variable is already
    created.

    If you launch the Docker container directly from CLI, use the above
    arguments with `docker run` command. Remember add GDB plugin path to
    environment variable as FAST_MODEL_PLUGINS_PATH inside container.

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
[docs/guides/openiotsdk_commissioning.md](/../guides/openiotsdk_commissioning.md)
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
