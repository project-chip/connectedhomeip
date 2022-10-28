# Unify matter bridge unit testing

The Matter bridge test infrastructure leverages the test ecosystem of Matter itself. Writing a unit test is done with the NL Unit test framework and Matter
helper functions. Please refer to [Test example](TestExample.cpp) for an example of how to write a simple unit test. After writing a unit test remember to add the source file in the [BUILD.gn](BUILD.gn).

## Compiling the unit test

Before compiliing a unit test, use the following GN command to generate the ninja files necessary for the build process:

```bash
silabs_examples/unify-matter-bridge/linux$ gn gen out/host --args='chip_build_tests=true use_coverage=true'
```

After generating the ninja files, the tests are generated with the following command:

```bash
silabs_examples/unify-matter-bridge/linux$ ninja -C out/host check
```

This generates test binaries inside `out/host/tests` where test executables can
be found.

## Running the unit test

Unit tests are run by first locating your test binary and then executing it.

```bash
matter/silabs_examples/unify-matter-bridge/linux$ ./out/host/tests/TestExample
'#0:','ExampleTests'
'#3:','Example::TestExample','PASSED'
'#6:','0','1'
'#7:','0','1'
```

## Getting unit test coverage

To understand unit test coverage, run the following helper script:

```bash
matter/silabs_examples/unify-matter-bridge/linux$ ../../../scripts/build_coverage.sh --output_root out/host
```
