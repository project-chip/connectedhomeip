# Python based lighting example (bridge) device to DALI.

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

Control the Python lighting matter device:

```shell
source ./out/python_env/bin/activate

chip-device-ctrl

chip-device-ctrl > connect -ble 3840 20202021 12344321
chip-device-ctrl > zcl NetworkCommissioning AddOrUpdateWiFiNetwork 12344321 0 0 ssid=str:YOUR_SSID credentials=str:YOUR_PASSWORD breadcrumb=0
chip-device-ctrl > zcl NetworkCommissioning ConnectNetwork 12344321 0 0 networkID=str:YOUR_SSID breadcrumb=0
chip-device-ctrl > close-ble
chip-device-ctrl > resolve 5544332211 1 (pass appropriate fabric ID and node ID, you can get this from get-fabricid)
chip-device-ctrl > zcl OnOff Toggle 12344321 1 0
```
