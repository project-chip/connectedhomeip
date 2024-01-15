# Building

-   docs/guides/BUILDING.md
-   Bootstrap to begin building
    -   will install all your tools etc.
    -   . scripts/bootstrap.sh - run this first, or if builds fail
    -   . scripts/activate.sh - faster if you’ve already bootstrapped
-   Build System - gn / ninja
    -   You can use a standard gn gen / ninja -C to build if you want
-   Script Options
    -   ./gn_build.sh - “all” build - host system tools + run unit tests
    -   scripts/build/build_examples.py
        -   scripts/build/build_examples.py targets
        -   scripts/build/build_examples.py --target <your target> build
        -   builds to out/<target_name>/
    -   scripts/build_python.sh -i <venv_name>
        -   use `source <venv_name>/bin/activate` to activate
        -   needed for python tests and chip-repl

# Example App + chip-tool

-   chip-all-clusters-app is the default _test_ app for QA / CI
    -   NOTE: THIS IS NOT A GOOD STARTING POINT FOR YOUR OWN DEVICE
    -   All clusters contains a lot of clusters for test and is not guaranteed
        to be spec compliant
-   chip-tool is the normal test controller
    -   docs/guides/chip_tool_guide.md
-   Testing Locally
    -   app and controller use a different different default port so you can run
        tests between app and controller in two terminals
-   chip-all-clusters-app
    -   use --help for command line options
    -   --discriminator, --passcode
    -   --KVS to specify where to store the app data (default is /tmp)
        -   “factory reset” by deleting the KVS file (rm /tmp/chip\_\* for
            default)
    -   --trace_decode 1 for verbose output, --trace_file, --trace_log
    -   --dac_provider, --vendor-id, --product-id to change the DAC

## Running an example

Build a sample app to run against and chip-tool. You can use the build_examples
script for this, selecting a target as appropriate
(`./scripts/build/build_examples.py targets` to see available targets)

    ./scripts/build/build_examples.py --target linux-x64-all-clusters build
    ./scripts/build/build_examples.py --target linux-x64-chip-tool

Start the example app from the target directory in one terminal.

    ./out/linux-x64-all-clusters/chip-all-clusters-app

The controller (chip-tool) is run from a second terminal. First thing we need to
do is to commission the device. You can get the code from the logs of the
example app. Both the QR and manual code work.

    cd ./out/linux-x64-chip-tool
    ./chip-tool pairing code 0x12344321 MT:-24J0AFN00KA0648G0

(here, pairing is the old name for commissioning, we just never changed the
tool) 0x12344321 is the node id we want to assign to the node. 0x12344321 is the
default for testing MT:-24J0AFN00KA0648G0 is the text string of the QR code.

Sending a command

    ./chip-tool onoff on 0x12344321 1

Sending a command with argument

    ./chip-tool unittesting test-simple-argument-request 1 0x12344321 0

Sending a command with a struct argument - use json

    ./chip-tool unittesting test-struct-argument-request '{"a":0, "b":false, "c":2, "d":"1234ef", "e":"char_string"}' 0x12344321 0

Reading an attribute

    ./chip-tool onoff read on-off 0x12344321 1
