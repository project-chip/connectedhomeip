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

To check out the Matter code, there are two options: one is to check out all
platforms together, which is recommended; the other is to check out with support
for specific platforms, which can obviously reduce the project size.

### Checking out All Platforms

To check out the Matter repository with all platforms, run the following
command:

```
git clone --recurse-submodules git@github.com:project-chip/connectedhomeip.git

```

### Specific platforms Checking out

-   first step, checking out matter top level repo with command below:

```
  git clone --depth=1 git@github.com:project-chip/connectedhomeip.git

```

-   Second step, check out third-party platform support repos as follows:

```
  python3 scripts/checkout_submodules.py --shallow --platform platform1,platform2...

```

For Linux host example:

```
 ./scripts/checkout_submodules.py --shallow --platform  linux

```

For Darwin host example:

```
 ./scripts/checkout_submodules.py --shallow --platform  darwin

```

Please note that in the above commands, you should replace platform1,platform2
with the specific platform names you wish to check out.

## Updating Matter code

If you already have the Matter code checked out, run the following commands to
update the repository and synchronize submodules:

```
git pull
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
     python3-pip unzip libgirepository1.0-dev libcairo2-dev libreadline-dev \
     default-jre
```

#### UI builds

If building via `build_examples.py` and `-with-ui` variant, also install SDL2:

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

#### Enable experimental Bluetooth support and disable battery plugin in BlueZ

The Matter application on Linux uses BlueZ to communicate with the Bluetooth
controller. The BlueZ version that comes with Ubuntu 22.04 does not support all
the features required by the Matter application by default. To enable these
features, you need to enable experimental Bluetooth support in BlueZ.

Also disable the battery plugin from BlueZ, because iOS devices advertises a
battery service via BLE, which requires pairing if accessed. BlueZ includes a
battery plugin by default which tries to connect to the battery service. The
authentication fails, because in this case no BLE pairing has been done. If the
BlueZ battery plugin is not disabled, the BLE connection will be terminated
during the Matter commissioning process.

1. Edit the `bluetooth.service` unit by running the following command:

    ```sh
    sudo systemctl edit bluetooth.service
    ```

1. Add the following content to the override file:

    ```ini
    [Service]
    ExecStart=
    ExecStart=/usr/lib/bluetooth/bluetoothd -E -P battery
    ```

1. Restart the Bluetooth service by running the following command:

    ```sh
    sudo systemctl restart bluetooth.service
    ```

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

For platforms defined in [`scripts/setup/zap.json`](/scripts/setup/zap.json),
`bootstrap.sh` will download a compatible ZAP tool version from CIPD and set it
up in `$PATH`.

