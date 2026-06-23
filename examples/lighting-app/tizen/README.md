# CHIP Tizen Lighting Example

This example runs on Tizen 10 on Raspberry Pi 4.

For full documentation including Docker Compose setup, VS Code extension
configuration, alternative build methods, and more, see the
[Tizen Platform Documentation](../../../docs/platforms/tizen/index.md).

## Building

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

For alternative build methods using GN and Ninja, see
[Building for Tizen – Alternative](../../../docs/platforms/tizen/building.md#alternative-building-with-gn-and-ninja).

## Installing TPK

The `sdb` tool is available inside the Tizen Docker container. To use `sdb`
from VS Code, install the
[Tizen Extension for VS Code](../../../docs/platforms/tizen/vscode_setup.md#tizen-extension-installation).

Install the TPK on the connected device:

```bash
sdb install out/tizen-arm64-light-no-thread/flashbundle/org.tizen.matter.example.lighting-1.0.0-arm64.tpk
```

## Launching Application

Launch the application on the device using `app_launcher`:

```bash
sdb shell app_launcher --start org.tizen.matter.example.lighting -- wifi true discriminator 1234 passcode 11223344
```

When passing user arguments via `app_launcher`, strings cannot start with `-`
(minus) character and all arguments must consist of a name and value. Boolean
options should have value equal to `"true"`.

For more details on testing TPK apps on Raspberry Pi, see
[Testing TPK Apps on Raspberry Pi](../../../docs/platforms/tizen/raspberry_pi_tpk.md).

## Debugging

For debugging with GDB CLI, see
[Debugging with GDB](../../../docs/platforms/tizen/debugging_gdb.md).

For VS Code graphical debugging and tasks, see
[VS Code Setup for Tizen Development](../../../docs/platforms/tizen/vscode_setup.md).
