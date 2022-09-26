# Building Matter

Matter supports configuring the build with
[GN](https://gn.googlesource.com/gn/), a fast and scalable meta-build system
that generates inputs to [ninja](https://ninja-build.org/).

Tested on:

-   macOS 10.15
-   Debian 11
-   Ubuntu 22.04 LTS

Build system features:

-   Very fast and small footprint
-   Cross-platform handling: (Linux, Darwin, embedded arm, etc.)
-   Multiple toolchains & cross toolchain dependencies
-   Integrates automated testing framework: `ninja check`
-   Introspection: `gn desc`
-   Automatic formatting: `gn format`

## Checking out the Matter code

To check out the Matter repository:

```
git clone --recurse-submodules git@github.com:project-chip/connectedhomeip.git
```

If you already have a checkout, run the following command to sync submodules:

```
git submodule update --init
```

## Prerequisites

Before building, you'll need to install a few OS specific dependencies.

### Installing prerequisites on Linux

On Debian-based Linux distributions such as Ubuntu, these dependencies can be
satisfied with the following:

```
sudo apt-get install git gcc g++ pkg-config libssl-dev libdbus-1-dev \
     libglib2.0-dev libavahi-client-dev ninja-build python3-venv python3-dev \
     python3-pip unzip libgirepository1.0-dev libcairo2-dev libreadline-dev
```

### Installing prerequisites on macOS

On macOS, first install Xcode from the Mac App Store. The remaining dependencies
can be installed and satisfied using [Brew](https://brew.sh/):

```
brew install openssl pkg-config
```

However, that does not expose the package to `pkg-config`. To fix that, one
needs to run something like the following:

Intel:

```
cd /usr/local/lib/pkgconfig
ln -s ../../Cellar/openssl@1.1/1.1.1g/lib/pkgconfig/* .
```

where `openssl@1.1/1.1.1g` may need to be replaced with the actual version of
OpenSSL installed by Brew.

Apple Silicon:

```
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:"/opt/homebrew/opt/openssl@3/lib/pkgconfig"
```

Note: If using MacPorts, `port install openssl` is sufficient to satisfy this
dependency.

### Installing prerequisites on Raspberry Pi 4

Using `rpi-imager`, install the Ubuntu _22.04_ 64-bit _server_ OS for arm64
architectures on a micro SD card.

Boot the SD card, login with the default user account "ubuntu" and password
"ubuntu", then proceed with
[Installing prerequisites on Linux](#installing-prerequisites-on-linux).

Finally, install some Raspberry Pi specific dependencies:

```
sudo apt-get install pi-bluetooth avahi-utils
```

You need to reboot your RPi after install `pi-bluetooth`.

By default, wpa_supplicant is not allowed to update (overwrite) configuration,
if you want the Matter app to be able to store the configuration changes
permanently, we need to make the following changes.

1. Edit the dbus-fi.w1.wpa_supplicant1.service file to use configuration file
   instead.

```
sudo nano /etc/systemd/system/dbus-fi.w1.wpa_supplicant1.service
```

Change the wpa_supplicant start parameters to:

```
ExecStart=/sbin/wpa_supplicant -u -s -i wlan0 -c /etc/wpa_supplicant/wpa_supplicant.conf
```

2. Add the wpa-supplicant configuration file

```
sudo nano /etc/wpa_supplicant/wpa_supplicant.conf
```

And add the following content to the file:

```
ctrl_interface=DIR=/run/wpa_supplicant
update_config=1
```

Finally, reboot your RPi.

## Prepare for building

Before running any other build command, the `scripts/activate.sh` environment
setup script should be sourced at the top level. This script takes care of
downloading GN, ninja, and setting up a Python environment with libraries used
to build and test.

```
source scripts/activate.sh
```

If this script says the environment is out of date, it can be updated by
running:

```
source scripts/bootstrap.sh
```

The `scripts/bootstrap.sh` script re-creates the environment from scratch, which
is expensive, so avoid running it unless the environment is out of date.

## Build for the host OS (Linux or macOS)

This will build all sources, libraries, and tests for the host platform:

```
source scripts/activate.sh

gn gen out/host

ninja -C out/host
```

This generates a configuration suitable for debugging. To configure an optimized
build, specify `is_debug=false`:

```
gn gen out/host --args='is_debug=false'

ninja -C out/host
```

The directory name `out/host` can be any directory, although it's conventional
to build within the `out` directory. This example uses `host` to emphasize that
we're building for the host system. Different build directories can be used for
different configurations, or a single directory can be used and reconfigured as
necessary via `gn args`.

To run all tests, run:

```
ninja -C out/host check
```

To run only the tests in `src/inet/tests`, you can run:

```
ninja -C out/host src/inet/tests:tests_run
```

Note that the build system caches passing tests, so if you see

```
ninja: no work to do
```

that means that the tests passed in a previous build.

## Build custom configuration

The build is configured by setting build arguments. These are set by passing the
`--args` option to `gn gen`, by running `gn args` on the output directory, or by
hand editing `args.gn` in the output directory. To configure a new build or edit
the arguments to existing build, run:

```
source scripts/activate.sh

gn args out/custom

ninja -C out/custom
```

Two key builtin build arguments are `target_os` and `target_cpu`, which control
the OS & CPU of the build.

To see help for all available build arguments:

```
gn gen out/custom
gn args --list out/custom
```

## Build examples

Examples can be built in two ways, as separate projects that add Matter in the
third_party directory, or in the top level Matter project.

To build the `chip-shell` example as a separate project:

```
cd examples/shell
gn gen out/debug
ninja -C out/debug
```

To build it at the top level, see below under "Unified Builds".

## Unified builds

To build a unified configuration that approximates the set of continuous builds:

```
source scripts/activate.sh

gn gen out/unified --args='is_debug=true target_os="all"'

ninja -C out/unified all
```

This can be used prior to change submission to configure, build, and test the
gcc, clang, mbedtls, & examples configurations all together in one parallel
build. Each configuration has a separate subdirectory in the output dir.

This unified build can be used for day to day development, although it's more
expensive to build everything for every edit. To save time, you can name the
configuration to build:

```
ninja -C out/unified host_gcc
ninja -C out/unified check_host_gcc
```

Replace `host_gcc` with the name of the configuration, which is found in the
root `BUILD.gn`.

You can also fine tune the configurations generated via arguments such as:

```
gn gen out/unified --args='is_debug=true target_os="all" enable_host_clang_build=false'
```

For a full list, see the root `BUILD.gn`.

Note that in the unified build, targets have multiple instances and need to be
disambiguated by adding a `(toolchain)` suffix. Use `gn ls out/debug` to list
all of the target instances. For example:

```
gn desc out/unified '//src/controller(//build/toolchain/host:linux_x64_clang)'
```

Note: Some platforms that can be built as part of the unified build require
downloading additional tools. To add these to the build, the location must be
provided as a build argument. For example, to add the Simplelink cc13x2_26x2
examples to the unified build, install
[SysConfig](https://www.ti.com/tool/SYSCONFIG) and add the following build
arguments:

```
gn gen out/unified --args="target_os=\"all\" enable_ti_simplelink_builds=true ti_sysconfig_root=\"/path/to/sysconfig\""
```

## Getting help

GN has builtin help via

```
gn help
```

Recommended topics:

```
gn help execution
gn help grammar
gn help toolchain
```

Also see the
[quick start guide](https://gn.googlesource.com/gn/+/master/docs/quick_start.md).

## Introspection

GN has various introspection tools to help examine the build configuration.

To show all of the targets in an output directory:

```
gn ls out/host
```

To show all of the files that will be built:

```
gn outputs out/host '*'
```

To show the GN representation of a configured target:

```
gn desc out/host //src/inet --all
```

To dump the GN representation of the entire build as JSON:

```
gn desc out/host/ '*' --all --format=json
```

To show the dependency tree:

```
gn desc out/host //:all deps --tree --all
```

To find dependency paths:

```
gn path out/host //src/transport/tests:tests //src/system
```

To list useful information for linking against libCHIP:

```
gn desc out/host //src/lib include_dirs
gn desc out/host //src/lib defines
gn desc out/host //src/lib outputs

# everything as JSON
gn desc out/host //src/lib --format=json
```

## Maintaining Matter

If you make any change to the GN build system, the next build will regenerate
the ninja files automatically. No need to do anything.
