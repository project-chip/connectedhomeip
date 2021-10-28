# README

Python based lighting example (bridge) device to DALI.

## Installation

Build the Python/C library:

```shell
cd ~/connectedhomeip/
git submodule update --init
source scripts/activate.sh
#./scripts/build_python_device.sh --chip_mdns platform --chip_detail_logging true

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
chip-device-ctrl > zcl NetworkCommissioning AddWiFiNetwork 12344321 0 0 ssid=str:YOURSSID credentials=str:YOURPASSWD breadcrumb=0 timeoutMs=1000
chip-device-ctrl > zcl NetworkCommissioning EnableNetwork 12344321 0 0 networkID=str:YOURSSID breadcrumb=0 timeoutMs=1000
chip-device-ctrl > close-ble
chip-device-ctrl > resolve 5544332211 1 (pass appropriate fabric ID and node ID, you can get this from get-fabricid)
chip-device-ctrl > zcl OnOff Toggle 12344321 1 0
```

**For our network**

```shell
connect -ble 3840 20202021 12344321

zcl NetworkCommissioning AddWiFiNetwork 12344321 0 0 ssid=str:DatNeueWlan credentials=str:09August1981 breadcrumb=0 timeoutMs=1000

zcl NetworkCommissioning EnableNetwork 12344321 0 0 networkID=str:DatNeueWlan breadcrumb=0 timeoutMs=1000

close-ble

get-fabricid

# use the fabric ID as first argument
resolve 4709919307557107237 12344321

zcl OnOff Toggle 12344321 1 0
```

## stop docker

sudo service docker stop

## allow dhclient to rename its sub-processes

sudo nano /etc/apparmor.d/sbin.dhclient

@{PROC}/@{pids}/task/[0-9]\*/comm rw,

## disable avahi v6 (since dhclient only requests v4)

sudo nano /etc/avahi/avahi-daemon.conf

```txt
[server]
use-ipv6=no
```

## root for dhclient

possibly this would help

sudo setcap CAP_NET_BIND_SERVICE,CAP_NET_RAW=+ep \$(which dhclient)

## avahi

compile with minimal mdns instead of with avahi

sudo service avahi-daemon stop

## for testing

ip link set dev wlan0 down
