# Matter Linux/Mac All Clusters Example

## Compiling all-clusters-app for testing on Linux and Mac

To compile all-clusters-app on Intel Mac, run:

```
$ ./scripts/run_in_build_env.sh "./scripts/build/build_examples.py --target darwin-x64-all-clusters-no-ble-asan-clang build"
```

at the top level of the Matter tree.

To compile on an Arm Mac, run:

```
$ ./scripts/run_in_build_env.sh "./scripts/build/build_examples.py --target darwin-arm64-all-clusters-no-ble-asan-clang build"
```

Similarly, to compile on Linux x86-64 run:

```
$ ./scripts/run_in_build_env.sh "./scripts/build/build_examples.py --target linux-x64-all-clusters-no-ble-asan-clang build"
```

And to compile on Linux ARM run:

```
$ ./scripts/run_in_build_env.sh "./scripts/build/build_examples.py --target linux-arm64-all-clusters-no-ble-asan-clang build"
```

## Fuzzing integration

This example also supports compilation with libfuzzer enabled. This should be
used when trying to fuzz-test the Matter SDK.

### Compiling with fuzzing enabled

To compile with libfuzzer enabled on Mac, run:

```
$ ./scripts/run_in_build_env.sh "./scripts/build/build_examples.py --target darwin-x64-all-clusters-no-ble-asan-libfuzzer-clang build"
```

at the top level of the Matter tree.

Similarly, to compile on Linux run:

```
$ ./scripts/run_in_build_env.sh "./scripts/build/build_examples.py --target linux-x64-all-clusters-no-ble-asan-libfuzzer-clang build"
```

### Running libfuzzer-enabled binaries

#### Initial run

To run the resulting binary with no particular inputs do:

```
$ ./out/darwin-x64-all-clusters-no-ble-asan-libfuzzer-clang/chip-all-clusters-app-fuzzing
```

or

```
$ ./out/linux-x64-all-clusters-no-ble-asan-libfuzzer-clang/chip-all-clusters-app-fuzzing
```

If this crashes, it will output the input that caused the crash in a variety of
formats, looking something like this:

```
0xe,0x0,0xf1,0xb1,0xf1,0xf1,0xf1,0xf1,0xed,0x73,0x7,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xc1,0x0,0x0,0x0,0x0,0x0,0x5c,0xf3,0x25,0x0,0x0,0x0,0x0,0x0,
\016\000\361\261\361\361\361\361\355s\007\000\000\000\000\000\000\000\301\000\000\000\000\000\\\363%\000\000\000\000\000
artifact_prefix='./'; Test unit written to ./crash-c9fd2434ccf4a33a7f49765dcc519e1fd529a8e5
Base64: DgDxsfHx8fHtcwcAAAAAAAAAwQAAAAAAXPMlAAAAAAA=
```

Note that this creates a file holding the input that caused the crash.

#### Run with a fixed input

To run the binary with a specific input, place the input bytes in a file (which
a crashing run of the fuzzer does automatically). If `$(INPUT_FILE)` is the name
of that file, then run:

```
$ ./out/darwin-x64-all-clusters-no-ble-asan-libfuzzer-clang/chip-all-clusters-app-fuzzing $(INPUT_FILE)
```

or

```
$ ./out/linux-x64-all-clusters-no-ble-asan-libfuzzer-clang/chip-all-clusters-app-fuzzing $(INPUT_FILE)
```

#### Additional execution options.

The binary can be run with `-help=1` to see more available options.

Running with `ASAN_OPTIONS="handle_abort=2"` set in the environment may produce
nicer stack traces.

### Trigger event using all-cluster-app event named pipe

You can send a command to all-cluster-app to trigger specific event via
all-cluster-app event named pipe /tmp/chip_all_clusters_fifo-<PID>.

#### Trigger `SoftwareFault` events

1. Generate event `SoftwareFault` when a software fault takes place on the Node.

```
$ echo '{"Name":"SoftwareFault"}' > /tmp/chip_all_clusters_fifo-<PID>
```

#### Trigger `HardwareFault` events

1. Generate event `HardwareFaultChange` to indicate a change in the set of
   hardware faults currently detected by the Node.

```
$ echo '{"Name":"HardwareFaultChange"}' > /tmp/chip_all_clusters_fifo-<PID>
```

2. Generate event `RadioFaultChange` to indicate a change in the set of radio
   faults currently detected by the Node.

```
$ echo '{"Name":"RadioFaultChange"}' > /tmp/chip_all_clusters_fifo-<PID>
```

3. Generate event `NetworkFaultChange` to indicate a change in the set of
   network faults currently detected by the Node.

```
$ echo '{"Name":"NetworkFaultChange"}' > /tmp/chip_all_clusters_fifo-<PID>
```

4. Generate event `BootReason` to indicate the reason that caused the device to
   start-up, from the following set of `BootReasons`.

-   `PowerOnReboot` The Node has booted as the result of physical interaction
    with the device resulting in a reboot.

-   `BrownOutReset` The Node has rebooted as the result of a brown-out of the
    Node’s power supply.

-   `SoftwareWatchdogReset` The Node has rebooted as the result of a software
    watchdog timer.

-   `HardwareWatchdogReset` The Node has rebooted as the result of a hardware
    watchdog timer.

-   `SoftwareUpdateCompleted` The Node has rebooted as the result of a completed
    software update.

-   `SoftwareReset` The Node has rebooted as the result of a software initiated
    reboot.

```
$ echo '{"Name":"<BootReason>"}' > /tmp/chip_all_clusters_fifo-<PID>
```

#### Trigger Switch events

1. Generate event `SwitchLatched`, when the latching switch is moved to a new
   position.

```
$ echo '{"Name":"SwitchLatched","NewPosition":3}' > /tmp/chip_all_clusters_fifo-<PID>
```

2. Generate event `InitialPress`, when the momentary switch starts to be
   pressed.

```
$ echo '{"Name":"InitialPress","NewPosition":3}' > /tmp/chip_all_clusters_fifo-<PID>
```

3. Generate event `LongPress`, when the momentary switch has been pressed for a
   "long" time.

```
$ echo '{"Name":"LongPress","NewPosition":3}' > /tmp/chip_all_clusters_fifo-<PID>
```

4. Generate event `ShortRelease`, when the momentary switch has been released.

```
$ echo '{"Name":"ShortRelease","PreviousPosition":3}' > /tmp/chip_all_clusters_fifo-<PID>
```

5. Generate event `LongRelease` when the momentary switch has been released and
   after having been pressed for a long time.

```
$ echo '{"Name":"LongRelease","PreviousPosition":3}' > /tmp/chip_all_clusters_fifo-<PID>
```

6. Generate event `MultiPressOngoing` to indicate how many times the momentary
   switch has been pressed in a multi-press sequence, during that sequence.

```
$ echo '{"Name":"MultiPressOngoing","NewPosition":3,"CurrentNumberOfPressesCounted":4}' > /tmp/chip_all_clusters_fifo-<PID>
```

7. Generate event `MultiPressComplete` to indicate how many times the momentary
   switch has been pressed in a multi-press sequence, after it has been detected
   that the sequence has ended.

```
$ echo '{"Name":"MultiPressComplete","PreviousPosition":3,"TotalNumberOfPressesCounted":2}' > /tmp/chip_all_clusters_fifo-<PID>
```
