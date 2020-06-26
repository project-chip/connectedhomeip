## Build Documentation

The cmake build system in CHIP is used as follows:

### Building cmake

The version of cmake from apt-get (version 3.16.3) can have issues with the
commands to build autoconf packages in third_party/nl\*. To use cmake in CHIP,
build a local copy of cmake from source:

```
  cd ~/tools
  wget https://cmake.org/files/v3.17/cmake-3.17.2.tar.gz
  tar xzf cmake-3.17.2
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
  make -f Makefile-bootstrap repos

  # Configure tree
  mkdir -p build/default
  cd build/default
  cmake -GNinja ../..
```

To setup a cmake/make environment, replace the last command with:

```
  cmake ../..
```

### Configuration

The configuration of the build tree can be inspected and modified with the
following gui:

```
  ccmake ../..
```

### Build operations

Details of the various build operations used in the performance and target
requirements sections are given here.

#### First configure and build

| Build System | Command | | autotools | (./bootstrap && ./configure && make) |
| cmake / ninja | (cmake -GNinja . && ninja) | | cmake / make | (cmake . &&
make) |

#### Full build from clean

| Build System | Command | autotools | make | | cmake / ninja | ninja | | cmake
/ make | make |

#### One line rebuild

This operation measures the build time on a completely built tree after changing
one line of code. The command is the same as 5.3.2 Full build from clean from
top of tree. This is a very common case in practical developer flows, and
provides a good litmus test on the dependency system within the build
environment. This operation is expected to be very fast, ideally operating only
on the file that has changed and generating all dependent artifacts. Any
evaluation of non-related portions of the tree are assessed as a distracting
waste of developer time and resources.

#### Zero change rebuild

This operation measures the build time on a completely built tree after running
the build command again with no changes. The command is the same as 5.3.2 Full
build from clean from top of tree. This operation is not uncommon in practical
developer flows. A developer may take a break, or come back to a project the
next business day and want to confirm their tree is built to the latest version.
Affirmation that no changes have taken place is important and should occur in
near zero time.

#### Build and run tests

In the case of autotools, this is equivalent of running `make check` for the
first time on a tree that has already been bootstrapped, configured, and cleaned
with `make clean`. This command is expected to be similar to 5.3.2 Full build
from clean but will additionally build additional test-specific executables and
run all such tests in an automated fashion.

| Build System | Command | | autotools | make check | | cmake / ninja | ninja
test | | cmake / make | make test |

#### Run tests

This operation is similar to 5.3.5 Build and run tests but is run after the
tests have all been built already and will simply retrigger the last step of
executing all the automated tests again.

#### Clean tree

This command cleans the tree from the top-level.

| Build System | Command | | autotools | make clean | | cmake / ninja | ninja
clean | | cmake / make | make clean |

#### Coverage

This command generates a code coverage report on a tree that has built and run
all tests.

```
  cmake -GNinja -DBUILD_COVERAGE=1 ../..
  ninja
  ninja test
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
  mkdir -p build/nrf5
  cd build/nrf5
  cmake -GNinja ../.. -DCHIP_PLATFORM=nrf52840
  ninja
```

Currently supported platforms are `standalone` (default), `linux`, and `nrf52840`.
