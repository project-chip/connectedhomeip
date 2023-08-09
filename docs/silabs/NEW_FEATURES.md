# Silicon Labs Matter New Features

## New Features for v2.1.0-1.1
- ### Update to GSDK Version 4.2.3

  The GSDK Version 4.2.3 includes important security enhancements for the Open Thread Stack which in turn updates all Matter over Thread implementations.

## New Features for v2.0.0-1.1

## Matter Intermittently Connected Devices (ICD / Sleepy) Improvements
- ### Expose the API for a device to change the max interval in a subscription request
    A device can, when accepting a subscription request, change the maximum reporting interval for it to match with its idle time instead of accepting the requested intervals.
When not using this API, the ICDs idle time interval could functionally be shortened by a controller without the ICD being able to refuse the subscription.

    For more details see the [Matter Intermittently Connected Devices](./general/MATTER_ICD.md) section.

- ### Persistent Subscription
    The device will persist a subscription to be able to recover its subscription with a subscriber in case of a reboot.

    For more details see the [Matter Intermittently Connected Devices](./general/MATTER_ICD.md) section.

- ### During the commissioning flow, the device will not be able to go to idle mode until the commissioning is complete.
    Before, the thread devices would go to idle mode right after completing its srp registration but before the sigma1 message was received.
With 1.1, devices will remain in Active Mode until the commissioning is complete.

- ### Adding the Active Mode Threshold feature.
    When a thread ICD goes from Idle Mode to Active Mode, the ICD will stay in Active Mode for a defined amount before going back to Idle Mode.
The timer starts after the last communication.
For this feature to take affect, there needs to be at least one message coming in or going out. If the device just polls its thread router and nothing else happens, this doesn't come into play.

    For more details see the [Openthread Sleepy End Device](./general/OT_SLEEPY_END_DEVICE.md) section.

## Matter Sleepy End Devices over Wi-Fi

- A Matter Sleepy End Device is a device where the EFR32 along with the Wi-Fi coprocessor go into sleep mode during idle time resulting in less power consumption. This helps devices running on limited power sources like battery powered devices.
The EFR32 goes into sleep mode whenever the device is idle. The Wi-Fi coprocessor goes into Power Save (PS) Poll legacy power save mode where it wakes for every Delivery Traffic Information Map (DTIM) interval to check for any packets.

    For more details see the [Wi-Fi Sleepy End Device](wifi/WIFI_SLEEPY_END_DEVICE.md) section.

## Added support for CMPS (Custom Part Manufacturing Service)

- The new provisioning script allows easy setup of factory data in Matter devices. This tool provides a common architecture for use both in development and production environments, which allows easy deployment of the developed products.

    The provisioning script may be used to set all factory settings, including Discriminator, Passcode, and the attestation credentials (PAI, DAC and CD).

    The provision/provision.py tool supersedes the silabs_example/credentials/creds.py script.

## Matter Wi-Fi Direct Internet Connectivity

- The Matter Wi-Fi Direct Internet Connectivity (DIC) solution connects to proprietary cloud solutions for direct access to software downloads, to device controls, or to aggregate statistics on where and how the device is being used.

    For more information on this feature see the [Matter Wi-Fi Direct Internet Connectivity(DIC)](wifi/DIC_Wi-Fi.md) section.
