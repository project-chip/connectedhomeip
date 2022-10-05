# Unify matter bridge unit testing

For unit testing the matter bridge we are tapping into the test eco system of
Matter. Writing a unit test is done with the NL Unit test framework and Matter
helper functions. Please refer to [Test example](TestExample.cpp) for writing a
simple unit test. After writing a unit test remember to add the source file in
the [BUILD.gn](BUILD.gn).

## Compiling the unit test

When wanting to compile the unit test use the below GN command to generate the ninja files:

```bash
silabs_examples/unify-matter-bridge/linux$ gn gen out/host --args='chip_build_tests=true use_coverage=true'
```

After generating the ninja files generating the tests is done as below:

```bash
silabs_examples/unify-matter-bridge/linux$ ninja -C out/host check
```

This generates test binaries inside `out/host/tests` where test executables can
be found.

## Running the unit test

So far we are not aware of how to run all the unit tests so running a unit test
currently is done by locating your test binary and executing it.

```bash
matter/silabs_examples/unify-matter-bridge/linux$ ./out/host/tests/TestExample
'#0:','ExampleTests'
'#3:','Example::TestExample','PASSED'
'#6:','0','1'
'#7:','0','1'
```

## Getting unit test coverage

For getting unit test coverage the Matter repo has a helper script. So after generating your test files you can run the below script:

```bash
matter/silabs_examples/unify-matter-bridge/linux$ ../../../scripts/build_coverage.sh --output_root out/host
```
