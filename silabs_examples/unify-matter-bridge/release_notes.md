# Matter Bridge Release Notes

## [0.1] - Aug 8th 2022

- The bridge service does not persists mapping between endpoint numbers and unify UNIDs. As a consequence the endpoints may swap ids on each program startup.

- Group mapping does not check for existing unify groups. As a consequence the matter bridge may use an already assigned unify group.

- The bridge service does not persists its matter fabric credentials between system reboots. After RPi reboot, the bridge will always be noncommissioned. This can be fix by choosing a proper location for the KVS file using the `--kvs` command line flag.

- The Bridge has only support for a limited number of clusters, but it will present device types of with unsupported cluster never the less. The bridge supported mapping of the following clusters:
  - Bridge Device Information
  - Level
  - OnOff
  - Identify
  - Group

- Endpoint 0 of the bridge shows support for the Identify cluster, but the identify commands has no effects.

- The version chip-tool provided with the unify matter bridge is unable to send group cast messages. As a workaround the chip-tool from the matter master branch can be used.
