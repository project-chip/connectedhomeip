# Unify matter bridge unit testing

The Matter bridge test infrastructure leverages the test ecosystem of Matter itself. Writing a unit test is done with the NL Unit test framework and Matter
helper functions. Please refer to [Test example](TestExample.cpp) for an example of how to write a simple unit test. After writing a unit test remember to add the source file in the [BUILD.gn](BUILD.gn).

## Build the docker container (host compilation)

```bash
dev-machine:~$ docker build -t unify-matter-host --build-arg ARCH=amd64 silabs_examples/unify-matter-bridge/docker/
```

Starting the docker:

```bash
dev-machine:~$ docker run -it -v $PWD:/matter -v $PWD/../uic-matter:/uic unify-matter-host
```

Compile and install libunify for host.

```bash
root@docker:/uic$ cmake -DCMAKE_INSTALL_PREFIX=$PWD/stage -GNinja -B build_unify_amd64/ -S components
root@docker:/uic$ cmake --build build_unify_amd64
root@docker:/uic$ cmake --install build_unify_amd64 --prefix $PWD/stage
root@docker:/uic$ export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$PWD/stage/share/pkgconfig
```

Activate Matter development environment.

```bash
root@docker:/matter$ source ./scripts/activate.sh
```

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
