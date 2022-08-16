# Setup VS Code environment

## Requirements

1. Install Visual Studio Code for your operating system of choice here:
   https://code.visualstudio.com/Download
2. Install [Git](https://git-scm.com/) if you haven't already
3. *Windows Only* Enable git to use LF instead of CLRF by default: `git config
   --global core.autocrlf false`
4. Git clone the Silicon Labs Matter repository here:
   https://github.com/SiliconLabs/matter
5. Launch Visual Studio Code, and open the cloned folder
6. When prompted to "install recommended extensions" please select "Install"
   * Recommended Extensions can be found [here](../../../../.vscode/extensions.json)
7. Ensure the following extensions are installed:
   1. C/C++ Extension Pack - language support for C/C++
   2. Python - language support for Python
   3. Cortex-Debug - GDB debugger support
8. ARM GCC Toolchain
   (https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads) -
   required arm-none-eabi tools
9. J-Link Software Tools - required J-Link GDB Server for J-Link based debuggers
   (https://www.segger.com/downloads/jlink)
10. Python - (https://www.python.org/downloads/)

## Bootstrapping your source tree (one time)

1. Under the "Terminal" menu (or using shortcut Ctrl+shift+p), select "Run
   Task..."
2. Select the "Bootstrap" task

-----

[Table of Contents](../../README.md)
