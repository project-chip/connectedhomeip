# Docker connectedhomeip:chip-build

project-chip/chip-build is the name of the Docker image used by CHIP for
continuous integration and other builds.

Contents of this directory:

-   build.sh - utility for building (and optionally) tagging and pushing the
    chip-build Docker image
-   version - the semver-style version of the image in use for this branch of
    CHIP
-   Dockerfile - description of the image

Select SDK OpenSSL linkage with a `docker build` argument. The same choices
apply to `chip-build-minimal` and `chip-cert-bins`:

| Build argument | SDK OpenSSL selection |
| --- | --- |
| Omitted (or empty) | Host OpenSSL (3.0 on Ubuntu 24.04) |
| `OPENSSL_STATIC=true` | Private OpenSSL 3.5, static linkage |
| `OPENSSL_STATIC=false` | Private OpenSSL 3.5, dynamic linkage |

An explicit value sets the SDK-only `CHIP_OPENSSL_STATIC` and
`CHIP_OPENSSL_ROOT` defaults. OpenSSL 3.5 is installed privately under
`/opt/matter/openssl`, selected through `CHIP_OPENSSL_ROOT` only by the SDK's
OpenSSL configuration. Unrelated tools built in this image or derived images
(such as Cirque) keep the system OpenSSL headers, pkg-config metadata and runtime
libraries. Dynamically linked SDK binaries use a private RUNPATH; the system
loader cache is unchanged. Cross-compilation sysroots are unaffected.
Rebuild consuming binaries when updating OpenSSL.
Host OpenSSL may lack algorithms needed by newer SDK features; choose private
OpenSSL explicitly when those features are required.

This applies to example applications and SDK tools built inside the image. For
example, in a bootstrapped SDK checkout inside the static OpenSSL image:

```sh
./scripts/build/build_examples.py --target linux-x64-network-manager build
```

Use a fresh output directory or rerun GN generation after changing images; an
existing Ninja build can retain the previous library selection. To request
static OpenSSL per build in a regular image, use the
`linux-x64-network-manager-openssl-static` target instead. See the
[SDK build instructions](../../../../../scripts/build/README.md#static-openssl-on-linux)
for other applications and direct GN builds.

Please update version when any required tooling is updated. Some rough
guidelines:

-   Updating a tool? Increment dot version unless the tool has a major version
    delta or a backward incompatibility
-   Adding a tool? Increment minor version: e.g. 0.2.1 -> 0.3.0
-   Removing a tool? Increment major version: e.g. 1.2 -> 2.0

Note, you must have privileged access to the connectedhomeip:chip-build on
Docker Hub

Typical use:

1.  new build tool dependency identified
2.  add tool to Dockerfile
3.  update version
4.  `$ ./build.sh`, which installs the image locally as the new version
5.  update the image version in the devcontainer.json
6.  verify that the build works locally in the new image
7.  `$ docker login`
8.  `$ ./build.sh --push --latest`, _*Note:*_ omit `--latest` unless on the
    master branch
