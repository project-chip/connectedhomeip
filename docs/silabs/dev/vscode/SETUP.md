# Set Up the VS Code environment

## General Requirements

1. Install Visual Studio Code for your operating system of choice here:
   https://code.visualstudio.com/Download
2. Install [Git](https://git-scm.com/) if you haven't already
3. _Windows Only_ Enable git to use LF instead of CLRF by default:
   `git config --global core.autocrlf false`
4. Git clone the Silicon Labs Matter repository here:
   https://github.com/SiliconLabs/matter
5. Launch Visual Studio Code, and open the cloned folder
6. When prompted to "install recommended extensions" please select "Install"
    - Recommended Extensions can be found
      [here](../../../../.vscode/extensions.json)
7. Ensure the following extensions are installed:
    1. C/C++ Extension Pack - language support for C/C++
    2. Python - language support for Python
    3. Cortex-Debug - GDB debugger support
8. ARM GCC Toolchain
   (https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads) -
   required arm-none-eabi tools
9. J-Link Software Tools - required J-Link GDB Server for J-Link-based debuggers
   (https://www.segger.com/downloads/jlink)
10. Python - (https://www.python.org/downloads/)

### Installing prerequisites on macOS

On macOS, you must install Xcode from the Mac App Store. The remaining
dependencies can be installed and satisfied using [Brew](https://brew.sh/):

> `$ brew install openssl pkg-config`

However, that does not expose the package to `pkg-config`. To fix that, run
something like the following:

Intel:

> `$ cd /usr/local/lib/pkgconfig ln -s ../../Cellar/openssl@1.1/1.1.1g/lib/pkgconfig/* .`

where `openssl@1.1/1.1.1g` may need to be replaced with the actual version of
OpenSSL installed by Brew.

Apple Silicon:

> `$ export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:"/opt/homebrew/opt/openssl@3/lib/pkgconfig"`

Note: If using MacPorts, `port install openssl` is sufficient to satisfy this
dependency.

### Mac OS (Apple Silicon) Users

These requirements are are needed for Mac OS (Apple Silicon) in addition to
those mentioned above. <br>

1. Add ARM GCC toolchain to the search path within `bootstrap.sh` by adding this
   line of code:

    > `$ export PATH="/Applications/ARM/bin:$PATH"`

2. Users may also have to specify which `pkg-config`/`openssl` to use by adding
   these lines of code as well:

    > `$ export PATH="/opt/homebrew/opt/openssl@3/bin:$PATH"`

    > `$ export PKG_CONFIG_PATH="/opt/homebrew/opt/openssl@3/lib/pkgconfig"`

### Installing prerequisites on Linux

On Debian-based Linux distributions such as Ubuntu, these dependencies can be
satisfied with the following:

> ` $ sudo apt-get install git gcc g++ pkg-config libssl-dev libdbus-1-dev libglib2.0-dev libavahi-client-dev ninja-build python3-venv python3-dev python3-pip unzip libgirepository1.0-dev libcairo2-dev libreadline-dev`

## Bootstrapping your source tree (one time)

1. Under the "Terminal" menu (or using shortcut Ctrl+shift+p), select "Run
   Task..."
2. Select the "Bootstrap" task

---

[Table of Contents](../../README.md)
