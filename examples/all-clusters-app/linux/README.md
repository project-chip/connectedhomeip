# Matter Linux/Mac All Clusters Example

## Fuzzing integration

This example supports compilation with libfuzzer enabled.

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
