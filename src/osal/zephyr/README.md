# Zephyr port of CHIP OSAL

The Zephyr port implements the CHIP OSAL abstractions, always using the
equivalent native Zephyr module with the goal of mapping to the native Zephyr
module in the most minimal and direct way.

The Zephyr port provides implementations of the following CHIP OSAL
abstractions:

-   [tasks](#Task) (aka threads)
-   [mutex](#Mutex)
-   [semaphores](#Semaphore)
-   [queues](#Queue)
-   [timers](#Timer)
-   [time](#Time)

## Install Zephyr

1. Install Host Dependencies

    ```bash
    sudo apt-get update
    sudo apt-get upgrade

    sudo apt-get install --no-install-recommends git cmake ninja-build gperf \
      ccache dfu-util device-tree-compiler wget \
      python3-pip python3-setuptools python3-tk python3-wheel xz-utils file \
      make gcc gcc-multilib g++-multilib

    pip3 install --user cmake

    wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.10.3/zephyr-sdk-0.10.3-setup.runZephyr
    chmod +x zephyr-sdk-0.10.3-setup.run
    ./zephyr-sdk-0.10.3-setup.run -- -d ~/zephyr-sdk-0.10.3
    ```

2. Set these environment variables

    ```bash
    echo "export ZEPHYR_BASE=$HOME/zephyrproject/zephyr" >> $HOME/.zephyrrc
    echo "export ZEPHYR_TOOLCHAIN_VARIANT=zephyr" >> $HOME/.zephyrrc
    echo "export ZEPHYR_SDK_INSTALL_DIR=$HOME/zephyr-sdk-0.10.3" >> $HOME/.zephyrrc
    ```

3. Get the Zephyr source code
    ```bash
    pip3 install --user west
    west init zephyrproject
    cd zephyrproject
    west update
    pip3 install --user -r zephyr/scripts/requirements.txt
    ```

---

## Reference

### Task

The port uses native
[Zephyr Thread](https://docs.zephyrproject.org/latest/reference/kernel/threads/index.html):
`struct k_thread` using `k_thread_create()`.

### Mutex

The port uses native
[Zephyr Mutex](https://docs.zephyrproject.org/latest/reference/kernel/synchronization/mutexes.html):
`struct k_mutex` using `k_mutex_init()`.

### Semaphore

The port uses native
[Zephyr Semaphore](https://docs.zephyrproject.org/latest/reference/kernel/synchronization/semaphores.html):
`struct k_sem` using `k_sem_init()`.

### Queue

The port uses native
[Zephyr Message Queue](https://docs.zephyrproject.org/latest/reference/kernel/data_passing/message_queues.html):
`struct k_msgq` using `k_msgq_init()`.

### Timer

The port uses native
[Zephyr Timer](https://docs.zephyrproject.org/latest/reference/kernel/timing/timers.html):
`struct k_timer` using `k_timer_init()`.

### Time

The port uses native
[Zephyr Uptime Timing](https://docs.zephyrproject.org/latest/reference/kernel/timing/clocks.html):
using `k_uptime_get()`.

## Port Details

### Support Status

| Module | Zephyr    |
| ------ | --------- |
| Task   | DOCS ONLY |
| Mutex  | DOCS ONLY |
| Sem    | DOCS ONLY |
| Timer  | DOCS ONLY |
| Time   | DOCS ONLY |
| Queue  | DOCS ONLY |

### Zephyr Port Layout

```
src/osal/Zephyr
├── chip
│   └── os_port.h         - Primary port-specific header to hook into public src/include/chip/osal.h
├── os_mutex.c            - Implementation of chip_os_mutex
├── os_queue.c            - Implementation of chip_os_queue
├── os_sem.c              - Implementation of chip_os_sem
├── os_task.c             - Implementation of chip_os_task
├── os_time.c             - Implementation of chip_os_time
├── os_timer.c            - Implementation of chip_os_timer
└── README.md             - This README
```
