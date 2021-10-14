# README

Python based lighting example (bridge) device to DALI.

## Installation

Build the Python/C library:

```shell
cd ~/connectedhomeip/
git submodule update --init
source scripts/activate.sh
./scripts/build_python_device.sh --chip_mdns platform --chip_detail_logging true
source ./out/python_env/bin/activate
```

Install the python dependencies:

```shell
cd examples/lighting-app/python
pip3 install python-dali
```

Plug-in a python-dali compatible USB-DALI interface.

## Usage

Run the Python lighting matter device:

```shell
cd examples/lighting-app/python
python lighting.py
```

Control the Python lighting matter device:

```shell
source ./out/python_env/bin/activate

chip-device-ctrl

chip-device-ctrl > connect -ip 127.0.0.1 20202021 1234
chip-device-ctrl > zcl OnOff Toggle 1234 1 0
```
