# MATTER CHEF APP

The purpose of the chef app is to to:

1. Increase the coverage of device types in Matter
2. Provide a sample application that may have its data model easily configured.

Chef uses the shell app a starting point, but processes the data model defined
on ZAP files during build time. This procedure is handled by its unified build
script: `chef.py`.

When processing ZAP files as part of the build process, Chef places the
auto-generated zap artifacts under its `out` temporary folder. Chef uses
artifacts from `zzz_generated` for CI/CD.

All device types available (.zap files) are found inside the `devices` folder.

## Building your first sample

1. Make sure you have the toolchain installed for your desired target.
2. Run `chef.py` the first time to create a `config.yaml` configuration file. If
   you already have SDK environment variables such as IDF_PATH (esp32) and
   ZEPHYR_BASE (nrfconnect) it will use those values as default.
3. Update your the SDK paths on `config.yaml`. TTY is the path used by the
   platform to enumerate its device as a serial port. Typical values are:

```
    # ESP32 macOS

    TTY: /dev/tty.usbmodemXXXXXXX

    # ESP32 Linux

    TTY: /dev/ttyACM0

    # NRFCONNECT macOS

    TTY: /dev/tty.usbserial-XXXXX

    # NRFCONNECT Linux

    TTY: /dev/ttyUSB0
```

4. Run `$ chef.py -u` to update zap and the toolchain (on selected platforms).
5. Run `$ chef.py -gzbf -t <platform> -d lighting`. This command will run the
   ZAP GUI opening the `devices/lighting.zap` file and will allow editing. It
   will then generate the zap artifacts, place them on the `zap-generated`
   folder, run a build and flash the binary in your target.
6. Run `chef.py -h` to see all available commands.

## Creating a new device type in your device library

Follow guide in [NEW_CHEF_DEVICES.md](NEW_CHEF_DEVICES.md).

## Folder Structure and Guidelines

-   `<platform>`: build system and `main.cpp` file for every supported platform.
    When porting a new platform, please minimize the source code in this folder,
    favoring the `common` folder for code that is not platform related.
-   `common`: contains code shared between different platforms. It may contain
    source code that enables specific features such as `LightingManager` class
    or `LockManager`, as long as the application dynamically identify the
    presence of the relevant cluster configurations and it doesn't break the use
    cases where chef is built without these clusters.
-   `devices`: contains the data models that may be used with chef. As of Matter
    1.0 the data models are defined using .zap files.
-   `out`: temporary folder used for placing ZAP generated artifacts.
-   `sample_app_util`: guidelines and scripts for generating file names for new
    device types committed to the `devices` folder.
-   `config.yaml`: contains general configuration for the `chef.py` script. As
    of Matter 1.0 this is used exclusively for toolchain and TTY interface
    paths.
-   `chef.py`: main script for generating samples. More info on its help
    `chef.py -h`.

## General Linux Options

When building chef for the Linux platform there are several options available at
runtime. These options are also available for many Linux samples. Do not
conflate these with chef options available at build time.

Ex.:

-   --discriminator <discriminator>: A 12-bit unsigned integer match the value
    which a device advertises during commissioning.
-   --passcode <passcode>: A 27-bit unsigned integer, which serves as proof of
    possession during commissioning. If not provided to compute a verifier, the
    --spake2p-verifier-base64 must be provided.
-   --secured-device-port <port>: A 16-bit unsigned integer specifying the
    listen port to use for secure device messages (default is 5540).
-   --KVS <filepath>: A file to store Key Value Store items.

For a full list, call the generated linux binary with

-   -h, --help: Print this output and then exit.

## CI

All CI jobs for chef can be found in `.github/workflows/chef.yaml`.

These jobs use a platform-specific image with base `chip-build`. Such images
contain the toolchain for the respective platform under `/opt`.

CI jobs call chef with the options `--ci -t $PLATFORM`. The `--ci` option will
execute builds for all devices specified in `ci_allow_list` defined in
`cicd_config.json` (so long as these devices are also in `/devices`) on the
specified platform.

CI jobs also call the function `bundle_$PLATFORM` at the end of each example
build. This function should copy or move build output files from the build
output location into `_CD_STAGING_DIR`. Typically, the set of files touched is
the minimal set of files needed to flash a device. See the function
`bundle_esp32` for reference.

### Adding a platform

First, implement a `bundle_$PLATFORM` function.

Next, ensure that the examples in `ci_allow_list` build in a container using the
relevant platform image. You can simulate the workflow locally by mounting your
CHIP repo into a container and executing the CI command:

