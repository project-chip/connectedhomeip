# Admin Flow

* CHIP SDK is maintained on a main branch `master` and release branches cut out for specific deliverables like `test_event2.2`
* CSG fork of the CHIP SDK is maintained with all existing main and release branches left untouched and pristine. 
* All CSG specific changes should be done on newly created branches (with a `csg/` prefix) based on these release branches. eg: `test_event2.2` -> `csg/test_event2.2`
* Only admins on the repo have access to pull in changes from the upstream repo and update all local main and release branches using the following commands or equivalent:
```bash
git fetch upstream
git pull upstream 
git push origin
``` 
* Once updated create a local version of the release branch with a csg prefix to add our changes. This branch will be the main merge target for all the PRs in this dev period.

# Development Flow

* Developers must create a branch with the following prefix `csg/fix` or `csg/feature` and submit their PRs to be merged to only the csg version of the release branch.
* If the admins upstream the branches, the developers must rebase their changes resolve their conflicts locally before submitting the PR
* Changes should be only additive, isolated and minimal, effort should be made to minimize the footprint on changes in the existing code.

# Building

* At the root directory:
```bash
source scripts/activate.sh
gn gen out/debug
ninja -C out/debug
```
* A .whl file is generate at `out/debug/controller/python/chip-0.0-cp37-abi3-linux_aarch64.whl`
* Copy this wheel file into the sample test client folder or into the test harness controller dockerfile folder.

# Testing with a real accessory
* You need to map the the system dbus to the docker dbus and run the docker with --privileged to be able to communicate with the real accessory 
* Launch the a Docker container for chip device controller
Sample Docker File Used: https://docs.google.com/document/d/1-CMDrKEBGnHhSvAUMHH7ZSRDiKhuu2iwI-GYVUjI7DY/edit?resourcekey=0-VTKXbTk9YYQmdq10xa_f7g#heading=h.yzihj7w54f7a
docker run --name docker-python-controller -v /var/run/dbus:/var/run/dbus --privileged -it <Docker Image>

* From the Launched Python controller Docker container run the chip device controller with the BLE adapter parameter set to hci0
chip-device-ctrl --bluetooth-adapter=hci0
* You should now be able to run 'ble-scan' and 'connect -ble' actions from the chip device controller

# Testing with virtual BLE accessory
* For this you will need to create virtual BLE interfaces in the Test harness Docker dev Container
* You need to map the the system dbus to the docker dbus and run the docker with --privileged to be able to create the virtual ble interfaces using Bluez 

* Setting up the Virtual BLE interfaces from Inside a docker container to be used by Other docker containers on the same host:
The bluez in ubuntu seems already the same as the version as CHIP repo, so we don't have to build bluetoothd by ourselves.
Bluetoothd will start on the host on boot, So we infact skip the bluetoothd setup 

You still need to install bluez inside container

* Starting with a Clean Raspi host post Bootup:
ubuntu@ubuntu:~$ ps -a
    PID TTY          TIME CMD
   2357 pts/0    00:00:00 ps

hci0:	Type: Primary  Bus: UART
	BD Address: B8:27:EB:C4:E2:93  ACL MTU: 1021:8  SCO MTU: 64:1
	UP RUNNING 
	RX bytes:794 acl:0 sco:0 events:52 errors:0
	TX bytes:2760 acl:0 sco:0 commands:52 errors:0
 
* Bringing up the Virtual Interfaces in Docker container 1:
Follow the instructions as mentioned on chip-device-controller documentation
```bash
cd third_party/bluez/repo/
./bootstrap
./configure --prefix=/usr --mandir=/usr/share/man --sysconfdir=/etc --localstatedir=/var --enable-experimental --with-systemdsystemunitdir=/lib/systemd/system --with-systemduserunitdir=/usr/lib/systemd --enable-deprecated --enable-testing --enable-tools
make
emulator/btvirt -L -l<Number of Virtual BLE interfaces to be created> &
```
* Check hciconfig to confirm the said number of Virtual BLE interfaces are created

* Launch the a Docker container for chip Linux Lighting app
Sample Docker File used: https://docs.google.com/document/d/1xOizHV3ZeG_mu70CJWp-tN4xYoDxRhjEnwFKzhkIiC0/edit#heading=h.9hjqswbm7x50
docker run --name docker-chip-lighting-app -v /var/run/dbus:/var/run/dbus --privileged -it <Docker Image>

* Launch the Linux Lighting App in the docker-chip-lighting-app Docker container specifying the ble-device parameter as follows:
./chip-lighting-app --ble-device 1 --wifi

* Launch the Chip Device controller in a different container(ex: docker-python-controller docker container as mentioned above in Testing with real accessory)
* Run the chip device controller with --ble-adapter set to one of the virtual BLE interfaces(ex: hci2 this interface has to be different then one used by the virtual BLE accessory) as follows:
chip-device-ctrl --bluetooth-adapter=hci2
* You should now be able to run 'ble-scan' and 'connect -ble' actions from the chip device controller

# Testing with Hybrid setup with Real and Virtual BLE accessories
* When we have a real accessory along with a virtual ble accessory you will be able to interact/control with only one accessory at a time. In such Hybrid Setup you will have to switch between BLE adapters in chip-device-controller to handle respective accessories.
* To switch between ble interfaces run "ble-adapter-select <adapter number example: hci1 to scan and connect to virtual accessory and hci0 to scan and connect to real accessory>"
