# Python-based lighting example (bridge) device to DALI

## Installation

Build the Python/C library:

```shell
cd ~/connectedhomeip/
git submodule update --init
source scripts/activate.sh

./scripts/build_python_device.sh --chip_detail_logging true

sudo su # dhclient is called, needs root
source ./out/python_env/bin/activate
```

Install the python dependencies:

```shell
pip3 install python-dali
```

Plug-in a python-dali compatible USB-DALI interface.

## Usage

Run the Python lighting matter device:

```shell
cd examples/lighting-app/python
python lighting.py
```

Control the Python lighting matter device using the Python controller REPL:

[Python Controller](https://github.com/project-chip/connectedhomeip/blob/master/src/controller/python/README.md)
