# Arm Virtual Hardware (AVH) based tests

This folder contains end to end tests that use the
[Arm Virtual Hardware (AVH)](https://www.arm.com/products/development-tools/simulation/virtual-hardware)
service.

The tests require the `AVH_API_TOKEN` environment variable is set with the value
from `AVH -> Profile -> API -> API Token`.

## Current tests

-   [`test_lighting_app.py`](test_lighting_app.py)
    -   This test uses two virtual Raspberry Pi Model 4 boards running Ubuntu
        Server 22.04 and pre-built `chip-tool` and `chip-lighting-app` binaries
        (`linux-arm64`), and tests commissioning and control over BLE and Wi-Fi
        using the virtual Bluetooth and Wi-Fi network features of AVH.

## Running the tests

1. Install dependencies

```
pip3 install -r requirements.txt
```

2. Set AVH_API_TOKEN` environment variable

```
export AVH_API_TOKEN=<AVH API TOKEN value>
```

3. Place cross-compiled `chip-tool` and `lighting-app` binaries in `out` folder

4. Run

```
python3 -u -m unittest test_lighting_app.py
```
