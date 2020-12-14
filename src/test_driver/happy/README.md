## Happy Tests

> Happy simulates complex network topologies. On a single Linux machine, Happy
> can create multiple nodes with network stacks that are independent from each
> other. Some nodes may be connected to simulated Thread networks, others may
> connect to simulated Wi-Fi, WAN (Internet), or cellular networks.

**You need to run happy tests on Linux platforms.**

### Usage

-   We suggest using
    [rootless Docker](https://docs.docker.com/engine/security/rootless/) if you
    have concerns about `--privileged` option on docker run. In your
    project-chip checkout path, run:

            $ docker run --rm --privileged \
              --entrypoint /bin/bash \
              --sysctl "net.ipv6.conf.all.disable_ipv6=0 net.ipv4.conf.all.forwarding=1 net.ipv6.conf.all.forwarding=1" \
              -it --mount type=bind,source=`pwd`,target=`pwd` \
              connectedhomeip/chip-build:0.4.18

    Mount your checkout to the same path as your local path avoids errors during
    bootstrap.

-   Install packages

            $ sudo scripts/tests/happy_tests.sh install_packages

-   Install happy framework

            $ scripts/tests/happy_tests.sh bootstrap

-   Build CHIP

            $ scripts/build/gn_bootstrap.sh
            $ scripts/build/gn_gen.sh --args="chip_enable_happy_tests=true"
            $ scripts/build/gn_build.sh

-   Run Tests

            $ RUN_HAPPY_TESTS=1 scripts/tests/gn_tests.sh

-   Run a single test

            $ scripts/tests/happy_test_wrapper.py" --test-bin-dir "out/tests" {test script path}
