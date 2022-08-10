# Windows Support

short guide on using example build scripts on windows.

## Requirements

1. PowerShell (Version 5.0 and up preferred)
2. git

## Usage

run setup script.

```PowerShell
./scripts/windows_setup.ps1 --setup
```

when running an example build script, the arguments and their order is
important.

```PowerShell
./scripts/examples/gn_efr32_example.sh <AppRootFolder> <outputFolder> <efr32_board_name> [<Build options>]
```

for example:

```PowerShell
./scripts/examples/gn_efr32_example.ps1 /examples/lighting-app/efr32/ /out/lighting-app BRD4161A
```

To cleanup environment:

```PowerShell
./scrips/windows_setup.ps1 --cleanup
```
