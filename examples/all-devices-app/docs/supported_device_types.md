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

| Device Type Name               | ID           | Notes |
| ------------------------------ | ------------ | ----- |
| Aggregator                     | 14 (0x000E)  |       |
| Air Quality Sensor             | 44 (0x002C)  |       |
| Bridged Node                   | 19 (0x0013)  |       |
| Chime                          | 326 (0x0146) |       |
| Contact Sensor                 | 21 (0x0015)  |       |
| Dimmable Light                 | 257 (0x0101) |       |
| Fan                            | 43 (0x002B)  |       |
| Network Infrastructure Manager | 144 (0x0090) |       |
| Occupancy Sensor               | 263 (0x0107) |       |
| On/Off Light                   | 256 (0x0100) |       |
| Power Source                   | 17 (0x0011)  |       |
| Proximity Ranger               | 338 (0x0152) |       |
| Root Node                      | 22 (0x0016)  |       |
| Smoke CO Alarm                 | 118 (0x0076) |       |
| Soil Sensor                    | 69 (0x0045)  |       |
| Speaker                        | 34 (0x0022)  |       |
| Temperature Sensor             | 770 (0x0302) |       |
| Water Leak Detector            | 67 (0x0043)  |       |

## Unimplemented Device Types (86 total)

| Device Type Name                  | ID            | Mandatory Server Clusters                                                          | Missing Mandatory Clusters                            | Notes |
| --------------------------------- | ------------- | ---------------------------------------------------------------------------------- | ----------------------------------------------------- | ----- |
| AV Analysis Node                  | 329 (0x0149)  | None                                                                               | None (Ready)                                          |       |
| Air Purifier                      | 45 (0x002D)   | Identify, Fan Control                                                              | None (Ready)                                          |       |
| Ambient Context Sensor            | 336 (0x0150)  | None                                                                               | None (Ready)                                          |       |
| Arc Fault Circuit Interrupter     | 1301 (0x0515) | User Label, Power Topology                                                         | None (Ready)                                          |       |
| Audio Doorbell                    | 321 (0x0141)  | Identify, Switch, Camera AV Stream Management, WebRTC Transport Provider           | None (Ready)                                          |       |
| Basic Video Player                | 40 (0x0028)   | On/Off, Media Playback, Keypad Input                                               | Media Playback, Keypad Input                          |       |
| Battery Storage                   | 24 (0x0018)   | None                                                                               | None (Ready)                                          |       |
| Camera                            | 322 (0x0142)  | Camera AV Stream Management, WebRTC Transport Provider                             | None (Ready)                                          |       |
| Camera Controller                 | 327 (0x0147)  | WebRTC Transport Requestor                                                         | None (Ready)                                          |       |
| Casting Audio Player              | 33 (0x0021)   | Media Playback, Content Launcher, Chime                                            | Media Playback, Content Launcher                      |       |
| Casting Video Client              | 41 (0x0029)   | None                                                                               | None (Ready)                                          |       |
| Casting Video Player              | 35 (0x0023)   | On/Off, Media Playback, Keypad Input, Content Launcher                             | Media Playback, Keypad Input, Content Launcher        |       |
| Closure                           | 560 (0x0230)  | Identify, Closure Control                                                          | None (Ready)                                          |       |
| Closure Controller                | 574 (0x023E)  | None                                                                               | None (Ready)                                          |       |
| Closure Panel                     | 561 (0x0231)  | Closure Dimension                                                                  | None (Ready)                                          |       |
| Color Dimmer Switch               | 261 (0x0105)  | Identify                                                                           | None (Ready)                                          |       |
| Color Temperature Light           | 268 (0x010C)  | Identify, Groups, On/Off, Level Control, Scenes Management, Color Control          | Color Control                                         |       |
| Commissioning By Proxy            | 146 (0x0092)  | None                                                                               | None (Ready)                                          |       |
| Content App                       | 36 (0x0024)   | Keypad Input, Application Launcher, Application Basic                              | Keypad Input, Application Launcher, Application Basic |       |
| Control Bridge                    | 2112 (0x0840) | Identify                                                                           | None (Ready)                                          |       |
| Cook Surface                      | 119 (0x0077)  | None                                                                               | None (Ready)                                          |       |
| Cooktop                           | 120 (0x0078)  | On/Off                                                                             | None (Ready)                                          |       |
| Device Energy Management          | 1293 (0x050D) | Device Energy Management                                                           | None (Ready)                                          |       |
| Dimmable Plug-In Unit             | 267 (0x010B)  | Identify, Groups, On/Off, Level Control, Scenes Management                         | None (Ready)                                          |       |
| Dimmer Switch                     | 260 (0x0104)  | Identify                                                                           | None (Ready)                                          |       |
| Dishwasher                        | 117 (0x0075)  | Operational State                                                                  | Operational State                                     |       |
| Door Lock                         | 10 (0x000A)   | Identify, Door Lock                                                                | Door Lock                                             |       |
| Door Lock Controller              | 11 (0x000B)   | None                                                                               | None (Ready)                                          |       |
| Doorbell                          | 328 (0x0148)  | None                                                                               | None (Ready)                                          |       |
| Electrical Circuit Breaker        | 1302 (0x0516) | User Label, Power Topology                                                         | None (Ready)                                          |       |
| Electrical Distribution Enclosure | 1303 (0x0517) | Power Topology                                                                     | None (Ready)                                          |       |
| Electrical Energy Tariff          | 1299 (0x0513) | None                                                                               | None (Ready)                                          |       |
| Electrical Meter                  | 1300 (0x0514) | Electrical Power Measurement, Electrical Energy Measurement                        | None (Ready)                                          |       |
| Electrical Sensor                 | 1296 (0x0510) | Power Topology                                                                     | None (Ready)                                          |       |
| Electrical Surge Protector        | 1304 (0x0518) | User Label, Power Topology                                                         | None (Ready)                                          |       |
| Electrical Utility Meter          | 1297 (0x0511) | Meter Identification                                                               | Meter Identification                                  |       |
| Energy EVSE                       | 1292 (0x050C) | Energy EVSE, Energy EVSE Mode                                                      | Energy EVSE Mode                                      |       |
| Extended Color Light              | 269 (0x010D)  | Identify, Groups, On/Off, Level Control, Scenes Management, Color Control          | Color Control                                         |       |
| Extractor Hood                    | 122 (0x007A)  | Fan Control                                                                        | None (Ready)                                          |       |
| Floodlight Camera                 | 324 (0x0144)  | None                                                                               | None (Ready)                                          |       |
| Flow Sensor                       | 774 (0x0306)  | Identify, Flow Measurement                                                         | None (Ready)                                          |       |
| Generic Switch                    | 15 (0x000F)   | Identify, Switch                                                                   | None (Ready)                                          |       |
| Heat Pump                         | 777 (0x0309)  | None                                                                               | None (Ready)                                          |       |
| Heating/Cooling Unit              | 768 (0x0300)  | None                                                                               | None (Ready)                                          |       |
| Humidifier/Dehumidifier           | 125 (0x007D)  | None                                                                               | None (Ready)                                          |       |
| Humidity Sensor                   | 775 (0x0307)  | Identify, Relative Humidity Measurement                                            | None (Ready)                                          |       |
| Intercom                          | 320 (0x0140)  | Camera AV Stream Management, WebRTC Transport Provider, WebRTC Transport Requestor | None (Ready)                                          |       |
| Irrigation System                 | 64 (0x0040)   | None                                                                               | None (Ready)                                          |       |
| Joint Fabric Administrator        | 304 (0x0130)  | Joint Fabric Datastore, Joint Fabric Administrator                                 | Joint Fabric Datastore, Joint Fabric Administrator    |       |
| Laundry Dryer                     | 124 (0x007C)  | Operational State                                                                  | Operational State                                     |       |
| Laundry Washer                    | 115 (0x0073)  | Operational State                                                                  | Operational State                                     |       |
| Light Sensor                      | 262 (0x0106)  | Identify, Illuminance Measurement                                                  | None (Ready)                                          |       |
| Meter Reference Point             | 1298 (0x0512) | Identify                                                                           | None (Ready)                                          |       |
| Microwave Oven                    | 121 (0x0079)  | Operational State, Microwave Oven Mode, Microwave Oven Control                     | Operational State, Microwave Oven Mode                |       |
| Mode Select                       | 39 (0x0027)   | Mode Select                                                                        | Mode Select                                           |       |
| Mounted Dimmable Load Control     | 272 (0x0110)  | Identify, Groups, On/Off, Level Control, Scenes Management                         | None (Ready)                                          |       |
| Mounted On/Off Control            | 271 (0x010F)  | Identify, Groups, On/Off, Scenes Management                                        | None (Ready)                                          |       |
| OTA Provider                      | 20 (0x0014)   | OTA Software Update Provider                                                       | None (Ready)                                          |       |
| OTA Requestor                     | 18 (0x0012)   | OTA Software Update Requestor                                                      | None (Ready)                                          |       |
| On/Off Light Switch               | 259 (0x0103)  | Identify                                                                           | None (Ready)                                          |       |
| On/Off Plug-in Unit               | 266 (0x010A)  | Identify, Groups, On/Off, Scenes Management                                        | None (Ready)                                          |       |
| On/Off Sensor                     | 2128 (0x0850) | Identify                                                                           | None (Ready)                                          |       |
| Oven                              | 123 (0x007B)  | None                                                                               | None (Ready)                                          |       |
| Pressure Sensor                   | 773 (0x0305)  | Identify, Pressure Measurement                                                     | None (Ready)                                          |       |
| Pump                              | 771 (0x0303)  | Identify, On/Off, Pump Configuration and Control                                   | Pump Configuration and Control                        |       |
| Pump Controller                   | 772 (0x0304)  | Identify                                                                           | None (Ready)                                          |       |
| Rain Sensor                       | 68 (0x0044)   | Identify, Boolean State                                                            | None (Ready)                                          |       |
| Refrigerator                      | 112 (0x0070)  | None                                                                               | None (Ready)                                          |       |
| Residual Current Circuit Breaker  | 1305 (0x0519) | User Label, Power Topology                                                         | None (Ready)                                          |       |
| Robotic Vacuum Cleaner            | 116 (0x0074)  | Identify, RVC Run Mode, RVC Operational State                                      | RVC Run Mode, RVC Operational State                   |       |
| Room Air Conditioner              | 114 (0x0072)  | Identify, On/Off, Thermostat                                                       | Thermostat                                            |       |
| Secondary Network Interface       | 25 (0x0019)   | Network Commissioning                                                              | None (Ready)                                          |       |
| Snapshot Camera                   | 325 (0x0145)  | Camera AV Stream Management                                                        | None (Ready)                                          |       |
| Solar Power                       | 23 (0x0017)   | None                                                                               | None (Ready)                                          |       |
| Streaming Audio Player            | 32 (0x0020)   | Media Playback, Content Launcher, Chime                                            | Media Playback, Content Launcher                      |       |
| Temperature Controlled Cabinet    | 113 (0x0071)  | Temperature Control                                                                | None (Ready)                                          |       |
| Thermostat                        | 769 (0x0301)  | Identify, Thermostat                                                               | Thermostat                                            |       |
| Thermostat Controller             | 778 (0x030A)  | None                                                                               | None (Ready)                                          |       |
| Thread Border Router              | 145 (0x0091)  | Thread Network Diagnostics, Thread Border Router Management                        | None (Ready)                                          |       |
| Video Doorbell                    | 323 (0x0143)  | None                                                                               | None (Ready)                                          |       |
| Video Remote Control              | 42 (0x002A)   | None                                                                               | None (Ready)                                          |       |
| Water Freeze Detector             | 65 (0x0041)   | Identify, Boolean State                                                            | None (Ready)                                          |       |
| Water Heater                      | 1295 (0x050F) | Water Heater Management, Water Heater Mode, Thermostat                             | Water Heater Mode, Thermostat                         |       |
| Water Valve                       | 66 (0x0042)   | Identify, Valve Configuration and Control                                          | None (Ready)                                          |       |
| Window Covering                   | 514 (0x0202)  | Identify, Window Covering                                                          | Window Covering                                       |       |
| Window Covering Controller        | 515 (0x0203)  | None                                                                               | None (Ready)                                          |       |
