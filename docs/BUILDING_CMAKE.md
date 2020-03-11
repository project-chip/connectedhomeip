## Build Documentation

The cmake build system in CHIP is used as follows:

### Building cmake

The version of cmake from apt-get (version 3.16.3) has issues handling
`ExternalProject_Add` commands to build autoconf packages in third_party/nl\*.
Therefore, to use cmake in CHIP, build a local copy of cmake from source:

```
  cd ~/tools
  wget https://cmake.org/files/v3.17/cmake-3.17.2.tar.gz
  tar xzf cmake-3.17.2.tar.gz
  cd cmake-3.17.2
  ./bootstrap
  make
  # Either add ~/tools/cmake-3.17.2/bin to PATH or
  make install
```

### Clone, configure, and setup

To setup the cmake/ninja environment, run the following:

```
  # Pull in proper branch
  git clone git@github.com:turon/connectedhomeip.git
  cd connectedhomeip

  # Pull in third_party repos
  git submodule update --init

  # Configure tree
  mkdir -p build/default
  cd build/default
  cmake -GNinja ../..
```

To setup a cmake/make environment, remove the `-GNinja` flag in the last
command:

```
  cmake ../..
```

### Configuration

The configuration of the build tree can be inspected and modified with the
following gui:

```
  ccmake ../..
```

#### Debug vs. Release build configurations

The build defaults to a Debug build. To explicitly set the build type pass the
following flag to cmake:

```
  cmake -GNinja ../.. -DCMAKE_BUILD_TYPE=Debug
  cmake -GNinja ../.. -DCMAKE_BUILD_TYPE=Release
```

NOTE: The -D option to cmake will respond to tab completion with available
options on many systems.

### Build operations

Details of the various build operations used in the performance and target
requirements sections are given here.

#### First configure

CMake can generate build files for multiple build systems:

| Build System | Command | | cmake / ninja | `cmake -GNinja ..` | | cmake / make
| `cmake ..` |

#### Basic build

| Build System | Command | | cmake / ninja | `ninja` | | cmake / make | `make` |

#### Run tests and build them if needed

| Build System | Command | | cmake / ninja | `ninja check` | | cmake / make |
`make check` |

#### Run pre-built tests

| Build System | Command | | cmake / ninja | `ninja test` | | cmake / make |
`make test` |

#### Build documentation

This command build the Doxygen documentation from the top-level.

| Build System | Command | | cmake / ninja | `ninja doc` | | cmake / make |
`make doc` |

#### Clean tree

This command cleans the tree from the top-level.

| Build System | Command | | cmake / ninja | `ninja clean` | | cmake / make |
`make clean` |

#### Coverage

This command generates a code coverage report on a tree that has built and run
all tests.

```
  cmake -GNinja -DBUILD_COVERAGE=1 ../..
  ninja coverage
```

#### Pretty and Pretty-check

This command detects any style violations:

```
  ninja pretty-check
```

This command fixes any style violations:

```
  ninja pretty
```

#### Building other platforms

To build other platforms, pass the CHIP_PLATFORM parameter.

```
  cmake -B build/nrf -GNinja -DCHIP_PLATFORM=nrf52840
  ninja -C build/nrf
```

Currently supported platforms are `standalone` (default), `linux`, and
`nrf52840`.
