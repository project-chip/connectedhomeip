# Building Matter

Matter supports configuring the build with
[GN](https://gn.googlesource.com/gn/), a fast and scalable meta-build system
that generates inputs to [ninja](https://ninja-build.org/).

## Tested Operating Systems

The build system has been tested on the following Operating Systems:

-   macOS 10.15
-   Debian 11 (64 bit required)
-   Ubuntu 22.04 LTS

## Build system features

The Matter build system has the following features:

-   Very fast and small footprint
-   Cross-platform handling: Linux, Darwin, Embedded Arm, among others
-   Multiple toolchains & cross toolchain dependencies
-   Integrates automated testing framework: `ninja check`
-   Introspection: `gn desc`
-   Automatic formatting: `gn format`

## Checking out the Matter code

To check out the Matter repository, run the following command:

```
git clone --recurse-submodules git@github.com:project-chip/connectedhomeip.git
```

## Synchronizing submodules

If you already have the Matter code checked out, run the following command to
synchronize submodules:

```
git submodule update --init
```

## Prerequisites

Before building, you must install a few OS specific dependencies.

### Installing prerequisites on Linux

On Debian-based Linux distributions such as Ubuntu, these dependencies can be
satisfied with the following command:

```
sudo apt-get install git gcc g++ pkg-config libssl-dev libdbus-1-dev \
     libglib2.0-dev libavahi-client-dev ninja-build python3-venv python3-dev \
     python3-pip unzip libgirepository1.0-dev libcairo2-dev libreadline-dev
```

#### UI builds

If building `-with-ui` variant, also install SDL2:

```
sudo apt-get install libsdl2-dev
```

### Installing prerequisites on macOS

On macOS, install Xcode from the Mac App Store.

#### UI builds

If building `-with-ui` variant, also install SDL2:

```
brew install sdl2
```

### Installing prerequisites on Raspberry Pi 4

Complete the following steps:

1. Using `rpi-imager`, install the Ubuntu _22.04_ 64-bit _server_ OS for arm64
   architectures on a micro SD card.
1. Boot the SD card.
1. Log in with the default user account "ubuntu" and password "ubuntu"
1. Proceed with
   [Installing prerequisites on Linux](#installing-prerequisites-on-linux).
1. Install some Raspberry Pi specific dependencies:

    ```
    sudo apt-get install pi-bluetooth avahi-utils
    ```

1. Reboot your Raspberry Pi after installing `pi-bluetooth`.

#### Configuring wpa_supplicant for storing permanent changes

By default, wpa_supplicant is not allowed to update (overwrite) configuration.
If you want the Matter application to be able to store the configuration changes
permanently, you need to make the following changes:

1. Edit the `dbus-fi.w1.wpa_supplicant1.service` file to use configuration file
   instead by running the following command:

    ```
    sudo nano /etc/systemd/system/dbus-fi.w1.wpa_supplicant1.service
    ```

1. Run the following command to change the wpa_supplicant start parameters to
   the provided values:

    ```
    ExecStart=/sbin/wpa_supplicant -u -s -i wlan0 -c /etc/wpa_supplicant/wpa_supplicant.conf
    ```

1. Add the `wpa-supplicant` configuration file by running the following command:

    ```
    sudo nano /etc/wpa_supplicant/wpa_supplicant.conf
    ```

1. Add the following content to the `wpa-supplicant` file:

    ```
    ctrl_interface=DIR=/run/wpa_supplicant
    update_config=1
    ```

1. Reboot your Raspberry Pi.

## Installing ZAP tool

`bootstrap.sh` will download a compatible ZAP tool version and set it up in
`$PATH`. If you want to install or use a different version of the tool, you may
download one from the ZAP project's
[Releases](https://github.com/project-chip/zap/releases) page.

### Linux ARM

Zap does not provide binary releases for arm. Rosetta solves this for Darwin,
however for linux arm you will have to use a local ZAP, generally through
setting `$ZAP_DEVELOPMENT_PATH` (see the section `Which zap to use` below).

The file `scripts/setup/zap.json` contains the version that CIPD would download,
so you can download a compatible version from the zap project
[Releases](https://github.com/project-chip/zap/releases). To checkout as source
code the corresponding tag should exist in the zap
[repository tags](https://github.com/project-chip/zap/tags) list.

Example commands:

```sh
RUN set -x \
    && mkdir -p /opt/zap-${ZAP_VERSION} \
    && git clone https://github.com/project-chip/zap.git /opt/zap-${ZAP_VERSION} \
    && cd /opt/zap-${ZAP_VERSION} \
    && git checkout ${ZAP_VERSION} \
    && npm config set user 0 \
    && npm ci
ENV ZAP_DEVELOPMENT_PATH=/opt/zap-${ZAP_VERSION}
```

### Which ZAP to use

The ZAP tool scripting uses the following detection, in order of importance:

-   `$ZAP_DEVELOPMENT_PATH` to point to a ZAP checkout.

    -   Use this if you are developing ZAP locally and would like to run ZAP
        with your changes.

-   `$ZAP_INSTALL_PATH` to point to where `zap-linux.zip` or `zap-mac.zip` was
    unpacked.

    -   This allows you to not need to place `zap` or `zap-cli` (or both) in
        `$PATH`.

-   Otherwise, the scripts assume `zap-cli` or `zap` is available in `$PATH`.

## Prepare for building

Before running any other build command, the `scripts/activate.sh` environment
setup script should be sourced at the top level. This script takes care of
downloading GN, ninja, and setting up a Python environment with libraries used
to build and test.

Run the following command:

```
source scripts/activate.sh
```

### Updating the environment

If the script says the environment is out of date, you can update it by running
the following command:

```
source scripts/bootstrap.sh
```

The `scripts/bootstrap.sh` script re-creates the environment from scratch, which
is expensive, so avoid running it unless the environment is out of date.

## Build for the host OS (Linux or macOS)

Run the following commands to build all sources, libraries, and tests for the
host platform:

```
source scripts/activate.sh

gn gen out/host

ninja -C out/host
```

These commands generate a configuration suitable for debugging. To configure an
optimized build, specify `is_debug=false`:

```
gn gen out/host --args='is_debug=false'

ninja -C out/host
```

> **Note:** The directory name `out/host` can be any directory, although it's
> conventional to build within the `out` directory. This example uses `host` to
> emphasize building for the host system. Different build directories can be
> used for different configurations, or a single directory can be used and
> reconfigured as necessary via `gn args`.

To run all tests, run the following command:

```
ninja -C out/host check
```

To run only the tests in `src/inet/tests`, you can run the following command:

```
ninja -C out/host src/inet/tests:tests_run
```

> **Note:** The build system caches passing tests, so you may see the following
> message:
>
> ```
> ninja: no work to do
> ```
>
> This means that the tests passed in a previous build.

## Build custom configuration

The build is configured by setting build arguments. These you can set in one of
the following manners:

-   Passing the `--args` option to `gn gen`.
-   Running `gn args` on the output directory.
-   Editing `args.gn` in the output directory.

To configure a new build or edit the arguments to existing build, run the
following command:

```
source scripts/activate.sh

gn args out/custom

ninja -C out/custom
```

Two key built-in build arguments are `target_os` and `target_cpu`, which control
the OS and CPU of the build, respectively.

To see help for all available build arguments, run the following command:

```
gn gen out/custom
gn args --list out/custom
```

## Build examples

You can build examples in two ways.

### Build examples as separate projects

To build examples as separate projects that add Matter in the
`third_party directory`, run the following command with the correct path to the
example (here, `chip-shell`):

```
cd examples/shell
gn gen out/debug
ninja -C out/debug
```

### Build examples at the top level

You can build examples at the top level of the Matter project. See the following
"Unified builds" section for details.

## Unified builds

To build a unified configuration that approximates the set of continuous builds,
run the following commands:

```
source scripts/activate.sh

gn gen out/unified --args='is_debug=true target_os="all"'

ninja -C out/unified all
```

You can use this set of commands before changing a submission to configure,
build, and test the GCC, Clang, MbedTLS, and examples configurations all
together in one parallel build. Each configuration has a separate subdirectory
in the output directory.

This unified build can be used for day-to-day development, although it's more
expensive to build everything for every edit. To save time, you can name the
configuration to build:

```
ninja -C out/unified host_gcc
ninja -C out/unified check_host_gcc
```

Replace `host_gcc` with the name of the configuration, which is found in the
root `BUILD.gn`.

You can also fine tune the configurations generated with arguments. For example:

```
gn gen out/unified --args='is_debug=true target_os="all" enable_host_clang_build=false'
```

For a full list, see the root `BUILD.gn`.

In the unified build, targets have multiple instances and need to be
disambiguated by adding a `(toolchain)` suffix. Use `gn ls out/debug` to list
all of the target instances. For example:

```
gn desc out/unified '//src/controller(//build/toolchain/host:linux_x64_clang)'
```

> **Note:** Some platforms that can be built as part of the unified build
> require downloading additional tools. To add these to the build, the location
> must be provided as a build argument. For example, to add the Simplelink
> cc13x2_26x2 examples to the unified build, install
> [SysConfig](https://www.ti.com/tool/SYSCONFIG) and add the following build
> arguments:
>
> ```
> gn gen out/unified --args="target_os=\"all\" enable_ti_simplelink_builds=true > ti_sysconfig_root=\"/path/to/sysconfig\""
> ```

## Getting help

GN has integrated help that you can access with the `gn help` command.

Make sure to check the following recommended topics:

```
gn help execution
gn help grammar
gn help toolchain
```

Also see the
[quick start guide](https://gn.googlesource.com/gn/+/master/docs/quick_start.md).

## Introspection

GN has various introspection tools to help you examine the build configuration.
The following examples use the `out/host` output directory as example:

-   Show all of the targets in an output directory:

    ```
    gn ls out/host
    ```

-   Show all of the files that will be built:

    ```
    gn outputs out/host '*'
    ```

-   Show the GN representation of a configured target:

    ```
    gn desc out/host //src/inet --all
    ```

-   Dump the GN representation of the entire build as JSON:

    ```
    gn desc out/host/ '*' --all --format=json
    ```

-   Show the dependency tree:

    ```
    gn desc out/host //:all deps --tree --all
    ```

-   Find dependency paths:

    ```
    gn path out/host //src/transport/tests:tests //src/system
    ```

-   List useful information for linking against `libCHIP`:

    ```
    gn desc out/host //src/lib include_dirs
    gn desc out/host //src/lib defines
    gn desc out/host //src/lib outputs

    # everything as JSON
    gn desc out/host //src/lib --format=json
    ```

## Coverage

The code coverage script generates a report that details how much of the Matter
SDK source code has been executed. It also provides information on how often the
Matter SDK executes segments of the code and produces a copy of the source file,
annotated with execution frequencies.

Run the following command to initiate the script:

```
./scripts/build_coverage.sh
```

By default, the code coverage script is performed at the unit testing level.
Unit tests are created by developers, thus giving them the best overview of what
tests to include in unit testing. You can extend the coverage test by scope and
ways of execution with the following parameters:

```
  -c, --code                Specify which scope to collect coverage data.
                            'core': collect coverage data from core stack in Matter SDK. --default
                            'clusters': collect coverage data from clusters implementation in Matter SDK.
                            'all': collect coverage data from Matter SDK.
  -t, --tests               Specify which tools to run the coverage check.
                            'unit': Run unit test to drive the coverage check. --default
                            'yaml': Run yaml test to drive the coverage check.
                            'all': Run unit & yaml test to drive the coverage check.
```

Also, see the up-to-date unit testing coverage report of the Matter SDK
(collected daily) at:
[matter coverage](https://matter-build-automation.ue.r.appspot.com).

## Maintaining Matter

If you make any change to the GN build system, the next build will regenerate
the ninja files automatically. No need to do anything.
