## Build Documentation

The CHIP build system uses GNU autotools to build
various platform images on Linux or MacOS.

Tested on:
* MacOS
* Ubuntu 18.04


Build system features:
* Package management: 'make dist' and 'make distcheck'
* Cross-platform handling: (Linux, Darwin, iOS, Android, embedded arm, etc.)
* Multiple compiler support: clang, GCC
* Integrates automated testing framework: 'make check'
* Code style enforcement: 'make pretty' and 'make pretty-check' integration with clang-format

Assuming you have all the required tools installed, the system will build the code,
build a distribution, tidy, format, run tests, and run code coverage on the entire tree.  
Tests are built into the make system.


### Tool Prerequisites

To take advantage of all the current capabilities of the make system, you'll want:

* Bash 4.0 or greater
* GNU make, automake, autoconf, libtool
* C and C++ compilers
* clang-tidy
* clang-format-9
* gcov


#### How to install tool prerequisites on Linux

On Debian-based Linux distributions such as Ubuntu, these dependencies
can be satisfied with the following:

```
sudo apt-get install make autoconf automake libtool
sudo apt-get install clang-format-9
```

#### How to install tool prerequisites on macOS

On macOS, these dependencies can be installed and satisfied using
[Brew](https://brew.sh/):

```
brew install make autoconf automake libtool
brew install llvm@9
```

### Autotools Build Preparation

```
# Initial preparation
git clean -fdx
./bootstrap

make -f Makefile-Standalone
```

### Build Standalone (Native Linux or MacOS)

```
make -f Makefile-Standalone
```

### Build Custom configuration

```
# From top of clean tree
./bootstrap

mkdir out
cd out
../configure

# Build libraries
make

# Build distribution
make dist

# Build and check distribution
make distcheck

# Run tests
make check

# Verify coding style conformance
make pretty-check
```

### Build iOS

Install XCode and XQuarz.

```
make -f Makefile-iOS
```

### Build Android

Install Android Studio, Java, and NDK.

```
# Update these paths based on your environment and version of the tools (MacOS examples):
export JAVA_HOME=/Library/Java/JavaVirtualMachines/jdk-12.0.1.jdk/Contents/Home
export ANDROID_HOME=~/Library/Android/sdk
export ANDROID_NDK_HOME=~/Library/Android/sdk/ndk/21.0.6113669

make -f Makefile-Android
```

## Maintaining CHIP

If you want to maintain, enhance, extend, or otherwise modify CHIP, it
is likely you will need to change its build system, based on GNU
autotools, in some circumstances.

After any change to the CHIP build system, including any *Makefile.am*
files or the *configure.ac* file, you must run the `bootstrap` or
`bootstrap-configure` (which runs both `bootstrap` and `configure` in
one shot) script to update the build system.

### Dependencies

Due to its leverage of GNU autotools, if you want to modify or
otherwise maintain the CHIP build system, the following
additional packages are required and are invoked by `bootstrap`:

  * autoconf
  * automake
  * libtool

Instructions for installing these tools is in the Tool Prerequisites section above.
