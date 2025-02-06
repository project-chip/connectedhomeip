# Using Python CHIP Controller advanced features

This document extends the
[basic documentation](python_chip_controller_building.md) of the Python CHIP
Controller with the useful information when developing Python CHIP Controller
tool or Matter accessories on Linux.

<hr>

-   [Using Python CHIP Controller advanced features](#using-python-chip-controller-advanced-features)
    -   [Bluetooth LE virtualization on Linux](#bluetooth-le-virtualization-on-linux)
    -   [Debugging with gdb](#debugging-with-gdb)

<hr>

## Bluetooth LE virtualization on Linux

Commissioning over Bluetooth LE can be tested even if the controller and the
device run on the same machine. To that end, you will need to set up two virtual
interfaces working as Bluetooth LE central and peripheral, respectively.

1. Build `bluez` project from sources by completing the following steps:

    ```
    sudo apt-get update
    sudo apt-get install libtool m4 automake autotools-dev libudev-dev libical-dev libreadline-dev

    git clone https://github.com/bluez/bluez.git

    cd bluez
    ./bootstrap
    ./configure --prefix=/usr --mandir=/usr/share/man --sysconfdir=/etc --localstatedir=/var --enable-experimental --with-systemdsystemunitdir=/lib/systemd/system --with-systemduserunitdir=/usr/lib/systemd --enable-deprecated --enable-testing --enable-tools
    make
    ```

2. Run bluetoothd:

    ```
    sudo ./src/bluetoothd --experimental --debug &
    ```

3. Bring up two virtual Bluetooth LE interfaces:

    ```
    sudo ./emulator/btvirt -L -l2
    ```

    You can find the virtual interface by running `hciconfig` command:

    ```
    $ hciconfig

    hci2:	Type: Primary  Bus: Virtual
       BD Address: 00:AA:01:01:00:24  ACL MTU: 192:1  SCO MTU: 0:0
       UP RUNNING
       RX bytes:0 acl:95 sco:0 events:205 errors:0
       TX bytes:2691 acl:95 sco:0 commands:98 errors:0

    hci1:	Type: Primary  Bus: Virtual
       BD Address: 00:AA:01:00:00:23  ACL MTU: 192:1  SCO MTU: 0:0
       UP RUNNING
       RX bytes:0 acl:95 sco:0 events:208 errors:0
       TX bytes:3488 acl:95 sco:0 commands:110 errors:0
    ```

4. Run the Python CHIP Controller REPL with Bluetooth LE adapter defined from a
   command line:

    For example, add `--ble-adapter=2` to use the virtual interface `hci2`
    listed above.

    ```
    chip-repl --ble-adapter=2
    ```

<hr>

## Debugging with gdb

You can run the chip-repl under GDB for debugging, however, since the Matter SDK
library is a dynamic library, you cannot read the symbols unless it is fully
loaded.

The following block is a example debug session using GDB:

```
# GDB cannot run scripts directly
# so you need to run Python3 with the path of device controller REPL
# Here, we use the feature from bash to get the path of chip-repl without typing it.
$ gdb --args python3 `which chip-repl`
GNU gdb (GDB) 14.2
Copyright (C) 2023 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.
Type "show copying" and "show warranty" for details.
This GDB was configured as "x86_64-pc-linux-gnu".
Type "show configuration" for configuration details.
For bug reporting instructions, please see:
<https://www.gnu.org/software/gdb/bugs/>.
Find the GDB manual and other documentation resources online at:
    <http://www.gnu.org/software/gdb/documentation/>.

For help, type "help".
Type "apropos word" to search for commands related to "word"...
Reading symbols from python3...

This GDB supports auto-downloading debuginfo from the following URLs:
  <https://debuginfod.archlinux.org>
Enable debuginfod for this session? (y or [n]) n
Debuginfod has been disabled.
To make this setting permanent, add 'set debuginfod enabled off' to .gdbinit.
(No debugging symbols found in python3)
(gdb)
```

The Python will create lots of threads due to main loop, so you may want to
suppress thread related outputs first by running the following command:

```
(gdb) set print thread-events off
```

We cannot set breakpoints here, since the GDB knows nothing about the Matter
library, let run the Matter device controller first.

```
(gdb) run
Starting program: /home/sag/projects/project-chip/connectedhomeip/out/venv/bin/python3 /home/sag/projects/project-chip/connectedhomeip/out/venv/bin/chip-repl
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/usr/lib/libthread_db.so.1".
Python 3.11.9 (main, Apr 29 2024, 11:59:58) [GCC 13.2.1 20240417]
Type 'copyright', 'credits' or 'license' for more information
IPython 8.24.0 -- An enhanced Interactive Python. Type '?' for help.
[1716395111.775747][364405:364405] CHIP:CTL: Setting attestation nonce to random value
[1716395111.776196][364405:364405] CHIP:CTL: Setting CSR nonce to random value
InitBLE 0[1716395111.776809][364405:364405] CHIP:DL: writing settings to file (/tmp/chip_counters.ini-T7hX27)
[1716395111.776854][364405:364405] CHIP:DL: renamed tmp file to file (/tmp/chip_counters.ini)
[1716395111.776860][364405:364405] CHIP:DL: NVS set: chip-counters/reboot-count = 9 (0x9)
[1716395111.777261][364405:364405] CHIP:DL: Got Ethernet interface: eno2
[1716395111.777555][364405:364405] CHIP:DL: Found the primary Ethernet interface:eno2
[1716395111.777868][364405:364405] CHIP:DL: Got WiFi interface: wlp7s0
[1716395111.777877][364405:364405] CHIP:DL: Failed to reset WiFi statistic counts
────────────────────────────────────────────────────────────────────────────────────────────────────────── Matter REPL ──────────────────────────────────────────────────────────────────────────────────────────────────────────



            Welcome to the Matter Python REPL!

            For help, please type matterhelp()

            To get more information on a particular object/class, you can pass
            that into matterhelp() as well.


─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────
2024-05-22 18:25:11 allenwind PersistentStorage[364405] WARNING Initializing persistent storage from file: /tmp/repl-storage.json
2024-05-22 18:25:11 allenwind PersistentStorage[364405] WARNING Loading configuration from /tmp/repl-storage.json...
2024-05-22 18:25:11 allenwind CertificateAuthorityManager[364405] WARNING Loading certificate authorities from storage...
2024-05-22 18:25:11 allenwind CertificateAuthority[364405] WARNING New CertificateAuthority at index 1
2024-05-22 18:25:11 allenwind CertificateAuthority[364405] WARNING Loading fabric admins from storage...
2024-05-22 18:25:11 allenwind FabricAdmin[364405] WARNING New FabricAdmin: FabricId: 0x0000000000000001, VendorId = 0xFFF1
2024-05-22 18:25:11 allenwind FabricAdmin[364405] WARNING Allocating new controller with CaIndex: 1, FabricId: 0x0000000000000001, NodeId: 0x000000000001B669, CatTags: []


The following objects have been created:
        certificateAuthorityManager:    Manages a list of CertificateAuthority instances.
        caList:                         The list of CertificateAuthority instances.
        caList:                 A specific FabricAdmin object at index m for the nth CertificateAuthority instance.


Default CHIP Device Controller (NodeId: 112233): has been initialized to manage caList[0].adminList[0] (FabricId = 1), and is available as devCtrl

In [1]:
```

The prompt `In [1]:` indicates that the Matter SDK library has been loaded and
initialized by the Python Controller REPL, you can browse the symbols in the
Matter core library, setting breakpoints on functions and many other functions
provided by GDB.

You can use `Ctrl-Z` to send `SIGTSTP` to the Python 3 REPL process anytime you
want so you can set breakpoints (unfortunately Ctrl+C seems to be captured by
the REPL).

In [1]: (`Ctrl-Z` pressed here.)

```
Thread 1 "python3" received signal SIGTSTP, Stopped (user).
0x00007ffff7650ceb in kill () from /usr/lib/libc.so.6
(gdb)
```

For example, you can break on `DeviceCommissioner::PairDevice` by using `break`
command in GDB (`b` for short)

```
(gdb) b DeviceCommissioner::PairDevice
Breakpoint 1 at 0x7fffed453943: DeviceCommissioner::PairDevice. (4 locations)
(gdb)
```

Type `signal SIGCONT` to continue the device controller after stopping it with
signal stop, you may need another hit of `Enter` to see the prompt.

```
(gdb) signal SIGCONT
Continuing with signal SIGCONT.
In [1]:
```

Let do pairing over IP to see the effect of the breakpoint we just set.

```
In [1]: devCtrl.CommissionWithCode("MT:-24J0AFN00KA0648G00", 1234)

Thread 5 "python3" hit Breakpoint 1.1, chip::Controller::DeviceCommissioner::PairDevice (this=0x7fffd8003a90, remoteDeviceId=1234, setUpCode=0x7ffff453d490 "MT:-24J0AFN00KA0648G00", params=...,
    discoveryType=chip::Controller::DiscoveryType::kAll, resolutionData=...) at ../../src/controller/CHIPDeviceController.cpp:646
646     {
(gdb)
```

You can find the `this` pointer, and value of arguments passed to this function,
then you can use `bt` (for `backtrace`) to see the backtrace of the call stack.

```
(gdb) bt
(gdb) bt
#0  chip::Controller::DeviceCommissioner::PairDevice
    (this=0x7fffd8003a90, remoteDeviceId=1234, setUpCode=0x7fffef2555d0 "MT:-24J0AFN00KA0648G00", params=..., discoveryType=chip::Controller::DiscoveryType::kAll, resolutionData=...)
    at ../../src/controller/CHIPDeviceController.cpp:646
#1  0x00007fffed040825 in pychip_DeviceController_ConnectWithCode (devCtrl=0x7fffd8003a90, onboardingPayload=0x7fffef2555d0 "MT:-24J0AFN00KA0648G00", nodeid=1234, discoveryType=2 '\002')
    at ../../src/controller/python/ChipDeviceController-ScriptBinding.cpp:395
#2  0x00007ffff6ad5596 in ??? () at /usr/lib/libffi.so.8
#3  0x00007ffff6ad200e in ??? () at /usr/lib/libffi.so.8
#4  0x00007ffff6ad4bd3 in ffi_call () at /usr/lib/libffi.so.8
#5  0x00007ffff6aeaffc in ??? () at /usr/lib/python3.11/lib-dynload/_ctypes.cpython-311-x86_64-linux-gnu.so
#6  0x00007ffff6aeb4b4 in ??? () at /usr/lib/python3.11/lib-dynload/_ctypes.cpython-311-x86_64-linux-gnu.so
#7  0x00007ffff794a618 in _PyObject_MakeTpCall () at /usr/lib/libpython3.11.so.1.0
#8  0x00007ffff78f3d03 in _PyEval_EvalFrameDefault () at /usr/lib/libpython3.11.so.1.0
#9  0x00007ffff7adef90 in ??? () at /usr/lib/libpython3.11.so.1.0
#10 0x00007ffff79ebc0b in _PyObject_FastCallDictTstate () at /usr/lib/libpython3.11.so.1.0
#11 0x00007ffff79ebe02 in _PyObject_Call_Prepend () at /usr/lib/libpython3.11.so.1.0
#12 0x00007ffff79ec114 in ??? () at /usr/lib/libpython3.11.so.1.0
#13 0x00007ffff794a618 in _PyObject_MakeTpCall () at /usr/lib/libpython3.11.so.1.0
#14 0x00007ffff78f3d03 in _PyEval_EvalFrameDefault () at /usr/lib/libpython3.11.so.1.0
#15 0x00007ffff7adef90 in ??? () at /usr/lib/libpython3.11.so.1.0
#16 0x00007ffff7955b97 in PyObject_Vectorcall () at /usr/lib/libpython3.11.so.1.0
#17 0x00007ffff6aea174 in ??? () at /usr/lib/python3.11/lib-dynload/_ctypes.cpython-311-x86_64-linux-gnu.so
#18 0x00007ffff6aea28c in ??? () at /usr/lib/python3.11/lib-dynload/_ctypes.cpython-311-x86_64-linux-gnu.so
#19 0x00007ffff6ad5152 in ??? () at /usr/lib/libffi.so.8
#20 0x00007ffff6ad57b8 in ??? () at /usr/lib/libffi.so.8
#21 0x00007fffed5de848 in chip::DeviceLayer::Internal::GenericPlatformManagerImpl<chip::DeviceLayer::PlatformManagerImpl>::_DispatchEvent
    (this=0x7fffed88dc90 <chip::DeviceLayer::PlatformManagerImpl::sInstance+16>, event=0x7fffe6fffe30) at ../../src/include/platform/internal/GenericPlatformManagerImpl.ipp:304
#22 0x00007fffed5dd90d in chip::DeviceLayer::PlatformManager::DispatchEvent (this=0x7fffed88dc80 <chip::DeviceLayer::PlatformManagerImpl::sInstance>, event=0x7fffe6fffe30) at ../../src/include/platform/PlatformManager.h:503
#23 0x00007fffed5df45b in chip::DeviceLayer::Internal::GenericPlatformManagerImpl_POSIX<chip::DeviceLayer::PlatformManagerImpl>::ProcessDeviceEvents
    (this=0x7fffed88dc90 <chip::DeviceLayer::PlatformManagerImpl::sInstance+16>) at ../../src/include/platform/internal/GenericPlatformManagerImpl_POSIX.ipp:185
#24 0x00007fffed5dee64 in chip::DeviceLayer::Internal::GenericPlatformManagerImpl_POSIX<chip::DeviceLayer::PlatformManagerImpl>::_RunEventLoop (this=0x7fffed88dc90 <chip::DeviceLayer::PlatformManagerImpl::sInstance+16>)
--Type <RET> for more, q to quit, c to continue without paging--
    at ../../src/include/platform/internal/GenericPlatformManagerImpl_POSIX.ipp:227
#25 0x00007fffed5dd888 in chip::DeviceLayer::PlatformManager::RunEventLoop (this=0x7fffed88dc80 <chip::DeviceLayer::PlatformManagerImpl::sInstance>) at ../../src/include/platform/PlatformManager.h:403
#26 0x00007fffed5df3fe in chip::DeviceLayer::Internal::GenericPlatformManagerImpl_POSIX<chip::DeviceLayer::PlatformManagerImpl>::EventLoopTaskMain (arg=0x7fffed88dc90 <chip::DeviceLayer::PlatformManagerImpl::sInstance+16>)
    at ../../src/include/platform/internal/GenericPlatformManagerImpl_POSIX.ipp:256
#27 0x00007ffff76a6ded in ??? () at /usr/lib/libc.so.6
#28 0x00007ffff772a0dc in ??? () at /usr/lib/libc.so.6
(gdb)
```

The frame #0 and frame #1 are the function frames in the CHIP C++ library, the
other frames live in the Python interpreter so you can ignore it.
