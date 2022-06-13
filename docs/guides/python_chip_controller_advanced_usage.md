# Using Python CHIP Controller advanced features

This document extends the
[basic documentation](python_chip_controller_building.md) of the Python CHIP
Controller with the useful information when developing Python CHIP Controller
tool or Matter accessories on Linux.

<hr>

-   [Bluetooth LE virtualization on Linux](#virtualization)
-   [Debugging with gdb](#gdb)

<hr>

<a name="virtualization"></a>

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

4. Run the Python CHIP Controller with Bluetooth LE adapter defined from a
   command line:

    For example, add `--bluetooth-adapter=hci2` to use the virtual interface
    `hci2` listed above.

    ```
    chip-device-ctrl --bluetooth-adapter=hci2
    ```

<hr>

<a name="gdb"></a>

## Debugging with gdb

You can run the chip-device-ctrl under GDB for debugging, however, since the
Matter core support library is a dynamic library, you cannot read the symbols
unless it is fully loaded.

The following block is a example debug session using GDB:

```
# GDB cannot run scripts directly
# so you need to run Python3 with the path of device controller
# Here, we use the feature from bash to get the path of chip-device-ctrl without typing it.
$ gdb --args python3 `which chip-device-ctrl`
GNU gdb (Ubuntu 10.1-2ubuntu2) 10.1.90.20210411-git
Copyright (C) 2021 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.
Type "show copying" and "show warranty" for details.
This GDB was configured as "aarch64-linux-gnu".
Type "show configuration" for configuration details.
For bug reporting instructions, please see:
<https://www.gnu.org/software/gdb/bugs/>.
Find the GDB manual and other documentation resources online at:
    <http://www.gnu.org/software/gdb/documentation/>.

For help, type "help".
Type "apropos word" to search for commands related to "word"...
Reading symbols from python3...
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
Starting program: /usr/bin/python3 /home/ubuntu/.local/bin/chip-device-ctrl
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/lib/aarch64-linux-gnu/libthread_db.so.1".
CHIP:DIS: Init admin pairing table with server storage.
CHIP:IN: local node id is 0x000000000001b669
CHIP:DL: MDNS failed to join multicast group on wpan0 for address type IPv4: Inet Error 1016 (0x000003F8): Address not found
CHIP:ZCL: Using ZAP configuration...
CHIP:ZCL: deactivate report event
CHIP:CTL: Getting operational keys
CHIP:CTL: Generating operational certificate for the controller
CHIP:CTL: Getting root certificate for the controller from the issuer
CHIP:CTL: Generating credentials
CHIP:CTL: Loaded credentials successfully
CHIP:DL: Platform main loop started.
Chip Device Controller Shell

chip-device-ctrl >
```

The prompt `chip-device-ctrl >` indicates that the Matter core library is loaded
by Python, you can browse the symbols in the Matter core library, setting
breakpoints on functions and many other functions provided by GDB.

You can use `Ctrl-C` to send SIGINT to the controller anytime you want so you
can set breakpoints.

> (`Ctrl-C` pressed here.)

```
Thread 1 "python3" received signal SIGINT, Interrupt.
0x0000fffff7db79ec in __GI___select (nfds=<optimized out>, readfds=0xffffffffe760, writefds=0x0, exceptfds=0x0, timeout=<optimized out>) at ../sysdeps/unix/sysv/linux/select.c:49
49	../sysdeps/unix/sysv/linux/select.c: No such file or directory.
(gdb)
```

For example, you can break on `DeviceCommissioner::PairDevice` by using `break`
command in GDB (`b` for short)

```
(gdb) b DeviceCommissioner::PairDevice
Breakpoint 1 at 0xfffff5b0f6b4 (2 locations)
(gdb)
```

Type `continue` (`c` for short) to continue the device controller, you may need
another hit of `Enter` to see the prompt.

```
(gdb) c
Continuing.

chip-device-ctrl >
```

Let do pairing over IP to see the effect of the breakpoint we just set.

```
chip-device-ctrl > connect -ip 192.168.50.5 20202021 1
Device is assigned with nodeid = 1

Thread 1 "python3" hit Breakpoint 1, 0x0000fffff5b0f6b4 in chip::Controller::DeviceCommissioner::PairDevice(unsigned long, chip::RendezvousParameters&)@plt ()
   from /home/ubuntu/.local/lib/python3.9/site-packages/chip/_ChipDeviceCtrl.so
(gdb)
```

The `@plt` symbol means it is a symbol used by dynamic library loader, type `c`
(for `continue`) and it will break on the real function.

```
(gdb) c
Continuing.

Thread 1 "python3" hit Breakpoint 1, chip::Controller::DeviceCommissioner::PairDevice (this=0xd28540, remoteDeviceId=1, params=...) at ../../src/controller/CHIPDeviceController.cpp:827
827	{
(gdb)
```

You can find the `this` pointer, and value of arguments passed to this function,
then you can use `bt` (for `backtrace`) to see the backtrace of the call stack.

```
(gdb) bt
#0  chip::Controller::DeviceCommissioner::PairDevice(unsigned long, chip::RendezvousParameters&) (this=0xd28540, remoteDeviceId=1, params=...)
    at ../../src/controller/CHIPDeviceController.cpp:827
#1  0x0000fffff5b3095c in pychip_DeviceController_ConnectIP(chip::Controller::DeviceCommissioner*, char const*, uint32_t, chip::NodeId)
    (devCtrl=0xd28540, peerAddrStr=0xfffff467ace0 "192.168.50.5", setupPINCode=20202021, nodeid=1) at ../../src/controller/python/ChipDeviceController-ScriptBinding.cpp:234
#2  0x0000fffff7639148 in  () at /lib/aarch64-linux-gnu/libffi.so.8
#3  0x0000fffff7638750 in  () at /lib/aarch64-linux-gnu/libffi.so.8
#4  0x0000fffff7665a44 in  () at /usr/lib/python3.9/lib-dynload/_ctypes.cpython-39-aarch64-linux-gnu.so
#5  0x0000fffff7664c7c in  () at /usr/lib/python3.9/lib-dynload/_ctypes.cpython-39-aarch64-linux-gnu.so
#6  0x00000000004a54f0 in _PyObject_MakeTpCall ()
#7  0x000000000049cb10 in _PyEval_EvalFrameDefault ()
#8  0x0000000000496d1c in  ()
#9  0x00000000004b1eb0 in _PyFunction_Vectorcall ()
#10 0x0000000000498264 in _PyEval_EvalFrameDefault ()
#11 0x00000000004b1cb8 in _PyFunction_Vectorcall ()
#12 0x0000000000498418 in _PyEval_EvalFrameDefault ()
#13 0x0000000000496d1c in  ()
#14 0x00000000004b1eb0 in _PyFunction_Vectorcall ()
#15 0x0000000000498418 in _PyEval_EvalFrameDefault ()
#16 0x00000000004b1cb8 in _PyFunction_Vectorcall ()
#17 0x00000000004c6bc8 in  ()
#18 0x0000000000498264 in _PyEval_EvalFrameDefault ()
#19 0x00000000004b1cb8 in _PyFunction_Vectorcall ()
#20 0x0000000000498418 in _PyEval_EvalFrameDefault ()
#21 0x00000000004966f8 in  ()
#22 0x00000000004b1f18 in _PyFunction_Vectorcall ()
#23 0x0000000000498418 in _PyEval_EvalFrameDefault ()
#24 0x00000000004b1cb8 in _PyFunction_Vectorcall ()
#25 0x0000000000498264 in _PyEval_EvalFrameDefault ()
#26 0x00000000004966f8 in  ()
#27 0x0000000000496490 in _PyEval_EvalCodeWithName ()
#28 0x0000000000595b7c in PyEval_EvalCode ()
#29 0x00000000005c6a5c in  ()
#30 0x00000000005c0a70 in  ()
#31 0x00000000005c69a8 in  ()
#32 0x00000000005c6148 in PyRun_SimpleFileExFlags ()
#33 0x00000000005b60bc in Py_RunMain ()
#34 0x0000000000585a08 in Py_BytesMain ()
#35 0x0000fffff7d0c9d4 in __libc_start_main (main=
    0x5858fc <_start+60>, argc=2, argv=0xfffffffff498, init=<optimized out>, fini=<optimized out>, rtld_fini=<optimized out>, stack_end=<optimized out>) at ../csu/libc-start.c:332
#36 0x00000000005858f8 in _start ()
(gdb)
```

The frame #0 and frame #1 are the function frames in the CHIP C++ library, the
other frames live in the Python interpreter so you can ignore it.
