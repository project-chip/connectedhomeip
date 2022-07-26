# Using the Pre-Built Rasberry Pi "Matter Hub" Image
When using a Raspberry Pi as a controller in your Matter network you have two options 

# Building Environment using Raspberry Pi 4

To flash the Ubuntu OS onto the SD card:

1. Insert the flashed SD card (directly or using a card reader) into the
   laptop/PC that will run the Raspberry Pi Imager tool
2. Launch Raspberry Pi 4 Imager
3. Click on 'Choose OS' --> 'Other General-purpose OS' --> 'Ubuntu' --> 'Ubuntu 22.04 64-bit server OS'
4. Click 'Storage' and select the 'SD card detect'
5. This Raspberry Pi 4's console can be accessed in multiple ways, refer to:
   https://www.raspberrypi.com/documentation/computers/remote-access.html
   In this guide, Raspberry Pi 4 is being accessed using Putty. 
6. Enter the
   details like User name, Password, SSID and its password to connect to
   network. Click 'Save'
7. Click 'Write' and then 'Yes' when you are asked for permission to erase data
   on the SD card. It will then start flashing the OS onto the SD card
8. When it is done, click 'Continue'.
9. Remove the SD card from the reader and insert it into the Raspberry Pi as
    shown below:

<!-- ![Inserting SD into Pi](images/sd_into_pi.png) -->
<img src="images/sd_into_pi.png" alt="Inserting SD into Pi" width="550"/>

On powering up the board, the red and green lights should start blinking. To start
using the Raspberry Pi:

1. Use this reference to find the IP address of your Raspberry Pi: [Finding Your Raspberry Pi](../general/FIND_RASPI.md)
2. Once you find the IP address, launch Putty, select 'Session', enter the IP
    address of the Raspberry Pi, and click 'Open'
3. Enter the username and password given at the time of flashing and click
    'Enter'
    If you don't give username and password default is user account "ubuntu" and password "ubuntu",

4. Update the latest packages by running following commands in the terminal:

    > `$ sudo apt update`

    > `$ sudo apt install`

5. Install required packages using the following commands:

    > `$ sudo apt-get install git gcc g++ pkg-config libssl-dev libdbus-1-dev \
     libglib2.0-dev libavahi-client-dev ninja-build python3-venv python3-dev \
     python3-pip unzip libgirepository1.0-dev libcairo2-dev libreadline-dev'

    <br>
    > If you see any popups between installs, you can select 'Ok' or 'Continue'

## Installing prerequisites on Raspberry Pi 4
   Finally, install some Raspberry Pi specific dependencies:

In [this link to the Project CHIP GitHub Site](https://github.com/project-chip/connectedhomeip/blob/master/docs/guides/BUILDING.md), follow the instructions in
the section "Installing prerequisites on Raspberry Pi 4".

## Bluetooth Setup
   Make sure BLE is up and running on Raspberry Pi.
   Raspberry Pi internally has some issues with BLE that may cause
   it to crash. Because BLE is used for
   commissioning on Matter, make sure BLE is running.

  `$ sudo systemctl status bluetooth.service`

   To stop BLE if it is already running:    `$ sudo systemctl stop bluetooth.service`

   To restart the Bluetooth service, first enable it: 
   `$ sudo systemctl enable bluetooth.service`

   When you check the status of the Bluetooth service, it will be inactive because it has been enabled but not restarted: 
   `$ sudo systemctl status bluetooth.service`

   Restart the service:    `$ sudo systemctl restart bluetooth.service`

   Now the status of the service should be active and running:   `$ sudo systemctl status bluetooth.service`

----
[Table of Contents](../README.md) | [Thread Demo](../thread/DEMO_OVERVIEW.md) | [Wi-Fi Demo](./DEMO_OVERVIEW.md)