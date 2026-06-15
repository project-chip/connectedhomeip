# Matter Clusters Implementation Status

**Updated as of**: 2026-06-12 (Matter Specification SHA: `5a31ae2acb487bea09286243ccb5ad4ca9d3ef08`)

## Discovery & Updating Methodology
* **Specification Catalog**: Read cluster definitions from the AsciiDoc Markdown build of the Matter specification (`appclusters/*.md`).
* **SDK Code-Driven Status**: Search `src/app/clusters/` for implementations using `ServerClusterInterface` or `DefaultServerCluster`.
* **All-Devices Usage**: Check for active cluster includes or registrations under `examples/all-devices-app/all-devices-common/devices/`.

| Cluster Name                                         | ID            | Code-Driven in SDK | Used in All-Devices | Notes |
| ---------------------------------------------------- | ------------- | ------------------ | ------------------- | ----- |
| AV Analysis                                          | -             | No                 | No                  |       |
| Account Login                                        | 1294 (0x050E) | No                 | No                  |       |
| Air Quality                                          | 91 (0x005B)   | Yes                | Yes                 |       |
| Alarm Base                                           | -             | No                 | No                  |       |
| Ambient Context Sensing                              | 1073 (0x0431) | No                 | No                  |       |
| Ambient Sensing Union                                | -             | No                 | No                  |       |
| Application Basic                                    | 1293 (0x050D) | No                 | No                  |       |
| Application Launcher                                 | 1292 (0x050C) | No                 | No                  |       |
| Audio Control                                        | -             | No                 | No                  |       |
| Audio Output                                         | 1291 (0x050B) | No                 | No                  |       |
| Boolean State                                        | 69 (0x0045)   | Yes                | Yes                 |       |
| Boolean State Configuration                          | 128 (0x0080)  | Yes                | Yes                 |       |
| Camera AV Stream Management                          | 1361 (0x0551) | Yes                | No                  |       |
| Channel                                              | 1284 (0x0504) | No                 | No                  |       |
| Chime                                                | 1366 (0x0556) | Yes                | Yes                 |       |
| Closure Control                                      | 260 (0x0104)  | Yes                | No                  |       |
| Closure Dimension                                    | 261 (0x0105)  | Yes                | No                  |       |
| Color Control                                        | 768 (0x0300)  | No                 | No                  |       |
| Commissioning Proxy                                  | -             | No                 | No                  |       |
| Commodity Metering                                   | 2823 (0x0B07) | No                 | No                  |       |
| Commodity Price                                      | 149 (0x0095)  | No                 | No                  |       |
| Commodity Tariff                                     | 1792 (0x0700) | No                 | No                  |       |
| Content App Observer                                 | 1296 (0x0510) | No                 | No                  |       |
| Content Control                                      | 1295 (0x050F) | No                 | No                  |       |
| Content Launcher                                     | 1290 (0x050A) | No                 | No                  |       |
| Demand Response Load Control                         | -             | No                 | No                  |       |
| Device Energy Management                             | 152 (0x0098)  | Yes                | No                  |       |
| Device Energy Management Mode                        | 159 (0x009F)  | Yes                | No                  |       |
| Dishwasher Alarm                                     | 93 (0x005D)   | No                 | No                  |       |
| Dishwasher Mode                                      | 89 (0x0059)   | No                 | No                  |       |
| Door Lock                                            | 257 (0x0101)  | No                 | No                  |       |
| Dynamic Lighting                                     | -             | No                 | No                  |       |
| Electrical Alarm                                     | -             | No                 | No                  |       |
| Electrical Distribution                              | -             | No                 | No                  |       |
| Electrical Energy Measurement                        | 145 (0x0091)  | Yes                | No                  |       |
| Electrical Grid Conditions                           | 160 (0x00A0)  | No                 | No                  |       |
| Electrical Power Measurement                         | 144 (0x0090)  | Yes                | No                  |       |
| Electrical Protection Alarm                          | -             | No                 | No                  |       |
| Energy EVSE                                          | 153 (0x0099)  | Yes                | No                  |       |
| Energy EVSE Mode                                     | 157 (0x009D)  | Yes                | No                  |       |
| Energy Preference                                    | 155 (0x009B)  | No                 | No                  |       |
| Fan Control                                          | 514 (0x0202)  | Yes                | Yes                 |       |
| Flow Measurement                                     | 1028 (0x0404) | Yes                | No                  |       |
| Groups                                               | 4 (0x0004)    | Yes                | Yes                 |       |
| Humidistat                                           | -             | Yes                | No                  |       |
| Identify                                             | 3 (0x0003)    | Yes                | Yes                 |       |
| Illuminance Measurement                              | 1024 (0x0400) | Yes                | No                  |       |
| Keypad Input                                         | 1289 (0x0509) | No                 | No                  |       |
| Laundry Dryer Controls                               | 74 (0x004A)   | No                 | No                  |       |
| Laundry Washer Controls                              | 83 (0x0053)   | No                 | No                  |       |
| Laundry Washer Mode                                  | 81 (0x0051)   | No                 | No                  |       |
| Level Control                                        | 8 (0x0008)    | Yes                | Yes                 |       |
| Low Power                                            | 1288 (0x0508) | No                 | No                  |       |
| Media File Management                                | -             | No                 | No                  |       |
| Media Input                                          | 1287 (0x0507) | No                 | No                  |       |
| Media Playback                                       | 1286 (0x0506) | No                 | No                  |       |
| Messages                                             | 151 (0x0097)  | No                 | No                  |       |
| Meter Identification                                 | 2822 (0x0B06) | No                 | No                  |       |
| Microwave Oven Control                               | 95 (0x005F)   | Yes                | No                  |       |
| Microwave Oven Mode                                  | 94 (0x005E)   | Yes                | No                  |       |
| Mode Base                                            | -             | No                 | No                  |       |
| Mode Select                                          | 80 (0x0050)   | No                 | No                  |       |
| Network Identity Management                          | -             | Yes                | No                  |       |
| Occupancy Sensing                                    | 1030 (0x0406) | Yes                | Yes                 |       |
| On/Off                                               | 6 (0x0006)    | Yes                | Yes                 |       |
| Operational State                                    | 96 (0x0060)   | No                 | No                  |       |
| Oven Cavity Operational State                        | 72 (0x0048)   | No                 | No                  |       |
| Oven Mode                                            | 73 (0x0049)   | No                 | No                  |       |
| Pressure Measurement                                 | 1027 (0x0403) | Yes                | No                  |       |
| Proximity Ranging                                    | -             | Yes                | Yes                 |       |
| Pump Configuration and Control                       | 512 (0x0200)  | No                 | No                  |       |
| Push AV Stream Transport                             | 1365 (0x0555) | Yes                | No                  |       |
| RVC Clean Mode                                       | 85 (0x0055)   | No                 | No                  |       |
| RVC Operational State                                | 97 (0x0061)   | No                 | No                  |       |
| RVC Run Mode                                         | 84 (0x0054)   | No                 | No                  |       |
| Refrigerator Alarm                                   | 87 (0x0057)   | No                 | No                  |       |
| Refrigerator And Temperature Controlled Cabinet Mode | 82 (0x0052)   | Yes                | No                  |       |
| Scenes Management                                    | 98 (0x0062)   | Yes                | Yes                 |       |
| Service Area                                         | 336 (0x0150)  | No                 | No                  |       |
| Smoke CO Alarm                                       | 92 (0x005C)   | Yes                | Yes                 |       |
| Soil Measurement                                     | 1072 (0x0430) | Yes                | Yes                 |       |
| Switch                                               | 59 (0x003B)   | Yes                | No                  |       |
| Target Navigator                                     | 1285 (0x0505) | No                 | No                  |       |
| Temperature Alarm                                    | 100 (0x0064)  | Yes                | No                  |       |
| Temperature Control                                  | 86 (0x0056)   | Yes                | Yes                 |       |
| Temperature Controlled Cabinet Topology              | -             | Yes                | No                  |       |
| Temperature Measurement                              | 1026 (0x0402) | Yes                | Yes                 |       |
| Thermostat                                           | 513 (0x0201)  | No                 | No                  |       |
| Thermostat Mode                                      | -             | No                 | No                  |       |
| Thermostat User Interface Configuration              | 516 (0x0204)  | No                 | No                  |       |
| Thread Border Router Diagnostics                     | -             | No                 | No                  |       |
| Thread Border Router Management                      | 1106 (0x0452) | Yes                | Yes                 |       |
| Thread Network Directory                             | 1107 (0x0453) | Yes                | Yes                 |       |
| Valve Configuration and Control                      | 129 (0x0081)  | Yes                | No                  |       |
| Wake On LAN                                          | 1283 (0x0503) | No                 | No                  |       |
| Water Heater Management                              | 148 (0x0094)  | Yes                | No                  |       |
| Water Heater Mode                                    | 158 (0x009E)  | No                 | No                  |       |
| WebRTC Transport Provider                            | 1363 (0x0553) | Yes                | No                  |       |
| WebRTC Transport Requestor                           | 1364 (0x0554) | Yes                | No                  |       |
| Wi-Fi Network Management                             | 1105 (0x0451) | Yes                | Yes                 |       |
| Window Covering                                      | 258 (0x0102)  | No                 | No                  |       |
| Zone Management                                      | 1360 (0x0550) | Yes                | No                  |       |
