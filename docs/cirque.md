# The cirque testing suite

Cirque simulates complex network topologies based upon docker nodes. On a single Linux machine, it can create multiple nodes with network stacks that are independent from each other. Some nodes may be connected to simulated Thread networks, others may connect to simulated BLE or WiFi.

In Project CHIP, cirque is used for tntegration tests.

There is a script for running cirque tests, you can find it at `scripts/tests/cirque_tests.sh`

## Setting up cirque environment

After checkout, in your local project chip directory, run:

```
scripts/tests/cirque_tests.sh bootstrap
```

It will:

1. Install necessary packages required by cirque,

2. Build `chip-cirque-device-base` docker image for running devices,

3. Build openthread for simulating thread network.

> Note: Suggest using a clean checkout before running this.

## Run tests

You can run the tests by:

```
scripts/tests/cirque_tests.sh run_all_tests
```

You can get more infomation from the terminal output (like the location of logs).

> Note: The container (as well as the networks) will be preserved for debugging. You may need to clean it by yourself.
