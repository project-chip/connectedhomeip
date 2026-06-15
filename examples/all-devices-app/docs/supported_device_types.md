# Matter Device Types Implementation Status

**Updated as of**: 2026-06-12 (Matter Specification SHA: `5a31ae2acb487bea09286243ccb5ad4ca9d3ef08`)

## Discovery & Updating Methodology
* **Specification Catalog**: Read device definitions from the AsciiDoc Markdown build of the Matter specification (`device_library/*.md`).
* **All-Devices Support**: Check for a corresponding device implementation subdirectory under `examples/all-devices-app/all-devices-common/devices/`.

## Implemented Device Types

| Device Type Name               | ID           | Notes |
| ------------------------------ | ------------ | ----- |
| Aggregator                     | 14 (0x000E)  |       |
| Air Quality Sensor             | 44 (0x002C)  |       |
| Bridged Node                   | 19 (0x0013)  |       |
| Chime                          | 326 (0x0146) |       |
| Dimmable Light                 | 257 (0x0101) |       |
| Fan                            | 43 (0x002B)  |       |
| Network Infrastructure Manager | 144 (0x0090) |       |
| Occupancy Sensor               | 263 (0x0107) |       |
| On/Off Light                   | 256 (0x0100) |       |
| Power Source                   | 17 (0x0011)  |       |
| Proximity Ranger               | -            |       |
| Root Node                      | 22 (0x0016)  |       |
| Secondary Network Interface    | 25 (0x0019)  |       |
| Smoke CO Alarm                 | 118 (0x0076) |       |
| Soil Sensor                    | 69 (0x0045)  |       |
| Speaker                        | 34 (0x0022)  |       |
| Temperature Sensor             | 770 (0x0302) |       |

## Unimplemented Device Types

