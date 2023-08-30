# Silicon Labs Matter New Features

## New Features for v2.2.0-1.2-alpha.1

### Support for Intermittently Connected Devices (ICD)

- Full support for ICD Short idle time (SIT) Devices in support of the Matter 1.2 specification.
  - In this release Silicon Labs has provided full support for Short Idle Time intermittently connected devices. 
  - These are ICDs (formerly called Sleepy End Devices) which must remain responsive to user input such as Door Locks and Window Coverings.
- ICD Management cluster server implementation
  - Silicon Labs has provided an implementation of the ICD cluster server and the configuration of the ICD
- ICD Manager and ICD Event manager has been implemented to manage the Idle and Active mode of the ICD
- NEW DNS advertisement Text Key SAI: indicates the SLEEPY_ACTIVE_INTERVAL (default to 4000 ms when ICD is not enabled)
- NEW Matter ICD configuration defines:
	- CHIP_CONFIG_ICD_IDLE_MODE_INTERVAL set value for the ICD IdleInterval attribute
	- CHIP_CONFIG_ICD_ACTIVE_MODE_INTERVAL set value for the ICD ActiveInterval attribute
	- CHIP_CONFIG_ICD_ACTIVE_MODE_THRESHOLD set value for the ICD ActiveThreshold attribute
	- CHIP_CONFIG_ICD_CLIENTS_SUPPORTED_PER_FABRIC set value for the ICD ClientsSupportedPerFabric attribute
    - All of these defines can be configured by our following build arguments (default values listed here)
      - sl_idle_mode_interval_ms = 600000  `# 10min Idle Mode Interval`
      - sl_active_mode_interval_ms = 1000  `# 1s Active Mode Interval`
      - sl_active_mode_threshold_ms = 500  `# 500ms Active Mode Threshold`
      - sl_icd_supported_clients_per_fabric = 2  `# 2 registration slots per fabric`
    - The OpenThread polling rates used in either ICD mode can be configured with (default value listed here)
      - sl_ot_idle_interval_ms = 15000  `# 15s Idle Intervals`
      - sl_ot_active_interval_ms = 200  `# 200ms Active Intervals`
	
- CHANGES:
  - Optimized the subscription reports by synchronizing all client’s subscriptions with the ICD idle mode interval. This ensures the minimal amount of wake ups possible due to subscription reports
  - The previous `--sed` build preset has been replaced by `--icd`. This goes in line with previous sleepy end device behavior being deprecated and replaced by the ICD behavior.
  - Silicon Labs' Light Switch and Door Lock apps support the ICD implementation and have the ICD cluster enabled. To build those apps as ICDs, use the aforementioned prefix `--icd` in your usual build command.
    - e.g.: `./scripts/examples/gn_silabs_example.sh ./examples/light-switch-app/silabs/efr32/ ./out/light-switch-app_ICD BRD4187C --icd`
## New Features for v2.1.0-1.1
- ### Update to GSDK Version 4.2.3

  The GSDK Version 4.2.3 includes important security enhancements for the Open Thread Stack which in turn updates all Matter over Thread implementations.

## New Features for v2.0.0-1.1

### Matter Intermittently Connected Devices (ICD / Sleepy) Improvements
- #### Expose the API for a device to change the max interval in a subscription request
    A device can, when accepting a subscription request, change the maximum reporting interval for it to match with its idle time instead of accepting the requested intervals.
When not using this API, the ICDs idle time interval could functionally be shortened by a controller without the ICD being able to refuse the subscription.

    For more details see the [Matter Intermittently Connected Devices](./general/MATTER_ICD.md) section.

- #### Persistent Subscription
    The device will persist a subscription to be able to recover its subscription with a subscriber in case of a reboot.

    For more details see the [Matter Intermittently Connected Devices](./general/MATTER_ICD.md) section.

- #### During the commissioning flow, the device will not be able to go to idle mode until the commissioning is complete.
    Before, the thread devices would go to idle mode right after completing its srp registration but before the sigma1 message was received.
With 1.1, devices will remain in Active Mode until the commissioning is complete.

- #### Adding the Active Mode Threshold feature.
    When a thread ICD goes from Idle Mode to Active Mode, the ICD will stay in Active Mode for a defined amount before going back to Idle Mode.
The timer starts after the last communication.
For this feature to take affect, there needs to be at least one message coming in or going out. If the device just polls its thread router and nothing else happens, this doesn't come into play.

    For more details see the [Openthread Sleepy End Device](./general/OT_SLEEPY_END_DEVICE.md) section.

### Matter Sleepy End Devices over Wi-Fi

- A Matter Sleepy End Device is a device where the EFR32 along with the Wi-Fi coprocessor go into sleep mode during idle time resulting in less power consumption. This helps devices running on limited power sources like battery powered devices.
The EFR32 goes into sleep mode whenever the device is idle. The Wi-Fi coprocessor goes into Power Save (PS) Poll legacy power save mode where it wakes for every Delivery Traffic Information Map (DTIM) interval to check for any packets.

    For more details see the [Wi-Fi Sleepy End Device](wifi/WIFI_SLEEPY_END_DEVICE.md) section.

### Added support for CMPS (Custom Part Manufacturing Service)

- The new provisioning script allows easy setup of factory data in Matter devices. This tool provides a common architecture for use both in development and production environments, which allows easy deployment of the developed products.

    The provisioning script may be used to set all factory settings, including Discriminator, Passcode, and the attestation credentials (PAI, DAC and CD).

    The provision/provision.py tool supersedes the silabs_example/credentials/creds.py script.

### Matter Wi-Fi Direct Internet Connectivity

- The Matter Wi-Fi Direct Internet Connectivity (DIC) solution connects to proprietary cloud solutions for direct access to software downloads, to device controls, or to aggregate statistics on where and how the device is being used.

    For more information on this feature see the [Matter Wi-Fi Direct Internet Connectivity(DIC)](wifi/DIC_Wi-Fi.md) section.
