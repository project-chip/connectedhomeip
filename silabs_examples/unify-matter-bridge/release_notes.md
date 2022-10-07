# Matter Bridge Release Notes

## [0.4.0] - Oct 12th 2022

### Fixed

- Fixed the issue of the bridge service does not persists mapping between endpoint numbers and
  unify UNIDs. As a consequence the endpoints may swap ids on each program
  startup.

- Fixed the issue of group mapping does not check for existing unify groups. As a consequence the
  matter bridge may use an already assigned unify group.

### Added

- Added support for the following clusters:
  - Color Control
  - Temperature Measurement
  - Occupancy Sensing

- Added a commissioner command to the CLI when running the unify-matter-bridge.

### Known Issues

- Endpoint 0 of the bridge shows support for the Identify cluster, but the
  identify commands has no effects.
- When using Google Home with unify-matter-bridge only commissioning works, all
  Unify nodes appear as offline after commissioning.

## [0.1.0] - Aug 8th 2022

### Fixed

### Added

- The Bridge has only support for a limited number of clusters, but it will
  present device types of with unsupported cluster never the less. The bridge
  supported mapping of the following clusters:
  - Bridge Device Information
  - Level
  - OnOff
  - Identify
  - Group

### Known Issues

- The bridge service does not persists mapping between endpoint numbers and
  unify UNIDs. As a consequence the endpoints may swap ids on each program
  startup.

- Group mapping does not check for existing unify groups. As a consequence the
  matter bridge may use an already assigned unify group.

- The bridge service does not persists its matter fabric credentials between
  system reboots. After RPi reboot, the bridge will always be noncommissioned.
  This can be fix by choosing a proper location for the KVS file using the
  `--kvs` command line flag.

- Endpoint 0 of the bridge shows support for the Identify cluster, but the
  identify commands has no effects.

- The version chip-tool provided with the unify matter bridge is unable to send
  group cast messages. As a workaround the chip-tool from the matter master
  branch can be used.
