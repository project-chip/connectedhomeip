# Matter Device Types Implementation Status

**Updated as of**: 2026-06-12 (Matter Specification SHA:
`5a31ae2acb487bea09286243ccb5ad4ca9d3ef08`)

## Discovery & Updating Methodology

To update or validate this list manually, follow these steps:

1. **Verify Implemented Device Types**:

    - Inspect
      `examples/all-devices-app/all-devices-common/device-factory/DeviceFactory.h`.
    - Look at the `DeviceFactory` constructor. Every
      `RegisterCreator("name", ...)` call corresponds to an implemented device
      type.
    - Note: `Root Node` (0x0016) is a special device type that is always
      implemented on Endpoint 0, and is represented by `RootNodeDevice.h/cpp`
      under `examples/all-devices-app/all-devices-common/devices/root-node/`.

2. **Verify Mandatory Server Clusters per Device Type**:

    - Locate the device type definition in the Matter Specification (e.g. under
      `device_library/` in the spec repository, or using generated markdown
      files under `out/spec/build/markdown/master/device_library/`).
    - Find the sub-section `### Cluster Requirements` for the device type.
    - Identify all server-side clusters (Client/Server = Server) with
      conformance `M` (Mandatory). These must be listed in the "Mandatory Server
      Clusters" column.

3. **Identify Missing Mandatory Clusters**:
    - Compare the "Mandatory Server Clusters" list with the implementation
      status in `supported_clusters.md`.
    - If a mandatory server cluster is marked as `Code-Driven in SDK = No` in
      `supported_clusters.md`, it is a "Missing Mandatory Cluster".
    - If all mandatory server clusters are `Code-Driven in SDK = Yes`, the
      device type is "None (Ready)".

## Implemented Device Types (18 total)

| #   | Device Type Name               | ID           | Notes |
| --- | ------------------------------ | ------------ | ----- |
| 1   | Aggregator                     | 14 (0x000E)  |       |
| 2   | Air Quality Sensor             | 44 (0x002C)  |       |
| 3   | Bridged Node                   | 19 (0x0013)  |       |
| 4   | Chime                          | 326 (0x0146) |       |
| 5   | Contact Sensor                 | 21 (0x0015)  |       |
| 6   | Dimmable Light                 | 257 (0x0101) |       |
| 7   | Fan                            | 43 (0x002B)  |       |
| 8   | Network Infrastructure Manager | 144 (0x0090) |       |
| 9   | Occupancy Sensor               | 263 (0x0107) |       |
| 10  | On/Off Light                   | 256 (0x0100) |       |
| 11  | Power Source                   | 17 (0x0011)  |       |
| 12  | Proximity Ranger               | 338 (0x0152) |       |
| 13  | Root Node                      | 22 (0x0016)  |       |
| 14  | Smoke CO Alarm                 | 118 (0x0076) |       |
| 15  | Soil Sensor                    | 69 (0x0045)  |       |
| 16  | Speaker                        | 34 (0x0022)  |       |
| 17  | Temperature Sensor             | 770 (0x0302) |       |
| 18  | Water Leak Detector            | 67 (0x0043)  |       |

## Unimplemented Device Types (86 total)

