# CHIP automated builds scripts

CHIP compilation is generally split into two steps

1. Generate ninja/makefile for out-of-source builds
2. Compilation using ninja/makefiles

## Building manually

Manual building is generally platform-dependent. All build steps would require a
bootstrapped environment (loads a pigweed build environment) and will then be
followed by platform-specific instructions.

The file BUILDING.md describes general requirements and examples. Typical usage
is:

```sh
source scripts/activate
gn gen out/host
ninja -C out/host
```

## Unified build script

The script `build_examples.py` provides a single entry point for generating and
executing the build.

Build environment _MUST_ be properly configured for build_examples to succeed.
For example ESP32 builds requite IDF_PATH to be set. Building in the
corresponding build image or the chip vscode image satisfy the build environment
requirement.

Usage examples:

1. Compile the all clusters app for a ESP32 DevKitC

    ```sh
    ./scripts/build/build_examples.py --target esp32-devkitc-all-clusters build
    ```

2. Generate all the makefiles (but do not compile) using a specific output root

    ```sh
    ./scripts/build/build_examples.py --target linux-x64-chip-tool --out-prefix ./mydir gen
    ```

3. Compile the qpg lock app and copy the output in a 'artifact' folder. Note the
   argument order (artifact copying is an argument for the build command)

    ```sh
    ./scripts/build/build_examples.py --target qpg-lock build --copy-artifacts-to /tmp/artifacts
    ```

4. Find out possible target choices:

    ```sh
    ./scripts/build/build_examples.py --log-level fatal targets
    ```

## Static OpenSSL on Linux

Native Linux builds using `chip_crypto="openssl"` can select an installation
with `CHIP_OPENSSL_ROOT=/opt/matter/openssl` and static linkage with
`CHIP_OPENSSL_STATIC=true`. The GN arguments `chip_openssl_root` and
`chip_openssl_static` override these defaults. Cross builds and other crypto
backends do not inherit the image's OpenSSL selection.

Static OpenSSL symbols are hidden from dynamic dependencies so system libraries
can keep using the system OpenSSL. Do not add the private installation to the
global `LD_LIBRARY_PATH` or loader configuration.

The `openssl_3_5_tests` CI job builds its own all-clusters application and
Python controller with WebRTC disabled. This job is needed until Ubuntu's system
OpenSSL is upgraded to 3.5 and can also host other tests requiring that version,
such as `TC_NETIM`. Its current test list contains only `TC_OPCREDS_3_9`,
`TC_DA_1_10` and `TC_DA_1_12`. The regular REPL jobs retain their default
OpenSSL configuration.
