# Running the drinks machine app and controller

To run this example matter application, you need a version of the example app
and a controller. The example app can be compiled with gn / ninja, the
controller functions can be used through the chip-repl python shell

## App

### Compiling the app on linux

From the root of the chip tree

```
cd examples/valve/linux
gn gen out/debug
ninja -C out/debug
```

### Running the app on linux

From the root of the chip tree

```
./examples/valve/linux/out/debug/valve-app --KVS /tmp/valve_kvs.json
```

The KVS file is where the commissioning data is stored for the app, so be sure
to start the app with the same kvs file if you do not want to re-commission it
every time. All the standard linux app flags also work for this app. Use --help
to see the available options.

## Controller

### Compiling the chip-repl

To compile the chip-repl, from the root of the chip tree:

```
. scripts/activate.sh
./scripts/build_python.sh -i out/pyenv
source out/pyenv/activate
out/pyenv/bin/chip-repl
```

The chip-repl is a shell that lets you directly call python functions. It
instantiates a controller object that can be used to communicate with devices.
The controller is called devCtrl. By default, its KVS is at
/tmp/repl-storage.json, but this can be changed on the command line if desired.
Use --help to see options.

### Commissioning the valve app

As long as the controller and the application KVSs are kept constant, the app
should only need to be commissioned once.

To commission the device use:

```
from chip import ChipDeviceCtrl
await devCtrl.CommissionOnNetwork(nodeId=1, setupPinCode=20202021, filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=3840)
```

### Interacting with the valve app

To create a drinks machine controller:

```
import examples.valve.controller as DrinksController
dm = DrinksController.DrinkMachine(devCtrl, 1)
```

You can now call functions on the drinks machine controller. Tab completion will
work within the repl, so you can see the available options.