ZAP releases are copied to CIPD by an automated bot. You can check if a release
was copied by looking at tags created for
[ZAP CIPD Packages](https://chrome-infra-packages.appspot.com/p/fuchsia/third_party/zap)
in various platforms.

### Custom ZAP

If you want to install or use a different version of the tool, you may download
one from the [ZAP releases](https://github.com/project-chip/zap/releases) or
build it from source.

The file `scripts/setup/zap.json` contains the version that CIPD would download,
so you can refer to it to find a compatible version. The version is also
maintained at [`scripts/setup/zap.version`](/scripts/setup/zap.version).

To check out as source code, the corresponding tag should exist in the
[ZAP repository tags](https://github.com/project-chip/zap/tags) list.

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

-   `$ZAP_INSTALL_PATH` to point to where `zap-linux-x64.zip`,
    `zap-linux-arm64.zip` or `zap-mac-x64.zip` was unpacked.

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

## Using `build_examples.py`

The script `./scripts/build/build_examples.py` provides a uniform build
interface into using `gn`, `cmake`, `ninja` and other tools as needed to compile
various platforms.

Use `./scripts/build/build_examples.py targets` to see a list of supported
targets.

Example build commands:

```
# Compiles and runs all tests on the host:
./scripts/build/build_examples.py --target linux-x64-tests build

# Compiles fuzzing tagets using libfuzzer (fuzzing requires clang)
./scripts/build/build_examples.py --target linux-x64-tests-clang-asan-libfuzzer build

# Compiles a esp32 example
./scripts/build/build_examples.py --target esp32-m5stack-all-clusters build

# Compiles a nrf example
./scripts/build/build_examples.py --target nrf-nrf5340dk-pump build
```

### `libfuzzer` unit tests

`libfuzzer` unit tests tests are only compiled but not executed (you have to
manually execute them). For best error detection, some form of sanitizer like
`asan` should be used.

To compile, use:

```
./scripts/build/build_examples.py --target linux-x64-tests-clang-asan-libfuzzer build
```

After which tests should be located in
`out/linux-x64-tests-clang-asan-libfuzzer/tests/`.

#### `ossfuzz` configurations

`ossfuzz` configurations are not stand-alone fuzzing and instead serve as an
integration point with external fuzzing automated builds.

They pick up environment variables such as `$CFLAGS`, `$CXXFLAGS` and
`$LIB_FUZZING_ENGINE`.

You likely want `libfuzzer` + `asan` builds instead for local testing.

### `pw_fuzzer` `FuzzTests`

An Alternative way for writing and running Fuzz Tests is Google's `FuzzTest`
framework, integrated through `pw_fuzzer`. The Tests will have to be built and
executed manually.

```
./scripts/build/build_examples.py --target linux-x64-tests-clang-pw-fuzztest build
```

NOTE: `asan` is enabled by default in FuzzTest, so please do not add it in
build_examples.py invocation.

Tests will be located in:
`out/linux-x64-tests-clang-pw-fuzztest/chip_pw_fuzztest/tests/` where
`chip_pw_fuzztest` is the name of the toolchain used.

-   Details on How To Run Fuzz Tests in
    [Running FuzzTests](https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/fuzz_testing.md#running-fuzztests)

FAQ: In the event of a build failure related to missing files or dependencies
for pw_fuzzer, check the
[FuzzTest FAQ](https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/fuzz_testing.md#FAQ)

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
> cc13xx_26xx examples to the unified build, install
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

### How to Run

```
./scripts/build_coverage.sh [OPTIONS]
```

By default, the script

Builds the Matter SDK with coverage instrumentation (unless you specify a custom
--output_root). Runs the unit tests to generate coverage data. Produces an HTML
coverage report located at:

```
out/coverage/coverage/html/index.html
```

You can extend the coverage scope and test types with the following options:

Option Description -c, --code=<scope> Specify the scope to collect coverage
data. - core (default): Coverage from the core Matter SDK stack - clusters:
Coverage from cluster implementations - all: Coverage from the entire Matter SDK

--yaml Also run YAML-based tests, in addition to unit tests.

--python Also run Python-based tests, in addition to unit tests.

-o, --output_root=DIR If specified, skip the build phase and only run coverage
on the provided build output directory. This directory must have been built with
use_coverage=true and have had tests run already.

--target=<testname> When running unit tests, specifies a particular test target
to run (e.g., TestEmberAttributeBuffer.run).

-h, --help Print script usage and exit.

### Examples

Run coverage with the default scope (core) and only unit tests:

```
./scripts/build_coverage.sh
```

Run coverage including YAML tests (plus the always-enabled unit tests):

```
./scripts/build_coverage.sh --yaml
```

Run coverage including Python tests (plus the always-enabled unit tests):

```
./scripts/build_coverage.sh --python
```

Run coverage including both YAML and Python tests:

```
./scripts/build_coverage.sh --yaml --python
```

Change coverage scope to all (core + clusters) and run YAML tests:

```
./scripts/build_coverage.sh --code=all --yaml
```

### Viewing Coverage Results

After the script completes, open the following file in your web browser to view
the HTML coverage report:
[matter coverage](https://matter-build-automation.ue.r.appspot.com).

## Maintaining Matter

If you make any change to the GN build system, the next build will regenerate
the ninja files automatically. No need to do anything.
