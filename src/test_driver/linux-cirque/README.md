# The cirque testing suite

Cirque simulates complex network topologies based upon docker nodes. On a single
Linux machine, it can create multiple nodes with network stacks that are
independent from each other. Some nodes may be connected to simulated Thread
networks, others may connect to simulated BLE or WiFi.

In Project CHIP, cirque is used for integration tests.

There is a script for running cirque tests, you can find it at
`scripts/tests/cirque_tests.sh`

## "Docker out of docker" setup

If you don't want cirque break your local environment, you can enter a
environment insider docker

```
integrations/docker/images/stage-2/chip-build-cirque/run.sh --privileged --volume /dev/pts:/dev/pts --volume /tmp:/tmp -it -- bash
```

## Setting up cirque environment

After checkout, in your local project chip directory, run:

```
git submodule update --init
scripts/tests/cirque_tests.sh bootstrap
```

It will:

1. Install necessary packages required by cirque,

2. Build `project-chip/chip-cirque-device-base:latest` docker image for running
   devices,

3. Build openthread for simulating thread network.

> Note: Suggest using a clean checkout before running this.

## Run tests

You can run the tests by:

```
scripts/tests/cirque_tests.sh run_all_tests
```

You can get more information from the terminal output (like the location of
logs).

> Note: The container (as well as the networks) will be preserved for debugging.
> You may need to clean it by yourself.

## Run specific test

You can run a single cirque test by:

```
scripts/tests/cirque_tests.sh run_test <test name>
```

The test name is the script (`.sh`) file in `src/test_driver/linux-cirque`, like
`OnOffClusterTest`.

For example, you can run `OnOffClusterTest` by:

```
scripts/tests/cirque_tests.sh run_test OnOffClusterTest
```

The output of the test will be written to `stdout` and `stderr`.

## Specify log directory

To specify log directory, simply set `LOG_DIR` variable.

```
export LOG_DIR=/some/log/directory
scripts/tests/cirque_tests.sh run_all_tests
scripts/tests/cirque_tests.sh run_test OnOffClusterTest
```

Or

```
LOG_DIR=/some/log/directory scripts/tests/cirque_tests.sh run_all_tests
LOG_DIR=/some/log/directory scripts/tests/cirque_tests.sh run_test OnOffClusterTest
```

## Setup test topology only

You can run a ManualTest to setup test topology only:

```
./scripts/tests/cirque_tests.sh run_test ManualTest -t <topology file>
```

The topology file is a JSON file, which contains the definition of each node in
the network.

```
./scripts/tests/cirque_tests.sh run_test ManualTest -t src/test_driver/linux-cirque/topologies/three_node_with_thread.json
```

It will print the container id in log, you can execute commands inside them.

```
2021-04-06 15:01:57,780 [CHIPCirqueTest] INFO Finished setting up environment.
2021-04-06 15:01:57,780 [CHIPCirqueTest] INFO Device: CHIP-Server (Type: CHIP-Server, Container: 459c901ed9)
2021-04-06 15:01:57,780 [CHIPCirqueTest] INFO Device: CHIP-Tool (Type: CHIP-Tool, Container: c5831124e7)
2021-04-06 15:01:57,780 [CHIPCirqueTest] INFO Press Ctrl-C to stop the test.
2021-04-06 15:01:57,780 [CHIPCirqueTest] INFO Container will be cleaned when the test finished.
```

> You can run docker commands with these containers, for example, to launch a
> shell on CHIP-Tool, you can use:
>
> ```
> docker exec -it c5831124e7 /bin/bash
> ```
>
> For detailed command you can use, please refer to
> [official docker documents](https://docs.docker.com/engine/reference/commandline/cli/).

> It is not recommanded to run commands that can change the state of the
> container, for example: `attach` (will stop container once you exit), `stop`
> etc.

After you finished you test, press `Ctrl-C` and it will clean up testing
environment.

Refer to `test-manual.py`, `ManualTest.sh`, and topologies file under
`topologies` for detail.
