# CHIP Shell - OpenThread CLI pass-through

The CHIP Shell CLI can execute pass-through commands to the
[OpenThread cli](https://github.com/openthread/openthread/blob/master/src/cli/README.md)
directly.

## Setup

### Embedded

On embedded platforms, the otcli commands are available simply when OpenThread
support is enabled.

### Linux

On embedded Linux platforms, otcli commands require installation of some
OpenThread daemons:

```
# Start Border Router agent
sudo /usr/local/sbin/otbr-agent -d6 -v -I wpan0 spinel+hdlc+forkpty:///usr/local/bin/ot-rcp\?forkpty-arg=5
```

If this command is not available, the follow instructions will build and install
it:

```
# Build OpenThread Interface (simulation used as example -- alternatively could be RCP hardware)
cd third_party/openthread/repo
./script/bootstrap
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local -DOT_PLATFORM=simulation -GNinja
ninja -j8 && sudo ninja install

# Build Border Router functionality
cd third_party/ot-br-posix/repo
./script/bootstrap
mkdir build && cd build
cmake .. -DOTBR_DBUS=ON -GNinja -DCMAKE_INSTALL_PREFIX=/usr/local -
ninja -j8 && sudo ninja install

# Start Border Router agent
sudo /usr/local/sbin/otbr-agent -d6 -v -I wpan0 spinel+hdlc+forkpty:///usr/local/bin/ot-rcp\?forkpty-arg=5

# In a new shell, at top-level of CHIP repo, test Thread device layer is operational
./bootstrap
mkdir build && cd build
../configure  --with-device-layer=linux
make -C src/platform/tests TestThreadStackMgr
sudo ./src/platform/tests/TestThreadStackMgr
```
