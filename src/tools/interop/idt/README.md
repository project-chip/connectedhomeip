# Interop Debugging Tool

## Overview

The “Interop Debugging Tool” (IDT) is a python-based tool that supports a variety of 
commands that are useful in the context of interop testing of Matter devices and 
app controllers.

### Discovery

While in discovery mode, the tool displays all Matter devices that are in 
commission and/or operational mode. This is useful to have a clear 
understanding of all Matter devices currently “active” in the testing environment.

The Google Developer Center provides a good overview 
of [Commissionable and Operational Discovery](https://developers.home.google.com/matter/primer/commissionable-and-operational-discovery).   
See also section “4.3. Discovery” of the Matter spec for official documentation.

When run interactively, discovery functions in one of two modes: BLE and mDNS.

### Capture 

While in capture mode, the tool starts capturing all data of interest 
(e.g. video recording of interactions with the mobile app, logs from all components
involved, network packets capture, etc.) while a test is being conducted manually. It also 
provides feedback to the user on test setup and execution.

When the test completes, capture mode is stopped and all captured data is zipped in
a file that can then be sent to all parties involved in investigating any issue
uncovered via the manual test. Each ecosystem may implement an analysis that analyzes 
capture data, displays info to the user, probes the local environment and generates 
additional artifacts.

## Getting started

## Raspberry Pi installation

### Environment overview

The execution environment of IDT when using Raspberry Pi is shown in the figure below.

[TODO] add figure.

The Raspberry Pi is referred to as the "target environment" in this document. 
It is where all  the "discovery" and "capture" work is performed.

The "admin" computer is the machine used to connect to and control the pi, 
and to fetch artifacts which were created during capture from the pi.

This repository may be used on both the admin computer and the target environment.

### Environment details

1. This guide and the `idt` scripts expect ***one*** instance of `idt` on both the 
   admin computer (e.g. macbook) and the target environment.
1. The expected install location on the target environment is the 
   home directory of the user specified in `idt/scripts/vars.sh`. Default values 
   are: 
   ```
   PIHOST="kali-raspberry-pi" 
   PIUSER="kali"
   ```
1. Helper scripts may be used on admin computers that support `zsh` and `bash` 
   (Linux and macOS).
1. Windows may be used as the admin computer via tools like `PowerShell`, 
   `MobaXterm` and `FileZilla`.
1. This setup is intended to work with the admin computer and Pi connected to the
   same Wi-Fi network, which is also the Wi-Fi network used for testing.
1. Corporate networks are not expected to be used as test networks.


### Prepare the Pi

1. A >= 128 GB SD card is recommended.
1. Flash the Pi SD with the latest `Kali Linux RaspberryPi 2, 3, 4 and 400 (img.xz)` image (32 bit).
    * Use [Raspberry Pi Imager](https://github.com/raspberrypi/rpi-imager) to 
      [download the image and prepare the SD](https://www.kali.org/docs/arm/using-rpi-imager-to-write-raspberry-pi-images/).
1. Plug the SD into the Pi.
1. Connect the Pi to your network via **ethernet.** [TODO] provide alternative
1. Boot the Pi.

### Configure admin computer and push to the Pi

#### Linux and macOS admin computers

1. On your admin computer, source the `alias` script from the parent directory of `idt` to get `idt` commands in your current shell.
   ```bash
    source idt/scripts/alias.sh 
    ```
    * To avoid having to repeat this step for each session, optionally configure automatic aliases permanently.
    * ***NOTE:*** Once run, `idt` commands will be globally and automatically available. If you need to remove the installation, edit the `.rc` files mentioned in `setup_shell`.
    ```bash
    source idt/scripts/setup_shell.sh 
    ```
1. Send `idt` to the Pi:
   ```
   idt_push
   ```
1. `ssh` to the Pi (the default password for Kali is `kali`):
   * ***NOTE:*** You may need to wait a few minutes after boot for the `ssh` server to be available on the Pi. Retry if needed!
   ```
   idt_connect
   ```

#### Windows admin computers

1. Open `PowerShell`, cd to the directory containing `idt` and send `idt` to the Pi:
   ```
   scp -r ./idt/* kali@kali:/home/kali/idt
   ```
1. `ssh` to the Pi, e.g. with `MobaXterm` (the default password for Kali is `kali`)
   * ***NOTE:*** You may need to wait a few minutes after boot for the `ssh` server to be available on the Pi. Retry if needed!
   * Use `kali@kali-raspberry-pi` or `kali@$ip` where `$ip` is the Pi's IP found in 
     your router admin panel.

### Configure the Pi   

1. Once in an ssh session, set a strong password.
   ```
   sudo passwd kali
   ```
1. Inside the Pi, configure Wi-Fi. Disconnect ethernet and reboot.
   ```
   nmtui
   ```
1. Set up `idt`:
   ```commandline
   cd ~                               # Go to idt parent dir
   source idt/scripts/setup_shell.sh  # Setup atuo aliases
   source idt/scripts/alias.sh        # Get aliases now
   idt_bootstrap                      # Initial configuration
   idt_build                          # Build the container image
   ```

### Install updates

SCP may not overwrite all files. To clear the `idt` dir off of the Pi safely between pushes, exit the container and:

```
idt_clean
```

NOTE the idt artifacts directory is contained in idt, so running this will delete any artifacts ([TODO] change).

Then from the admin computer:

```
idt_push
```

## Debian installation

This package should also work on most other Debian (/based) systems.

The machine running `idt` should be connected to the same Wi-Fi network used for testing.

- From the parent directory of `idt`, run `source idt/scripts/alias.sh`.
- Optionally, run `source idt/scripts/setup_shell.sh ` to install aliases permanently.

> You may use `idt` natively (Python virtual environment) OR via Docker. 

### Python virtual environment

- `idt` is currently tested on `Python 3.11`.  
- `adb` and `tcpdump` are required.

Setup a virtual environment and install python dependencies:

```commandline
idt_go
cd idt
python -m venv /venv
source venv/bin/activate
pip install -r requirements.txt
```

Remember to re-enter the virtual environment as needed. [TODO] script

### Docker

- Run `idt_build` and `idt_activate` to enter the `idt` environment.

[TODO] Podman

## macOS installation

Most features other than BLE should work on macOS.

Follow the Debian installation steps above, but do not use Docker.

[TODO] macOS BLE support

## User guide

> ***IMPORTANT***  
> `idt_` commands are shell aliases helpful for administrative commands.  
> `idt` invokes the `idt` python package.

Pi users, as needed:

* For users with Windows admin computers, reconnect e.g., using `MobaXterm` 
* Other users reconnect `ssh` to the Pi (from your admin computer):
   ```
   idt_connect
   ```
* Run the `idt` container (from the Pi):
   ```
   idt_activate
   ```

### Capture

```commandline
idt capture -h

usage: idt capture [-h] [--platform {Android}]
                   [--ecosystem {PlayServices,PlayServicesUser,ALL}]
                   [--pcap {t,f}]
                   [--interface {wlp0s20f3,docker0,lo}]
                   [--additional {t,f}]

options:
  -h, --help            show this help message and exit
  --platform {Android}, -p {Android}
                        Run capture for a particular platform
                        (default Android)
  --ecosystem {PlayServices,PlayServicesUser,ALL}, -e {PlayServices,PlayServicesUser,ALL}
                        Run capture for a particular ecosystem or ALL
                        ecosystems (default ALL)
  --pcap {t,f}, -c {t,f}
                        Run packet capture (default t)
  --interface {wlp0s20f3,docker0,lo}, -i {wlp0s20f3,docker0,lo}
                        Run packet capture against a specified
                        interface (default wlp0s20f3)
  --additional {t,f}, -a {t,f}
                        Run ble and mdns scanners in the background
                        while capturing (default t)
```

#### Example ALL ecosystem run

```
idt capture

________________________________________________________________
Using artifact dir /home/user/idt/IDT_ARTIFACTS/idt_20230831_010707
________________________________________________________________

If there is no output below, press enter after connecting your phone under test OR
Enter (copy paste) the target device id from the list of available devices below OR
Enter $IP4:$PORT to connect wireless debugging.
192.168.8.206:43677
Detected connection string; attempting to connect: adb connect 192.168.8.206:43677
connected to 192.168.8.206:43677
Only one device detected; using 192.168.8.206:43677
Selected device 192.168.8.206:43677
Target android device ID is authorized: 192.168.8.206:43677

________________________________________________________________
Starting pcap
________________________________________________________________

Pausing to check if pcap started...
tcpdump: wlp0s20f3: You don't have permission to perform this capture on that device
(socket: Operation not permitted)
Pcap did not start, you might need root; please authorize if prompted.
[sudo] password for user: 

Retrying pcap start...
tcpdump: listening on wlp0s20f3, link-type EN10MB (Ethernet), snapshot length 262144 bytes
Pcap started, output in /home/user/idt/IDT_ARTIFACTS/idt_20230831_010707/pcap/idt_20230831_010721_pcap.cap

________________________________________________________________
Starting capture for PlayServices
________________________________________________________________

android_model: Pixel 6
android_version: 14
android_api: 34
build_fingerprint: 
odm_build_fingerprint: 
product_build_fingerprint: 
vendor_build_fingerprint: 
display_width: 1080
display_height: 2400
container_info: versionName=23.31.16 (190400-555695945)
    versionName=23.18.18 (190408-535401451)
home_module_info: com.google.android.gms.home [v233116000]
optional_home_module_info: com.google.android.gms.optional_home [233116065] [23.31.16 (100400-0)] [Download:0000003a/dl-Home.optional_233116100400.apk] [download:/Home.optional/233116100400:Home.optional:233116100400]
policy_home_module_info: 
thread_info: com.google.android.gms.threadnetwork [v233116000]
mdns_info: com.google.android.gms.mdns [v214]
gha_info: versionName=3.5.1.4
Screen recording started /home/user/idt/IDT_ARTIFACTS/idt_20230831_010707/Android/idt_20230831_010720_screencast.mp4
logcat started /home/user/idt/IDT_ARTIFACTS/idt_20230831_010707/Android/idt_20230831_010720_logcat.txt

________________________________________________________________
Starting capture for PlayServicesUser
________________________________________________________________


________________________________________________________________

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
Press enter to stop streaming
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

________________________________________________________________

________________________________________________________________
Stopping pcap
________________________________________________________________

Pcap stopped

________________________________________________________________
Stopping capture for PlayServices
________________________________________________________________

screen proc killed
screen proc pull attempted
logcat killed

________________________________________________________________
Stopping capture for PlayServicesUser
________________________________________________________________

WARNING adb shell screenrecord /mnt/sdcard/idt_20230831_010720_screencast.mp4 stop requested while not running
screen proc killed
screen proc pull attempted
WARNING adb -s 192.168.8.206:43677 logcat -T 1 >> /home/user/idt/IDT_ARTIFACTS/idt_20230831_010707/Android/idt_20230831_010720_logcat.txt stop requested while not running
logcat killed

________________________________________________________________
Analyze + collate capture for PlayServices
________________________________________________________________


************************
Matter commissioner logs
08-31 01:07:49.599 12151 12151 I MatterCommissioner: Starting commissioning flow for sessionId: -5819674226130038987 and device identifier: Discriminator L[0xF00]. [CONTEXT service_id=336 ]
08-31 01:07:49.603 12151 12151 I MatterCommissioner: Retrieved commissioner credentials. [CONTEXT service_id=336 ]
08-31 01:07:49.641 12151 12151 I MatterCommissioner: Looking for device with discriminator L[0xF00] [CONTEXT service_id=336 ]
08-31 01:07:49.663 12151 12151 I MatterCommissioner: Discovered device (VID=0xFFF1, PID=0x8000) [mDNS Address=fe80::a3a:f2ff:fe42:e210%wlan0:5540], attempting to connect. [CONTEXT service_id=336 ]
08-31 01:07:55.903 12151 12151 I MatterCommissioner: Timeout connecting to (VID=0xFFF1, PID=0x8000) [mDNS Address=fe80::a3a:f2ff:fe42:e210%wlan0:5540]. May be a stale advertisement. Skipping. [CONTEXT service_id=336 ]
08-31 01:07:55.905 12151 12151 I MatterCommissioner: Discovered device (VID=0xFFF1, PID=0x8000) [BLE Address=C2:EC:88:AF:DA:27], attempting to connect. [CONTEXT service_id=336 ]
08-31 01:07:59.620 12151 12151 I MatterCommissioner: Successfully connected to (VID=0xFFF1, PID=0x8000) [BLE Address=C2:EC:88:AF:DA:27] [CONTEXT service_id=336 ]
08-31 01:07:59.654 12151 12151 I MatterCommissioner: Connected to device m.efw@3482246 [CONTEXT service_id=336 ]
08-31 01:07:59.933 12151 12151 I MatterCommissioner: Retrieved DeviceDescriptor (VID=0xFFF1, PID=0x8000) [CONTEXT service_id=336 ]
08-31 01:08:00.341 12151 12151 I MatterCommissioner: Resolved 0 existing fabric(s). [CONTEXT service_id=336 ]
08-31 01:08:00.653 12151 12151 I MatterCommissioner: Retrieved supported network types: [WIFI] [CONTEXT service_id=336 ]
08-31 01:08:00.655 12151 12151 I MatterCommissioner: Generating device credentials. [CONTEXT service_id=336 ]
08-31 01:08:01.922 12151 12151 I MatterCommissioner: Retrieved commissioning nonces. [CONTEXT service_id=336 ]
08-31 01:08:02.271 12151 12151 I MatterCommissioner: Armed failsafe for 120s. [CONTEXT service_id=336 ]
08-31 01:08:02.765 12151 12151 I MatterCommissioner: Set regulatory config to US. [CONTEXT service_id=336 ]
08-31 01:08:03.303 12151 12151 I MatterCommissioner: Retrieved DAC chain. [CONTEXT service_id=336 ]
08-31 01:08:03.807 12151 12151 I MatterCommissioner: Retrieved PAI chain. [CONTEXT service_id=336 ]
08-31 01:08:04.518 12151 12151 I MatterCommissioner: Retrieved attestation response. [CONTEXT service_id=336 ]
08-31 01:08:04.518 12151 12151 I MatterCommissioner: Retrieved attestation challenge. [CONTEXT service_id=336 ]
08-31 01:08:05.240 12151 12151 I MatterCommissioner: Retrieved CsrResponse. [CONTEXT service_id=336 ]
08-31 01:08:07.686 12151 12151 I MatterCommissioner: Adding root certificate. [CONTEXT service_id=336 ]
08-31 01:08:08.304 12151 12151 I MatterCommissioner: Added root certificate. [CONTEXT service_id=336 ]
08-31 01:08:08.305 12151 12151 I MatterCommissioner: Adding NOC to device. [CONTEXT service_id=336 ]
08-31 01:08:09.518 12151 12151 I MatterCommissioner: Added NOC for fabric ID 9D61F8B3F5A54462 [CONTEXT service_id=336 ]
08-31 01:08:09.519 12151 12151 I MatterCommissioner: Updating Wi-Fi network on device. [CONTEXT service_id=336 ]
08-31 01:08:09.695 12151 12151 I MatterCommissioner: Added Wi-Fi network. [CONTEXT service_id=336 ]
08-31 01:08:13.567 12151 12151 I MatterCommissioner: Connected to Wi-Fi network. [CONTEXT service_id=336 ]
08-31 01:08:13.587 12151 12151 I MatterCommissioner: Establishing a CASE connection. (1 of 3) [CONTEXT service_id=336 ]
08-31 01:08:20.795 12151 12151 W MatterCommissioner: Failed to establish CASE connection; retrying (1 of 3) in 1,000 ms. [CONTEXT service_id=336 ]
08-31 01:08:20.795 12151 12151 W MatterCommissioner: m.eqp: Failed to establish CASE session for device 00000000C6B81913
08-31 01:08:20.795 12151 12151 W MatterCommissioner:    at m.emt.onConnectionFailure(:com.google.android.gms.optional_home@233116065@23.31.16 (100400-0):52)
08-31 01:08:20.795 12151 12151 W MatterCommissioner: Caused by: chip.devicecontroller.ChipDeviceControllerException: third_party/connectedhomeip/next/src/protocols/secure_channel/CASESession.cpp:494: CHIP Error 0x00000032: Timeout
08-31 01:08:21.798 12151 12151 I MatterCommissioner: Establishing a CASE connection. (2 of 3) [CONTEXT service_id=336 ]
08-31 01:08:26.316 12151 12151 W MatterCommissioner: Failed to establish CASE connection; retrying (2 of 3) in 2,000 ms. [CONTEXT service_id=336 ]
08-31 01:08:26.316 12151 12151 W MatterCommissioner: m.eqp: Failed to establish CASE session for device 00000000C6B81913
08-31 01:08:26.316 12151 12151 W MatterCommissioner:    at m.emt.onConnectionFailure(:com.google.android.gms.optional_home@233116065@23.31.16 (100400-0):52)
08-31 01:08:26.316 12151 12151 W MatterCommissioner: Caused by: chip.devicecontroller.ChipDeviceControllerException: third_party/connectedhomeip/next/src/protocols/secure_channel/CASESession.cpp:1965: CHIP Error 0x000000AC: Internal error
08-31 01:08:28.319 12151 12151 I MatterCommissioner: Establishing a CASE connection. (3 of 3) [CONTEXT service_id=336 ]
08-31 01:08:28.671 12151 12151 E MatterCommissioner: Giving up CASE connection after 3 attempts. [CONTEXT service_id=336 ]
08-31 01:08:28.671 12151 12151 E MatterCommissioner: m.eqp: Failed to establish CASE session for device 00000000C6B81913
08-31 01:08:28.671 12151 12151 E MatterCommissioner:    at m.emt.onConnectionFailure(:com.google.android.gms.optional_home@233116065@23.31.16 (100400-0):52)
08-31 01:08:28.671 12151 12151 E MatterCommissioner: Caused by: chip.devicecontroller.ChipDeviceControllerException: third_party/connectedhomeip/next/src/protocols/secure_channel/CASESession.cpp:1965: CHIP Error 0x000000AC: Internal error
08-31 01:08:28.672 12151 12151 I MatterCommissioner: Disarming failsafe. [CONTEXT service_id=336 ]
08-31 01:08:29.079 12151 12151 I MatterCommissioner: Could not disarm failsafe (error=0xAC) [CONTEXT service_id=336 ]


*********************************
Commissioning failure stack trace
08-31 01:08:29.084 12151 12151 E SetupDeviceViewModel: Commissioning failed with state Failed to discover operational device. [CONTEXT service_id=336 ]


**************
PASE Handshake
08-31 01:07:58.272 12151 21175 I native  : I0000 00:00:1693469278.272252   21175 chip_logging.cc:17] CHIP: EM: <<< [E:5552i S:0 M:77517238] (U) Msg TX to 0:0000000000000000 [0000] --- Type 0000:22 (SecureChannel:PASE_Pake1)
08-31 01:07:59.476 12151 18083 I native  : I0000 00:00:1693469279.476862   18083 chip_logging.cc:17] CHIP: EM: >>> [E:5552i S:0 M:143137737] (U) Msg RX from 0:0000000000000000 [0000] --- Type 0000:23 (SecureChannel:PASE_Pake2)
08-31 01:07:59.483 12151 18083 I native  : I0000 00:00:1693469279.483611   18083 chip_logging.cc:17] CHIP: EM: <<< [E:5552i S:0 M:77517239] (U) Msg TX to 0:0000000000000000 [0000] --- Type 0000:24 (SecureChannel:PASE_Pake3)


***************
mDNS resolution
08-31 01:08:13.591 12151 12187 I ServiceResolverAdapter: Resolving address for 61701BF6702881AF-00000000C6B81913._matter._tcp [CONTEXT service_id=336 ]
08-31 01:08:14.550 12151 12187 I ServiceResolverAdapter: Resolved 192.168.8.128[5540] for 61701BF6702881AF-00000000C6B81913._matter._tcp [CONTEXT service_id=336 ]
08-31 01:08:21.801 12151 12187 I ServiceResolverAdapter: Resolving address for 61701BF6702881AF-00000000C6B81913._matter._tcp [CONTEXT service_id=336 ]
08-31 01:08:21.820 12151 12187 I ServiceResolverAdapter: Resolved fe80::a3a:f2ff:fe42:e210%47[5540] for 61701BF6702881AF-00000000C6B81913._matter._tcp [CONTEXT service_id=336 ]
08-31 01:08:28.322 12151 12187 I ServiceResolverAdapter: Resolving address for 61701BF6702881AF-00000000C6B81913._matter._tcp [CONTEXT service_id=336 ]
08-31 01:08:28.341 12151 12187 I ServiceResolverAdapter: Resolved fe80::a3a:f2ff:fe42:e210%47[5540] for 61701BF6702881AF-00000000C6B81913._matter._tcp [CONTEXT service_id=336 ]
08-31 01:08:28.675 12151 12187 I ServiceResolverAdapter: Resolving address for 61701BF6702881AF-00000000C6B81913._matter._tcp [CONTEXT service_id=336 ]
08-31 01:08:28.695 12151 12187 I ServiceResolverAdapter: Resolved fe80::a3a:f2ff:fe42:e210%47[5540] for 61701BF6702881AF-00000000C6B81913._matter._tcp [CONTEXT service_id=336 ]


**************
CASE handshake
08-31 01:08:14.554 12151 12187 I native  : I0000 00:00:1693469294.554950   12187 chip_logging.cc:17] CHIP: EM: <<< [E:5572i S:0 M:77517240] (U) Msg TX to 0:0000000000000000 [0000] --- Type 0000:30 (SecureChannel:CASE_Sigma1)
08-31 01:08:14.555 12151 12187 I native  : I0000 00:00:1693469294.555289   12187 chip_logging.cc:17] CHIP: SC: Sent Sigma1 msg
08-31 01:08:22.008 12151 26045 I native  : I0000 00:00:1693469302.008939   26045 chip_logging.cc:17] CHIP: EM: <<< [E:5573i S:0 M:77517241] (U) Msg TX to 0:0000000000000000 [0000] --- Type 0000:30 (SecureChannel:CASE_Sigma1)
08-31 01:08:22.010 12151 26045 I native  : I0000 00:00:1693469302.010948   26045 chip_logging.cc:17] CHIP: SC: Sent Sigma1 msg
08-31 01:08:28.523 12151 26045 I native  : I0000 00:00:1693469308.522964   26045 chip_logging.cc:17] CHIP: EM: <<< [E:5574i S:0 M:77517243] (U) Msg TX to 0:0000000000000000 [0000] --- Type 0000:30 (SecureChannel:CASE_Sigma1)
08-31 01:08:28.523 12151 26045 I native  : I0000 00:00:1693469308.523758   26045 chip_logging.cc:17] CHIP: SC: Sent Sigma1 msg
08-31 01:08:28.882 12151 26045 I native  : I0000 00:00:1693469308.882481   26045 chip_logging.cc:17] CHIP: EM: <<< [E:5575i S:0 M:77517245] (U) Msg TX to 0:0000000000000000 [0000] --- Type 0000:30 (SecureChannel:CASE_Sigma1)
08-31 01:08:28.885 12151 26045 I native  : I0000 00:00:1693469308.884953   26045 chip_logging.cc:17] CHIP: SC: Sent Sigma1 msg


________________________________________________________________
Analyze + collate capture for PlayServicesUser
________________________________________________________________

Output zip: /home/user/idt/IDT_ARTIFACTS/idt_20230831_010707.zip
```

#### Artifacts

Each ecosystem and platform involved in the capture will have their own subdirectory in the root artifact dir.

To download your artifacts, run these commands from your admin computer:

`idt_fetch_artifacts`

On windows admin computers, you may use `FileZilla` to pull the archive listed at the end of output.

### Discovery

```commandline
idt discover -h

usage: idt discover [-h] --type {ble,b,mdns,m}

options:
  -h, --help            show this help message and exit
  --type {ble,b,mdns,m}, -t {ble,b,mdns,m}
                        Specify the type of discovery to execute
```

#### ble

```
idt discover -t b

________________________________________________________________
Using artifact dir /home/user/idt/IDT_ARTIFACTS/idt_20230831_012654
________________________________________________________________

2023-08-31 01:26:54,792.792 WARNING {matter_ble} [browse_interactive]
Scanning BLE
DCL Lookup: https://webui.dcl.csa-iot.org/ 

2023-08-31 01:27:00.836
DISCOVERED
EveEnergy511CC DE:4E:24:DF:4C:370000fff6-0000-1000-8000-00805f9b34fb
00cc010a13530001
RSSI -58
VID: 130a PID: 0053


2023-08-31 01:27:00.836
DISCOVERED
48-22-54-5F-33-BC 48:22:54:5F:33:BC0000fff6-0000-1000-8000-00805f9b34fb
00760a8811010101
RSSI -47
VID: 1188 PID: 0101


2023-08-31 01:27:37.165
LOST EveEnergy511CC_DE:4E:24:DF:4C:37


2023-08-31 01:27:55.343
DISCOVERED
FD-1F-29-7E-1B-31 FD:1F:29:7E:1B:310000fff6-0000-1000-8000-00805f9b34fb
00000ff1ff008000
RSSI -67
VID: fff1 PID: 8000


2023-08-31 01:28:13.519
LOST 48-22-54-5F-33-BC_48:22:54:5F:33:BC


^C
```

#### mDNS

```
 idt discover -t m

________________________________________________________________
Using artifact dir /home/user/idt/IDT_ARTIFACTS/idt_20230831_013934
________________________________________________________________

Browsing Matter mDNS, press enter to stop
2023-08-31 01:39:35,562.562 INFO {matter_mdns} [add_service]
ServiceInfo(type='_matterc._udp.local.', name='06AEDAC2B84C4599._matterc._udp.local.', addresses=[b'\xc0\xa8\x08\x9c'], port=5540, weight=0, priority=0, server='4822545F33BC.local.', properties={b'VP': b'4488+257', b'DT': b'266', b'DN': b'Mini Smart Wi-Fi Plug', b'SII': b'5000', b'SAI': b'300', b'T': b'1', b'D': b'2678', b'CM': b'0', b'RI': b'0900C680E972C4C1A25BA2F6B532BDEC42C3', b'PH': b'36', b'PI': b''}, interface_index=None)
COMMISSIONABLE
VID: 0x1188 PID: 0x101
 

2023-08-31 01:39:35,563.563 INFO {matter_mdns} [add_service]
ServiceInfo(type='_matter._tcp.local.', name='61701BF6702881AF-0000000081854C07._matter._tcp.local.', addresses=[b'\xc0\xa8\x08\x80'], port=5540, weight=0, priority=0, server='083AF242E210.local.', properties={b'T': b'1'}, interface_index=None)
OPERATIONAL 

2023-08-31 01:39:35,564.564 INFO {matter_mdns} [add_service]
ServiceInfo(type='_matter._tcp.local.', name='61701BF6702881AF-0000000068DB496F._matter._tcp.local.', addresses=[b'\xc0\xa8\x08\x9c'], port=5540, weight=0, priority=0, server='4822545F33BC.local.', properties={b'SII': b'5000', b'SAI': b'300', b'T': b'1'}, interface_index=None)
OPERATIONAL 

2023-08-31 01:39:35,664.664 INFO {matter_mdns} [add_service]
ServiceInfo(type='_matter._tcp.local.', name='61701bf6702881af-000000001a3fbb61._matter._tcp.local.', addresses=[b'\xc0\xa8\x08\xba'], port=5540, weight=0, priority=0, server='44070B4C4C2E.local.', properties={}, interface_index=None)
OPERATIONAL 

2023-08-31 01:39:35,668.668 INFO {matter_mdns} [add_service]
ServiceInfo(type='_meshcop._udp.local.', name='Google Nest Hub Max (a6f7)._meshcop._udp.local.', addresses=[b'\xc0\xa8\x08\xba'], port=49154, weight=0, priority=0, server='fuchsia-4407-0b4c-4c2e.local.', properties={b'vcd': b'106A6D1B9B99FEAA301026F7D2DDE8CD', b'id': b'\x10jm\x1b\x9b\x99\xfe\xaa0\x10&\xf7\xd2\xdd\xe8\xcd', b'vo': b'|\xd9\\', b'vvo': b'7CD95C', b'vxp': b'1dc3269b3297a6d6', b'vat': b'000064cd6e100d51', b'rv': b'1', b'tv': b'1.3.0', b'sb': b'\x00\x00\x001', b'nn': b'NEST-PAN-C15C', b'xp': b'\x1d\xc3&\x9b2\x97\xa6\xd6', b'vn': b'Google Inc.', b'mn': b'Google Nest Hub Max', b'xa': b'\x02\x08\xbd\xc7\x10N\xa6\xf7', b'at': b'\x00\x00d\xcdn\x10\rQ', b'pt': b'y\x16N\x9f'}, interface_index=None)
THREAD_BORDER_ROUTER 

2023-08-31 01:39:35,669.669 INFO {matter_mdns} [add_service]
ServiceInfo(type='_matter._tcp.local.', name='61701BF6702881AF-00000000031F052C._matter._tcp.local.', addresses=[], port=5540, weight=0, priority=0, server='2A188ACDA87BA8E7.local.', properties={b'SII': b'800', b'SAI': b'800', b'T': b'0'}, interface_index=None)
OPERATIONAL 
```

#### Artifacts

There is a per device log for ble scanning in `ble` subdirectory of the root artifact dir.

[TODO] mDNS per device log

## Contributing

See the README in `/capture/ecosystem` for guidance on adding new ecosystems.

`/capture/ecosystem/play_services_user` contains a minimal example implementation.
