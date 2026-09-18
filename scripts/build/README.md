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

Append `-openssl-static` to a Linux target to link OpenSSL archives into the SDK
and application. This option applies to all Linux host applications, including
network-manager, all-clusters, chip-tool, and chip-cert:

```sh
./scripts/build/build_examples.py --target linux-x64-network-manager-openssl-static build
```

For direct GN builds, set `chip_crypto="openssl" chip_openssl_static=true`. The
default is `false` unless `CHIP_OPENSSL_STATIC=true` is set by a build image.
Other system libraries keep their normal linkage. Set `chip_openssl_root` to a
private installation prefix to select its matching headers and libraries for SDK
targets only. Both `lib/pkgconfig` and `lib64/pkgconfig` layouts are accepted.
Dynamic SDK consumers get a RUNPATH to that library directory. The static option
requires `libssl.a` and `libcrypto.a`; it does not download or upgrade OpenSSL.
Static OpenSSL symbols are hidden from shared system libraries, so dependencies
such as libcurl can continue using their system OpenSSL. When the SDK uses
OpenSSL, libdatachannel and its crypto dependencies use the same selected
installation and linkage mode as the SDK. When deploying dynamic binaries
outside the build image, also deploy the private shared libraries at the
configured path.

The `chip-build`, `chip-build-minimal`, and `chip-cert-bins` Dockerfiles also
accept `--build-arg OPENSSL_STATIC=true` for static OpenSSL 3.5 or
`--build-arg OPENSSL_STATIC=false` for dynamic OpenSSL 3.5. Omitting the
argument (or passing an empty value) selects host OpenSSL, which is 3.0 on
Ubuntu 24.04. The images install OpenSSL 3.5 under `/opt/matter/openssl`;
explicit selections set SDK-only `CHIP_OPENSSL_ROOT` and `CHIP_OPENSSL_STATIC`
defaults. The target suffix is optional inside those images. Ordinary pkg-config
queries, compiler include paths, the OpenSSL CLI, and the loader cache continue
to use system OpenSSL, including in derived images such as Cirque. The private
image installation is ignored for cross sysroots and other target architectures;
those builds select their own OpenSSL. Use a fresh output directory or rerun GN
generation when switching an existing build to a different image or OpenSSL
installation.

To override an image's default for an individual SDK build, set both variables
on the build command (and use a fresh output directory):

```sh
# Host OpenSSL
CHIP_OPENSSL_ROOT= CHIP_OPENSSL_STATIC=false ./scripts/build/build_examples.py --target linux-x64-network-manager build
# Private OpenSSL 3.5, dynamic
CHIP_OPENSSL_ROOT=/opt/matter/openssl CHIP_OPENSSL_STATIC=false ./scripts/build/build_examples.py --target linux-x64-network-manager build
# Private OpenSSL 3.5, static
CHIP_OPENSSL_ROOT=/opt/matter/openssl CHIP_OPENSSL_STATIC=true ./scripts/build/build_examples.py --target linux-x64-network-manager build
```

These defaults apply to SDK OpenSSL targets only. Unrelated tools built in later
Docker stages keep host OpenSSL regardless of the selected SDK mode.

GitHub's Linux Standalone workflow builds examples with both explicit OpenSSL
3.5 linkage modes and checks the dependencies of chip-cert and network-manager.
The Linux test suite selects dynamic linkage; REPL and nightly test-app builds
select static linkage because their binaries are transferred to other runners.
The `setup-sdk-openssl` action validates the private installation already built
into the image and exports only the SDK-specific selection variables. It fails
if the installation is missing or has the wrong version; it never downloads or
compiles OpenSSL. Publish the updated Docker image and update workflow image
references before enabling this action. The published standalone tools come from
the static job.

Static linking preserves the selected OpenSSL version; it does not add missing
algorithms. For example, network-manager's Network Identity Management
operations require deterministic ECDSA, which the OpenSSL backend implements for
OpenSSL 3.2 and newer. Building with the static suffix against OpenSSL 3.0 still
leaves those operations unsupported. The Dockerfiles above provide a private
OpenSSL 3.5 installation, which must be explicitly selected to use its newer
algorithms.