```shell
docker run -it --mount source=$(pwd),target=/workspace,type=bind ghcr.io/project-chip/chip-build-$PLATFORM:$VERSION
```

In the container:

```shell
chown -R $(whoami) /workspace
cd /workspace
source ./scripts/bootstrap.sh
source ./scripts/activate.sh
./examples/chef/chef.py --ci -t $PLATFORM
```

Once you are confident the CI examples build and bundle in a container, add a
new job to the chef workflow.

Replace all instances of `$PLATFORM` with the new platform. Replace `$VERSION`
with the image version used in the rest of the workflows, or update the image
version for all images in the workflow as needed.

```yaml
chef_$PLATFORM:
    name: Chef - $PLATFORM CI Examples
    runs-on: ubuntu-latest
    if: github.actor != 'restyled-io[bot]'

    container:
        image: ghcr.io/project-chip/chip-build-$PLATFORM:$VERSION
        options: --user root

    steps:
        - name: Checkout
          uses: actions/checkout@v3
        - name: Checkout submodules & Bootstrap
          uses: ./.github/actions/checkout-submodules-and-bootstrap
          with:
              platform: $PLATFORM
        - name: CI Examples $PLATFORM
          shell: bash
          run: |
              ./scripts/run_in_build_env.sh "./examples/chef/chef.py --ci -t $PLATFORM"
```

## CD

Once CI is enabled for a platform, the platform may also be integrated into
`integrations/cloudbuild/`, where chef builds are defined in `chef.yaml`. See
the `README` in this path for more information.

Note that the image used in `chef.yaml` is `chip-build-vscode`. See
`docker/images/chip-build-vscode/Dockerfile` for the source of this image. This
image is a combination of the individual toolchain images. Therefore, before a
platform is integrated into chef CD, the toolchain should be copied into
`chip-build-vscode` and `chef.yaml` should be updated to use the new image
version.

Finally, add the new platform to `cd_platforms` in `cicd_config.json`. The
configuration should follow the following schema:

```json
"$PLATFORM": {
    "output_archive_prefix_1": ["option_1", "option_2"],
    "output_archive_prefix_2": [],
}
```

Take note of the configuration for `linux`:

```json
"linux": {
    "linux_x86": ["--cpu_type", "x64"],
    "linux_arm64_ipv6only": ["--cpu_type", "arm64", "--ipv6only"]
},
```

This will produce output archives prefixed `linux_x86` and
`linux_arm_64_ipv6only` and will append the respective options to each build
command for these targets.

To test your configuration locally, you may employ a similar strategy as in CI:

```shell
docker run -it --mount source=$(pwd),target=/workspace,type=bind ghcr.io/project-chip/chip-build-vscode:$VERSION
```

In the container:

```shell
chown -R $(whoami) /workspace
cd /workspace
source ./scripts/bootstrap.sh
source ./scripts/activate.sh
./examples/chef/chef.py --build_all --keep_going
```

You may also use the Google Cloud Build local builder as detailed in the
`README` of `integrations/cloudbuild/`.

## Adding new devices

To add new devices for chef:

-   Execute `python sample_app_util.py zap <zap_file> --rename-file` to rename
    the example and place the new file in `examples/chef/devices`.
    -   See the `README` in `examples/chef/sample_app_util/` for more info.
-   Execute `scripts/tools/zap_regen_all.py`, commit `zzz_generated` and
    `examples/chef/devices`.
    -   This is gated by the workflow in `.github/workflows/zap_templates.yaml`.
-   All devices added to the repository are built in CD.

## Manufacturer Extensions / Custom Clusters

You may add vendor-defined features to chef. The
`rootnode_onofflight_meisample*` device showcases its usage by using the Sample
MEI cluster which is defined on
`src/app/zap-templates/zcl/data-model/chip/sample-mei-cluster.xml`

This cluster has

-   One boolean attribute: `flip-flop`
-   A `ping` command with no arguments
-   A command/response pair `add-arguments`. The command takes two uint8
    arguments and the response command returns their sum.

You may test the `Sample MEI` via chip-tool using the following commands:

```
# commissioning of on-network chef device
chip-tool pairing onnetwork 1 20202021
# tests command to sum arguments: returns 30
chip-tool samplemei add-arguments 1 1 10 20
# sets Flip-Flop to false
chip-tool samplemei write flip-flop 0 1 1
# reads Flip-Flop
chip-tool samplemei read flip-flop 1 1
```
