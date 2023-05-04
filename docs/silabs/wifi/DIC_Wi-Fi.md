# Matter Wi-Fi Direct Internet Connectivity
-  Direct Internet Connectivity (DIC) is a silabs only feature to connect matter devices to proprietary cloud solutions(AWS,GCP,APPLE ...) directly.  As such, a Matter Wi-Fi device must support connecting locally on the Matter Fabric, via IPv6, and connecting to the Internet via IPv4.
-  Matter devices can be controlled by chip-tool or controller and the respective status of the attribute modified will be published to the cloud.
-  Remote user can install the cloud specific application to get the notifiction on the attribute status.

## Feature Design
![Silicon Labs - DIC design](./images/END_to_END_FLOW.png)

## End-to-End Set-up bring up
## Message Queuing Telemetry Transport (MQTT)
- MQTT is an OASIS standard messaging protocol for the Internet of Things (IoT). It is designed as an extremely lightweight publish/subscribe messaging transport that is ideal for connecting remote devices with a small code footprint and minimal network bandwidth. Refer https://mqtt.org/ for more details

### Configuration of MQTT server
To set up and configure AWS or Mosquitto for DIC support please see the following documentation: 
   - [AWS installation](./AWS_CONFIGURATION_REGISTERATION.md)
   - [Mosquitto installation](./MOSQUITTO_SETUP.md)
### Remote User Setup (MQTT Explorer)
     
  - A remote user is used to check the state of device. In this context MQTT explorer is used as a remote user.
  - [MQTT explorer setup and configuration](./MQTT_EXPLORER_SETUP.md)

### DUT Setup
 - Software and Hardware prerequisites for working with Silicon Labs Matter.
1. [Matter Hardware Requirements](../general/HARDWARE_REQUIREMENTS.md)
2. [Matter Software Requirements](../general/SOFTWARE_REQUIREMENTS.md)
3. Use the build command for DIC instead of Normal build command which one present in SW_SETUP.md [Don't use normal build comands](./SW_SETUP.md)
 - [Build for DIC](./BUILD_DIC.md)

## End-to-End Test of DIC Light Application

- User Setup (MQTT Explorer)
    - For executing with efr32, refer to the following: [Running the Matter Demo on EFR32 hosts](RUN_DEMO.md) 
    - For executing with SiWx917 SoC, refer to the following: [Running the Matter Demo on SiWx917 SoC](RUN_DEMO_SiWx917_SoC.md)
- In MQTT explorer under light/state topic, state of the light will be updated. Below is the screenshot for reference

![DIC STATUS UPDATE](./images/mqtt_explorer_4.png)
