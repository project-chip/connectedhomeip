# CHIP Tizen Lighting Example

This example runs on Tizen 10 on Raspberry Pi 4.

For full documentation including Docker Compose setup, VS Code extension
configuration, and more, see the
[Tizen Platform Documentation](../../../docs/platforms/tizen/index.md).

## Building

### Building with build_examples.py (Recommended)

The Docker Compose configuration with all required services (tizen, tizen-qemu,
crosscompile) is described in
[Building for Tizen](../../../docs/platforms/tizen/building.md).

Start and enter the Tizen compilation environment:

```bash
docker compose run --rm tizen bash
source scripts/activate.sh
```

Build the lighting app with TPK packaging:

```bash
./scripts/build/build_examples.py --target tizen-arm64-light-no-thread --enable-flashbundle build
```

The output will be in `out/tizen-arm64-light-no-thread/`.

### Building with GN and Ninja (Advanced)

For advanced use cases that require direct control over build arguments, you can
build manually using `gn gen` and `ninja`.

Activating environment:

```sh
source ./scripts/activate.sh
```

Generating build files for ARM:

```sh
gn gen --check \
    --fail-on-unused-args \
    --add-export-compile-commands=* \
    --root=$PW_PROJECT_ROOT/examples/lighting-app/tizen \
    "--args=target_os=\"tizen\" target_cpu=\"arm\" tizen_sdk_root=\"$TIZEN_SDK_ROOT\" tizen_sdk_sysroot=\"$TIZEN_SDK_SYSROOT\"" \
    $PW_PROJECT_ROOT/out/tizen-arm-light
```

Building:

```sh
ninja -C $PW_PROJECT_ROOT/out/tizen-arm-light
```

For ARM64 target, adjust the `target_cpu` argument:

```sh
gn gen --check \
    --fail-on-unused-args \
    --add-export-compile-commands=* \
    --root=$PW_PROJECT_ROOT/examples/lighting-app/tizen \
    "--args=target_os=\"tizen\" target_cpu=\"arm64\" tizen_sdk_root=\"$TIZEN_SDK_ROOT\" tizen_sdk_sysroot=\"$TIZEN_SDK_SYSROOT\"" \
    $PW_PROJECT_ROOT/out/tizen-arm64-light
```

## Preparing Tizen SDK Certificate (Optional)

When building Matter example application, this step is optional. In case when
author certificate and security profile are not found, they will be created
automatically. Automatically generated dummy certificate will have the following
options: name="Matter Example" email="matter@tizen.org" password="0123456789"

In order to create and use custom author certificate, one can use the commands
described below. Change the certificate password and author data as needed. The
security profile alias should not be changed, as the "CHIP" name is used in the
Matter example build system.

```sh
$TIZEN_SDK_ROOT/tools/ide/bin/tizen certificate \
    --alias=CHIP \
    --name=CHIP \
    --email=chip@tizen.org \
    --password=chiptizen

$TIZEN_SDK_ROOT/tools/ide/bin/tizen security-profiles add \
    --active \
    --name=CHIP \
    --author=$HOME/tizen-sdk-data/keystore/author/author.p12 \
    --password=chiptizen
```

This is only _one-time action_. To regenerate the author certificate and
security profile, you have to remove files from the `$HOME` directory using
specified commands:

```sh
rm -r \
    $HOME/tizen-sdk-data \
    $HOME/.tizen-cli-config \
    $HOME/.secretsdb
```

After that, normally call scripts to generate the author certificate and
security profile mentioned previously.

Please note, that regenerating the author certificate and security profile makes
it necessary to remove the previously installed Tizen app. You can't reinstall
an application on the Tizen device with a different certificate.

```sh
pkgcmd -u -n org.tizen.matter.example.lighting
```

## Packaging APP

```sh
ninja -C $PW_PROJECT_ROOT/out/tizen-arm-light chip-lighting-app:tpk
```

For ARM64:

```sh
ninja -C $PW_PROJECT_ROOT/out/tizen-arm64-light chip-lighting-app:tpk
```

## Installing TPK

Upload TPK package to device under test (DUT). Install it with `pkgcmd` as
follows:

```sh
pkgcmd -i -t tpk -p org.tizen.matter.example.lighting-1.0.0.tpk
```

Alternatively, you can use `sdb`:

```bash
sdb install out/tizen-arm64-light-no-thread/flashbundle/org.tizen.matter.example.lighting-1.0.0-arm64.tpk
```

## Launching Application

For launching Tizen application one should use `app_launcher`. It is possible to
pass user arguments from command line which might be used to control application
behavior. However, passed strings cannot start with "-" (minus) character and
all arguments have to consist of name and value. Boolean options (option without
argument) should have value equal to "true".

e.g.:

```sh
app_launcher --start=org.tizen.matter.example.lighting discriminator 43 wifi true
```

## Debugging

To debug app using vscode gdbserver has to be available on the target device,
run `./scripts/helpers/tizen_gdbserver_run.sh --help` for more information.

For more details on debugging with GDB CLI, see
[Debugging with GDB](../../../docs/platforms/tizen/debugging_gdb.md).

For VS Code graphical debugging and tasks, see
[VS Code Setup for Tizen Development](../../../docs/platforms/tizen/vscode_setup.md).