| #   | Device Type Name                  | ID            | Mandatory Server Clusters                                                          | Missing Mandatory Clusters                            | Notes |
| --- | --------------------------------- | ------------- | ---------------------------------------------------------------------------------- | ----------------------------------------------------- | ----- |
| 1   | AV Analysis Node                  | 329 (0x0149)  | None                                                                               | None (Ready)                                          |       |
| 2   | Air Purifier                      | 45 (0x002D)   | Identify, Fan Control                                                              | None (Ready)                                          |       |
| 3   | Ambient Context Sensor            | 336 (0x0150)  | None                                                                               | None (Ready)                                          |       |
| 4   | Arc Fault Circuit Interrupter     | 1301 (0x0515) | User Label, Power Topology                                                         | None (Ready)                                          |       |
| 5   | Audio Doorbell                    | 321 (0x0141)  | Identify, Switch, Camera AV Stream Management, WebRTC Transport Provider           | None (Ready)                                          |       |
| 6   | Basic Video Player                | 40 (0x0028)   | On/Off, Media Playback, Keypad Input                                               | Media Playback, Keypad Input                          |       |
| 7   | Battery Storage                   | 24 (0x0018)   | None                                                                               | None (Ready)                                          |       |
| 8   | Camera                            | 322 (0x0142)  | Camera AV Stream Management, WebRTC Transport Provider                             | None (Ready)                                          |       |
| 9   | Camera Controller                 | 327 (0x0147)  | WebRTC Transport Requestor                                                         | None (Ready)                                          |       |
| 10  | Casting Audio Player              | 33 (0x0021)   | Media Playback, Content Launcher, Chime                                            | Media Playback, Content Launcher                      |       |
| 11  | Casting Video Client              | 41 (0x0029)   | None                                                                               | None (Ready)                                          |       |
| 12  | Casting Video Player              | 35 (0x0023)   | On/Off, Media Playback, Keypad Input, Content Launcher                             | Media Playback, Keypad Input, Content Launcher        |       |
| 13  | Closure                           | 560 (0x0230)  | Identify, Closure Control                                                          | None (Ready)                                          |       |
| 14  | Closure Controller                | 574 (0x023E)  | None                                                                               | None (Ready)                                          |       |
| 15  | Closure Panel                     | 561 (0x0231)  | Closure Dimension                                                                  | None (Ready)                                          |       |
| 16  | Color Dimmer Switch               | 261 (0x0105)  | Identify                                                                           | None (Ready)                                          |       |
| 17  | Color Temperature Light           | 268 (0x010C)  | Identify, Groups, On/Off, Level Control, Scenes Management, Color Control          | Color Control                                         |       |
| 18  | Commissioning By Proxy            | 146 (0x0092)  | None                                                                               | None (Ready)                                          |       |
| 19  | Content App                       | 36 (0x0024)   | Keypad Input, Application Launcher, Application Basic                              | Keypad Input, Application Launcher, Application Basic |       |
| 20  | Control Bridge                    | 2112 (0x0840) | Identify                                                                           | None (Ready)                                          |       |
| 21  | Cook Surface                      | 119 (0x0077)  | None                                                                               | None (Ready)                                          |       |
| 22  | Cooktop                           | 120 (0x0078)  | On/Off                                                                             | None (Ready)                                          |       |
| 23  | Device Energy Management          | 1293 (0x050D) | Device Energy Management                                                           | None (Ready)                                          |       |
| 24  | Dimmable Plug-In Unit             | 267 (0x010B)  | Identify, Groups, On/Off, Level Control, Scenes Management                         | None (Ready)                                          |       |
| 25  | Dimmer Switch                     | 260 (0x0104)  | Identify                                                                           | None (Ready)                                          |       |
| 26  | Dishwasher                        | 117 (0x0075)  | Operational State                                                                  | Operational State                                     |       |
| 27  | Door Lock                         | 10 (0x000A)   | Identify, Door Lock                                                                | Door Lock                                             |       |
| 28  | Door Lock Controller              | 11 (0x000B)   | None                                                                               | None (Ready)                                          |       |
| 29  | Doorbell                          | 328 (0x0148)  | None                                                                               | None (Ready)                                          |       |
| 30  | Electrical Circuit Breaker        | 1302 (0x0516) | User Label, Power Topology                                                         | None (Ready)                                          |       |
| 31  | Electrical Distribution Enclosure | 1303 (0x0517) | Power Topology                                                                     | None (Ready)                                          |       |
| 32  | Electrical Energy Tariff          | 1299 (0x0513) | None                                                                               | None (Ready)                                          |       |
| 33  | Electrical Meter                  | 1300 (0x0514) | Electrical Power Measurement, Electrical Energy Measurement                        | None (Ready)                                          |       |
| 34  | Electrical Sensor                 | 1296 (0x0510) | Power Topology                                                                     | None (Ready)                                          |       |
| 35  | Electrical Surge Protector        | 1304 (0x0518) | User Label, Power Topology                                                         | None (Ready)                                          |       |
| 36  | Electrical Utility Meter          | 1297 (0x0511) | Meter Identification                                                               | Meter Identification                                  |       |
| 37  | Energy EVSE                       | 1292 (0x050C) | Energy EVSE, Energy EVSE Mode                                                      | Energy EVSE Mode                                      |       |
| 38  | Extended Color Light              | 269 (0x010D)  | Identify, Groups, On/Off, Level Control, Scenes Management, Color Control          | Color Control                                         |       |
| 39  | Extractor Hood                    | 122 (0x007A)  | Fan Control                                                                        | None (Ready)                                          |       |
| 40  | Floodlight Camera                 | 324 (0x0144)  | None                                                                               | None (Ready)                                          |       |
| 41  | Flow Sensor                       | 774 (0x0306)  | Identify, Flow Measurement                                                         | None (Ready)                                          |       |
| 42  | Generic Switch                    | 15 (0x000F)   | Identify, Switch                                                                   | None (Ready)                                          |       |
| 43  | Heat Pump                         | 777 (0x0309)  | None                                                                               | None (Ready)                                          |       |
| 44  | Heating/Cooling Unit              | 768 (0x0300)  | None                                                                               | None (Ready)                                          |       |
| 45  | Humidifier/Dehumidifier           | 125 (0x007D)  | None                                                                               | None (Ready)                                          |       |
| 46  | Humidity Sensor                   | 775 (0x0307)  | Identify, Relative Humidity Measurement                                            | None (Ready)                                          |       |
| 47  | Intercom                          | 320 (0x0140)  | Camera AV Stream Management, WebRTC Transport Provider, WebRTC Transport Requestor | None (Ready)                                          |       |
| 48  | Irrigation System                 | 64 (0x0040)   | None                                                                               | None (Ready)                                          |       |
| 49  | Joint Fabric Administrator        | 304 (0x0130)  | Joint Fabric Datastore, Joint Fabric Administrator                                 | Joint Fabric Datastore, Joint Fabric Administrator    |       |
| 50  | Laundry Dryer                     | 124 (0x007C)  | Operational State                                                                  | Operational State                                     |       |
| 51  | Laundry Washer                    | 115 (0x0073)  | Operational State                                                                  | Operational State                                     |       |
| 52  | Light Sensor                      | 262 (0x0106)  | Identify, Illuminance Measurement                                                  | None (Ready)                                          |       |
| 53  | Meter Reference Point             | 1298 (0x0512) | Identify                                                                           | None (Ready)                                          |       |
| 54  | Microwave Oven                    | 121 (0x0079)  | Operational State, Microwave Oven Mode, Microwave Oven Control                     | Operational State, Microwave Oven Mode                |       |
| 55  | Mode Select                       | 39 (0x0027)   | Mode Select                                                                        | Mode Select                                           |       |
| 56  | Mounted Dimmable Load Control     | 272 (0x0110)  | Identify, Groups, On/Off, Level Control, Scenes Management                         | None (Ready)                                          |       |
| 57  | Mounted On/Off Control            | 271 (0x010F)  | Identify, Groups, On/Off, Scenes Management                                        | None (Ready)                                          |       |
| 58  | OTA Provider                      | 20 (0x0014)   | OTA Software Update Provider                                                       | None (Ready)                                          |       |
| 59  | OTA Requestor                     | 18 (0x0012)   | OTA Software Update Requestor                                                      | None (Ready)                                          |       |
| 60  | On/Off Light Switch               | 259 (0x0103)  | Identify                                                                           | None (Ready)                                          |       |
| 61  | On/Off Plug-in Unit               | 266 (0x010A)  | Identify, Groups, On/Off, Scenes Management                                        | None (Ready)                                          |       |
| 62  | On/Off Sensor                     | 2128 (0x0850) | Identify                                                                           | None (Ready)                                          |       |
| 63  | Oven                              | 123 (0x007B)  | None                                                                               | None (Ready)                                          |       |
| 64  | Pressure Sensor                   | 773 (0x0305)  | Identify, Pressure Measurement                                                     | None (Ready)                                          |       |
| 65  | Pump                              | 771 (0x0303)  | Identify, On/Off, Pump Configuration and Control                                   | Pump Configuration and Control                        |       |
| 66  | Pump Controller                   | 772 (0x0304)  | Identify                                                                           | None (Ready)                                          |       |
| 67  | Rain Sensor                       | 68 (0x0044)   | Identify, Boolean State                                                            | None (Ready)                                          |       |
| 68  | Refrigerator                      | 112 (0x0070)  | None                                                                               | None (Ready)                                          |       |
| 69  | Residual Current Circuit Breaker  | 1305 (0x0519) | User Label, Power Topology                                                         | None (Ready)                                          |       |
| 70  | Robotic Vacuum Cleaner            | 116 (0x0074)  | Identify, RVC Run Mode, RVC Operational State                                      | RVC Run Mode, RVC Operational State                   |       |
| 71  | Room Air Conditioner              | 114 (0x0072)  | Identify, On/Off, Thermostat                                                       | Thermostat                                            |       |
| 72  | Secondary Network Interface       | 25 (0x0019)   | Network Commissioning                                                              | None (Ready)                                          |       |
| 73  | Snapshot Camera                   | 325 (0x0145)  | Camera AV Stream Management                                                        | None (Ready)                                          |       |
| 74  | Solar Power                       | 23 (0x0017)   | None                                                                               | None (Ready)                                          |       |
| 75  | Streaming Audio Player            | 32 (0x0020)   | Media Playback, Content Launcher, Chime                                            | Media Playback, Content Launcher                      |       |
| 76  | Temperature Controlled Cabinet    | 113 (0x0071)  | Temperature Control                                                                | None (Ready)                                          |       |
| 77  | Thermostat                        | 769 (0x0301)  | Identify, Thermostat                                                               | Thermostat                                            |       |
| 78  | Thermostat Controller             | 778 (0x030A)  | None                                                                               | None (Ready)                                          |       |
| 79  | Thread Border Router              | 145 (0x0091)  | Thread Network Diagnostics, Thread Border Router Management                        | None (Ready)                                          |       |
| 80  | Video Doorbell                    | 323 (0x0143)  | None                                                                               | None (Ready)                                          |       |
| 81  | Video Remote Control              | 42 (0x002A)   | None                                                                               | None (Ready)                                          |       |
| 82  | Water Freeze Detector             | 65 (0x0041)   | Identify, Boolean State                                                            | None (Ready)                                          |       |
| 83  | Water Heater                      | 1295 (0x050F) | Water Heater Management, Water Heater Mode, Thermostat                             | Water Heater Mode, Thermostat                         |       |
| 84  | Water Valve                       | 66 (0x0042)   | Identify, Valve Configuration and Control                                          | None (Ready)                                          |       |
| 85  | Window Covering                   | 514 (0x0202)  | Identify, Window Covering                                                          | Window Covering                                       |       |
| 86  | Window Covering Controller        | 515 (0x0203)  | None                                                                               | None (Ready)                                          |       |
