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

```
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

1. Compiles the Lock app on all supported platforms

    ```
    ./scripts/build/build_examples.py --app lock build
    ```

2. Compile the all clusters app for a ESP32 DevKitC

    ```
    ./scripts/build/build_examples.py --app all_clusters_app --board devkitc build
    ```

3. Generate all the build rules (but do not compile) all native apps

    ```
    ./scripts/build/build_examples.py --platform native generate
    ```

4. Generate all the makefiles (but do not compile) using a specific output root

    ```
    ./scripts/build/build_examples.py --platform native generate --out-prefix ./mydir
    ```

5. Compile the qpg lock app and copy the output in a 'artifact' folder. Note the
   argument order (artifact copying is an argument for the build command)

    ```
    ./scripts/build/build_examples.py --board qpg6100 --app lock build \
       --copy-artifacts-to /tmp/artifacts
    ```