| Device Type Name                  | ID            | Mandatory Server Clusters                                                          | Missing Mandatory Clusters                            | Notes |
| --------------------------------- | ------------- | ---------------------------------------------------------------------------------- | ----------------------------------------------------- | ----- |
| Air Purifier                      | 45 (0x002D)   | Identify, Fan Control                                                              | None (Ready)                                          |       |
| Arc Fault Circuit Interrupter     | -             | User Label, Power Topology                                                         | None (Ready)                                          |       |
| Audio Doorbell                    | 321 (0x0141)  | Identify, Switch, Camera AV Stream Management, WebRTC Transport Provider           | None (Ready)                                          |       |
| Auxiliary Load Switch             | -             | Boolean State                                                                      | None (Ready)                                          |       |
| Base                              | -             | None                                                                               | None (Ready)                                          |       |
| Basic Video Player                | 40 (0x0028)   | On/Off, Media Playback, Keypad Input                                               | Media Playback, Keypad Input                          |       |
| Battery Storage                   | 24 (0x0018)   | None                                                                               | None (Ready)                                          |       |
| Camera                            | 322 (0x0142)  | Camera AV Stream Management, WebRTC Transport Provider                             | None (Ready)                                          |       |
| Camera Controller                 | 327 (0x0147)  | WebRTC Transport Requestor                                                         | None (Ready)                                          |       |
| Casting Video Client              | 41 (0x0029)   | None                                                                               | None (Ready)                                          |       |
| Casting Video Player              | 35 (0x0023)   | On/Off, Media Playback, Keypad Input, Content Launcher                             | Media Playback, Keypad Input, Content Launcher        |       |
| Closure                           | 560 (0x0230)  | Identify, Closure Control                                                          | None (Ready)                                          |       |
| Closure Controller                | 574 (0x023E)  | None                                                                               | None (Ready)                                          |       |
| Closure Panel                     | 561 (0x0231)  | Closure Dimension                                                                  | None (Ready)                                          |       |
| Color Dimmer Switch               | 261 (0x0105)  | Identify                                                                           | None (Ready)                                          |       |
| Color Temperature Light           | 268 (0x010C)  | Identify, Groups, On/Off, Level Control, Scenes Management, Color Control          | Color Control                                         |       |
| Commissioning By Proxy            | -             | None                                                                               | None (Ready)                                          |       |
| Contact Sensor                    | 21 (0x0015)   | Identify, Boolean State                                                            | None (Ready)                                          |       |
| Content App                       | 36 (0x0024)   | Keypad Input, Application Launcher, Application Basic                              | Keypad Input, Application Launcher, Application Basic |       |
| Control Bridge                    | 2112 (0x0840) | Identify                                                                           | None (Ready)                                          |       |
| Cook Surface                      | 119 (0x0077)  | None                                                                               | None (Ready)                                          |       |
| Cooktop                           | 120 (0x0078)  | On/Off                                                                             | None (Ready)                                          |       |
| Dimmable Plug-In Unit             | 267 (0x010B)  | Identify, Groups, On/Off, Level Control, Scenes Management                         | None (Ready)                                          |       |
| Dimmer Switch                     | 260 (0x0104)  | Identify                                                                           | None (Ready)                                          |       |
| Dishwasher                        | 117 (0x0075)  | Operational State                                                                  | Operational State                                     |       |
| Door Lock                         | 10 (0x000A)   | Identify, Door Lock                                                                | Door Lock                                             |       |
| Door Lock Controller              | 11 (0x000B)   | None                                                                               | None (Ready)                                          |       |
| Doorbell                          | 328 (0x0148)  | None                                                                               | None (Ready)                                          |       |
| EVSE                              | -             | Energy EVSE, Energy EVSE Mode                                                      | None (Ready)                                          |       |
| Electrical Circuit Breaker        | -             | User Label, Power Topology                                                         | None (Ready)                                          |       |
| Electrical Distribution Enclosure | -             | Power Topology                                                                     | None (Ready)                                          |       |
| Electrical Energy Tariff          | 1299 (0x0513) | None                                                                               | None (Ready)                                          |       |
| Electrical Meter                  | 1300 (0x0514) | Electrical Power Measurement, Electrical Energy Measurement                        | None (Ready)                                          |       |
| Electrical Sensor                 | 1296 (0x0510) | Device Energy Management                                                           | None (Ready)                                          |       |
| Electrical Surge Protector        | -             | User Label, Power Topology                                                         | None (Ready)                                          |       |
| Electrical Utility Meter          | 1297 (0x0511) | Meter Identification                                                               | Meter Identification                                  |       |
| Extended Color Light              | 269 (0x010D)  | Identify, Groups, On/Off, Level Control, Scenes Management, Color Control          | Color Control                                         |       |
| Extractor Hood                    | 122 (0x007A)  | Fan Control                                                                        | None (Ready)                                          |       |
| Floodlight Camera                 | 324 (0x0144)  | None                                                                               | None (Ready)                                          |       |
| Flow Sensor                       | 774 (0x0306)  | Identify, Flow Measurement                                                         | None (Ready)                                          |       |
| Generic Switch                    | 15 (0x000F)   | Identify, Switch                                                                   | None (Ready)                                          |       |
| Heat Pump                         | 777 (0x0309)  | None                                                                               | None (Ready)                                          |       |
| Humidifier/Dehumidifier           | -             | None                                                                               | None (Ready)                                          |       |
| Humidity Sensor                   | 775 (0x0307)  | Identify, Relative Humidity Measurement                                            | None (Ready)                                          |       |
| Intercom                          | 320 (0x0140)  | Camera AV Stream Management, WebRTC Transport Provider, WebRTC Transport Requestor | None (Ready)                                          |       |
| Irrigation System                 | 64 (0x0040)   | None                                                                               | None (Ready)                                          |       |
| Joint Fabric Administrator        | 304 (0x0130)  | Joint Fabric Datastore, Joint Fabric Administrator                                 | Joint Fabric Datastore, Joint Fabric Administrator    |       |
| Laundry Dryer                     | 124 (0x007C)  | Operational State                                                                  | Operational State                                     |       |
| Laundry Washer                    | 115 (0x0073)  | Operational State                                                                  | Operational State                                     |       |
| Light Sensor                      | 262 (0x0106)  | Identify, Illuminance Measurement                                                  | None (Ready)                                          |       |
| Meter Reference Point             | 1298 (0x0512) | Identify                                                                           | None (Ready)                                          |       |
| Microwave Oven                    | 121 (0x0079)  | Operational State, Microwave Oven Mode, Microwave Oven Control                     | Operational State                                     |       |
| Mode Select                       | 39 (0x0027)   | Mode Select                                                                        | Mode Select                                           |       |
| Mounted Dimmable Load Control     | 272 (0x0110)  | Identify, Groups, On/Off, Level Control, Scenes Management                         | None (Ready)                                          |       |
| Mounted On/Off Control            | 271 (0x010F)  | Identify, Groups, On/Off, Scenes Management                                        | None (Ready)                                          |       |
| OTA Provider                      | 20 (0x0014)   | OTA Software Update Provider                                                       | OTA Software Update Provider                          |       |
| OTA Requestor                     | 18 (0x0012)   | OTA Software Update Requestor                                                      | OTA Software Update Requestor                         |       |
| On/Off Plug-in Unit               | 266 (0x010A)  | Identify, Groups, On/Off, Scenes Management                                        | None (Ready)                                          |       |
| On/Off Sensor                     | 2128 (0x0850) | Identify                                                                           | None (Ready)                                          |       |
| Oven                              | 123 (0x007B)  | None                                                                               | None (Ready)                                          |       |
| Pressure Sensor                   | 773 (0x0305)  | Identify, Pressure Measurement                                                     | None (Ready)                                          |       |
| Pump                              | 771 (0x0303)  | Identify, On/Off, Pump Configuration and Control                                   | Pump Configuration and Control                        |       |
| Pump Controller                   | 772 (0x0304)  | Identify                                                                           | None (Ready)                                          |       |
| Rain Sensor                       | 68 (0x0044)   | Identify, Boolean State                                                            | None (Ready)                                          |       |
| Refrigerator                      | 112 (0x0070)  | None                                                                               | None (Ready)                                          |       |
| Residual Current Circuit Breaker  | -             | User Label, Power Topology                                                         | None (Ready)                                          |       |
| Robotic Vacuum Cleaner            | 116 (0x0074)  | Identify, RVC Run Mode, RVC Operational State                                      | RVC Run Mode, RVC Operational State                   |       |
| Room Air Conditioner              | 114 (0x0072)  | Identify, On/Off, Thermostat                                                       | Thermostat                                            |       |
| Snapshot Camera                   | 325 (0x0145)  | Camera AV Stream Management                                                        | None (Ready)                                          |       |
| Solar Power                       | 23 (0x0017)   | None                                                                               | None (Ready)                                          |       |
| Temperature Controlled Cabinet    | 113 (0x0071)  | Temperature Control                                                                | None (Ready)                                          |       |
| Thermostat                        | 769 (0x0301)  | Identify, Thermostat                                                               | Thermostat                                            |       |
| Thermostat Controller             | 778 (0x030A)  | None                                                                               | None (Ready)                                          |       |
| Thread Border Router              | 145 (0x0091)  | Thread Network Diagnostics, Thread Border Router Management                        | None (Ready)                                          |       |
| Video Doorbell                    | 323 (0x0143)  | None                                                                               | None (Ready)                                          |       |
| Video Remote Control              | 42 (0x002A)   | None                                                                               | None (Ready)                                          |       |
| Water Freeze Detector             | 65 (0x0041)   | Identify, Boolean State                                                            | None (Ready)                                          |       |
| Water Heater                      | 1295 (0x050F) | Water Heater Management, Water Heater Mode, Thermostat                             | Water Heater Mode, Thermostat                         |       |
| Water Leak Detector               | 67 (0x0043)   | Identify, Boolean State                                                            | None (Ready)                                          |       |
| Water Valve                       | 66 (0x0042)   | Identify, Valve Configuration and Control                                          | None (Ready)                                          |       |
| Window Covering                   | 514 (0x0202)  | Identify, Window Covering                                                          | Window Covering                                       |       |
| Window Covering Controller        | 515 (0x0203)  | None                                                                               | None (Ready)                                          |       |
