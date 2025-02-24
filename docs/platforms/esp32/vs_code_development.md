# Visual Studio Code Development

[Visual Studio Code](https://code.visualstudio.com/) is a great and simple IDE
that can be used to build & develop with for Matter.

Matter supports the docker / remote container workflow in Visual Studio Code,
and has a container environment setup automatically. You can read more about
this workflow [here](https://code.visualstudio.com/docs/remote/containers).

Tested on:

-   Windows 11 Pro + WSL 2 + Ubuntu 22.04 LTS
-   Ubuntu 22.04 LTS

## Setup Steps

Follow
[these](https://github.com/project-chip/connectedhomeip/edit/master/docs/VSCODE_DEVELOPMENT.md)
steps to set up Visual Studio Code for Matter development.

(linux)=

### Linux

For Espressif esp32 devices:

1. Connect your device to the system before running the container.

1. Add `"--device=/dev/ttyUSB0",` inside runArgs in
   [devcontainer.json](../../../.devcontainer/devcontainer.json). Confirm the
   port of the device and change the parameter accordingly.

1. On opening connectedhomeip in Visual Studio code, At the bottom right of your
   window you should have a new box prompting you to re-open the window as a
   container. Hit yes.

1. The container will be built.

1. Install esp-idf : `cd /opt/espressif/esp-idf && ./install.sh`

1. Source esp-idf : `source /opt/espressif/esp-idf/export.sh`

1. Activate matter :
   `cd /workspaces/connectedhomeip && source scripts/bootstrap.sh && source scripts/activate.sh`

1. Confirm that the device is accessible : `ls -l /dev/ttyUSB*`

1. Go to lighting_example : `cd examples/lighting-app/esp32`

1. Set-target: `idf.py set-target esp32c3`

1. Build the project: `idf.py build`

1. Flash: `idf.py -p /dev/ttyUSB0 flash`

1. Monitor: `idf.py -p /dev/ttyUSB0 monitor`

### Windows

1. Connect to WSL Distro from VS Code.

1. Git clone inside WSL from Matter repository here:
   <https://github.com/project-chip/connectedhomeip>

1. Follow [these](https://github.com/dorssel/usbipd-win) steps on Windows and
   [these](https://github.com/espressif/vscode-esp-idf-extension/blob/master/docs/WSL.md#usbipd)
   steps in WSL to have serial port access inside WSL.

1. Confirm that the device is accessible inside WSL : `ls -l /dev/ttyUSB*`.

1. Open connectedhomeip within WSL on Visual Studio Code.

1. Perform step 2 onwards from {ref}`linux` guide.
