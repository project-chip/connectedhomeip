# Matter Clusters Implementation Status (159 total)

**Updated as of**: 2026-06-25 (Matter Specification SHA:
`5a31ae2acb487bea09286243ccb5ad4ca9d3ef08`)

## Discovery & Updating Methodology

To update or validate this list manually, follow these steps:

1. **Verify Specification Catalog**:

    - Refer to the master spec cluster sheet:
      `docs/ids_and_codes/spec_clusters.md` (updated automatically).
    - This provides the cluster names, decimal IDs, and hex IDs.

2. **Verify Code-Driven in SDK Status**:

    - Search/grep to locate all cluster classes implementing the code-driven
      model. For example, if using ripgrep (`rg`):
      `rg -n "public\s+(chip::app::)?DefaultServerCluster" src/app/` and
      `rg -n "public\s+(chip::app::)?ServerClusterInterface" src/app/`
    - Every C++ class found represents a code-driven cluster. Map its cluster ID
      to this table.
    - Note: Generic/templated implementations (e.g.
      `ConcentrationMeasurementCluster` in
      `src/app/clusters/concentration-measurement-server/` or
      `ResourceMonitoringCluster` in
      `src/app/clusters/resource-monitoring-server/`) implement multiple
      distinct spec-defined clusters (e.g. Carbon Dioxide, HEPA Filter
      Monitoring). All mapped clusters should be marked as "Yes".

3. **Verify Used in All-Devices**:
    - Search/grep to find which clusters are dynamically registered in the
      all-devices-app. For example, if using ripgrep (`rg`):
      `rg "LazyRegisteredServerCluster<|RegisteredServerCluster<" examples/all-devices-app/`
    - Check where these are registered. They are typically added to the endpoint
      in the device implementation files under
      `examples/all-devices-app/all-devices-common/device/types/` (e.g.
      `provider.AddCluster(...)`).
    - Mark as "Yes" only if they are actively instantiated and added to an
      endpoint.

## Cluster Implementation Details

| #         | Cluster Name                                               | ID            | Code-Driven in SDK | Used in All-Devices | Notes                                 |
| --------- | ---------------------------------------------------------- | ------------- | ------------------ | ------------------- | ------------------------------------- |
| 1         | AV Analysis                                                | 1367 (0x0557) | No                 | No                  |                                       |
| 2         | Access Control                                             | 31 (0x001F)   | Yes                | Yes                 |                                       |
| 3         | Account Login                                              | 1294 (0x050E) | No                 | No                  |                                       |
| 4         | Actions                                                    | 37 (0x0025)   | Yes                | No                  |                                       |
| 5         | Activated Carbon Filter Monitoring                         | 114 (0x0072)  | Yes                | No                  | Alias of Resource Monitoring          |
| 6         | Administrator Commissioning                                | 60 (0x003C)   | Yes                | Yes                 |                                       |
| 7         | Air Quality                                                | 91 (0x005B)   | Yes                | Yes                 |                                       |
| 8         | Ambient Context Sensing                                    | 1073 (0x0431) | Yes                | Yes                 |                                       |
| 9         | Ambient Sensing Union                                      | 1074 (0x0432) | No                 | No                  |                                       |
| 10        | Application Basic                                          | 1293 (0x050D) | No                 | No                  |                                       |
| 11        | Application Launcher                                       | 1292 (0x050C) | No                 | No                  |                                       |
| 12        | Audio Control                                              | 1298 (0x0512) | No                 | No                  |                                       |
| 13        | Audio Output                                               | 1291 (0x050B) | No                 | No                  |                                       |
| 14        | Ballast Configuration                                      | 769 (0x0301)  | No                 | No                  |                                       |
| 15        | Basic Information                                          | 40 (0x0028)   | Yes                | Yes                 |                                       |
| 16        | Binding                                                    | 30 (0x001E)   | Yes                | Yes                 |                                       |
| 17        | Boolean State                                              | 69 (0x0045)   | Yes                | Yes                 |                                       |
| 18        | Boolean State Configuration                                | 128 (0x0080)  | Yes                | No                  |                                       |
| 19        | Bridged Device Basic Information                           | 57 (0x0039)   | Yes                | Yes                 |                                       |
| 20        | Camera AV Settings User Level Management                   | 1362 (0x0552) | Yes                | No                  |                                       |
| 21        | Camera AV Stream Management                                | 1361 (0x0551) | Yes                | No                  |                                       |
| 22        | Carbon Dioxide Concentration Measurement                   | 1037 (0x040D) | Yes                | Yes                 | Instance of Concentration Measurement |
| 23        | Carbon Monoxide Concentration Measurement                  | 1036 (0x040C) | Yes                | Yes                 | Instance of Concentration Measurement |
| 24        | Channel                                                    | 1284 (0x0504) | No                 | No                  |                                       |
| 25        | Chime                                                      | 1366 (0x0556) | Yes                | Yes                 |                                       |
| 26        | Closure Control                                            | 260 (0x0104)  | Yes                | No                  |                                       |
| 27        | Closure Dimension                                          | 261 (0x0105)  | Yes                | No                  |                                       |
| 28        | Color Control                                              | 768 (0x0300)  | No                 | No                  |                                       |
| 29        | Commissioner Control                                       | 1873 (0x0751) | Yes                | No                  |                                       |
| 30        | Commissioning Proxy                                        | 1109 (0x0455) | Yes                | Yes                 |                                       |
| 31        | Commodity Metering                                         | 2823 (0x0B07) | No                 | No                  |                                       |
| 32        | Commodity Price                                            | 149 (0x0095)  | No                 | No                  |                                       |
| 33        | Commodity Tariff                                           | 1792 (0x0700) | No                 | No                  |                                       |
| 34        | Content App Observer                                       | 1296 (0x0510) | No                 | No                  |                                       |
| 35        | Content Control                                            | 1295 (0x050F) | No                 | No                  |                                       |
| 36        | Content Launcher                                           | 1290 (0x050A) | No                 | No                  |                                       |
| 37        | Descriptor                                                 | 29 (0x001D)   | Yes                | Yes                 |                                       |
| 38        | Device Energy Management                                   | 152 (0x0098)  | Yes                | Yes                 |                                       |
| 39        | Device Energy Management Mode                              | 159 (0x009F)  | Yes                | No                  | Instance of Mode Base                 |
| 40        | Diagnostic Logs                                            | 50 (0x0032)   | Yes                | No                  |                                       |
| 41        | Dishwasher Alarm                                           | 93 (0x005D)   | No                 | No                  |                                       |
| 42        | Dishwasher Mode                                            | 89 (0x0059)   | Yes                | Yes                 | Instance of Mode Base                 |
| 43        | Door Lock                                                  | 257 (0x0101)  | No                 | No                  |                                       |
| 44        | Dynamic Lighting                                           | 773 (0x0305)  | No                 | No                  |                                       |
| 45        | Ecosystem Information                                      | 1872 (0x0750) | No                 | No                  |                                       |
| 46        | Electrical Alarm                                           | 161 (0x00A1)  | No                 | No                  |                                       |
| 47        | Electrical Distribution                                    | 162 (0x00A2)  | No                 | No                  |                                       |
| 48        | Electrical Energy Measurement                              | 145 (0x0091)  | Yes                | Yes                 |                                       |
| 49        | Electrical Grid Conditions                                 | 160 (0x00A0)  | No                 | No                  |                                       |
| 50        | Electrical Power Measurement                               | 144 (0x0090)  | Yes                | Yes                 |                                       |
| 51        | Electrical Protection Alarm                                | 163 (0x00A3)  | No                 | No                  |                                       |
| 52        | Energy EVSE                                                | 153 (0x0099)  | Yes                | No                  |                                       |
| 53        | Energy EVSE Mode                                           | 157 (0x009D)  | Yes                | No                  | Instance of Mode Base                 |
| 54        | Energy Preference                                          | 155 (0x009B)  | No                 | No                  |                                       |
| 55        | Ethernet Network Diagnostics                               | 55 (0x0037)   | Yes                | No                  |                                       |
| 56        | Fan Control                                                | 514 (0x0202)  | Yes                | Yes                 |                                       |
| 57        | Fixed Label                                                | 64 (0x0040)   | Yes                | No                  |                                       |
| 58        | Flow Measurement                                           | 1028 (0x0404) | Yes                | Yes                 |                                       |
| 59        | Formaldehyde Concentration Measurement                     | 1067 (0x042B) | Yes                | Yes                 | Instance of Concentration Measurement |
| 60        | General Commissioning                                      | 48 (0x0030)   | Yes                | Yes                 |                                       |
| 61        | General Diagnostics                                        | 51 (0x0033)   | Yes                | Yes                 |                                       |
| 62        | Group Key Management                                       | 63 (0x003F)   | Yes                | Yes                 |                                       |
| 63        | Groupcast                                                  | 101 (0x0065)  | Yes                | Yes                 |                                       |
| 64        | Groups                                                     | 4 (0x0004)    | Yes                | Yes                 |                                       |
| 65        | HEPA Filter Monitoring                                     | 113 (0x0071)  | Yes                | No                  | Alias of Resource Monitoring          |
| 66        | Humidistat                                                 | 517 (0x0205)  | Yes                | Yes                 |                                       |
| 67        | ICD Management                                             | 70 (0x0046)   | Yes                | Yes                 |                                       |
| 68        | Identify                                                   | 3 (0x0003)    | Yes                | Yes                 |                                       |
| 69        | Illuminance Measurement                                    | 1024 (0x0400) | Yes                | Yes                 |                                       |
| 70        | Joint Fabric Administrator                                 | 1875 (0x0753) | No                 | No                  |                                       |
| 71        | Joint Fabric Datastore                                     | 1874 (0x0752) | No                 | No                  |                                       |
| 72        | Keypad Input                                               | 1289 (0x0509) | No                 | No                  |                                       |
| 73        | Laundry Dryer Controls                                     | 74 (0x004A)   | No                 | No                  |                                       |
| 74        | Laundry Washer Controls                                    | 83 (0x0053)   | Yes                | Yes                 |                                       |
| 75        | Laundry Washer Mode                                        | 81 (0x0051)   | Yes                | Yes                 | Instance of Mode Base                 |
| 76        | Leaf Wetness Measurement                                   | 1031 (0x0407) | No                 | No                  |                                       |
| 77        | Level Control                                              | 8 (0x0008)    | Yes                | Yes                 |                                       |
| 78        | Localization Configuration                                 | 43 (0x002B)   | Yes                | No                  |                                       |
| 79        | Low Power                                                  | 1288 (0x0508) | No                 | No                  |                                       |
| 80        | Media File Management                                      | 1297 (0x0511) | No                 | No                  |                                       |
| 81        | Media Input                                                | 1287 (0x0507) | No                 | No                  |                                       |
| 82        | Media Playback                                             | 1286 (0x0506) | No                 | No                  |                                       |
| 83        | Messages                                                   | 151 (0x0097)  | No                 | No                  |                                       |
| 84        | Meter Identification                                       | 2822 (0x0B06) | No                 | No                  |                                       |
| 85        | Microwave Oven Control                                     | 95 (0x005F)   | Yes                | Yes                 |                                       |
| 86        | Microwave Oven Mode                                        | 94 (0x005E)   | Yes                | Yes                 | Instance of Mode Base                 |
| 87        | Mode Select                                                | 80 (0x0050)   | Yes                | Yes                 |                                       |
| 88        | Network Commissioning                                      | 49 (0x0031)   | Yes                | Yes                 |                                       |
| 89        | Network Identity Management                                | 1104 (0x0450) | Yes                | No                  |                                       |
| 90        | Nitrogen Dioxide Concentration Measurement                 | 1043 (0x0413) | Yes                | Yes                 | Instance of Concentration Measurement |
| 91        | OTA Software Update Provider                               | 41 (0x0029)   | Yes                | No                  |                                       |
| 92        | OTA Software Update Requestor                              | 42 (0x002A)   | Yes                | No                  |                                       |
| 93        | Occupancy Sensing                                          | 1030 (0x0406) | Yes                | Yes                 |                                       |
| 94        | On/Off                                                     | 6 (0x0006)    | Yes                | Yes                 |                                       |
| 95        | Operational Credentials                                    | 62 (0x003E)   | Yes                | Yes                 |                                       |
| 96        | Operational State                                          | 96 (0x0060)   | Yes                | Yes                 |                                       |
| 97        | Oven Cavity Operational State                              | 72 (0x0048)   | Yes                | Yes                 |                                       |
| 98        | Oven Mode                                                  | 73 (0x0049)   | Yes                | No                  | Instance of Mode Base                 |
| 99        | Ozone Concentration Measurement                            | 1045 (0x0415) | Yes                | Yes                 | Instance of Concentration Measurement |
| 100       | PM1 Concentration Measurement                              | 1068 (0x042C) | Yes                | Yes                 | Instance of Concentration Measurement |
| 101       | PM10 Concentration Measurement                             | 1069 (0x042D) | Yes                | Yes                 | Instance of Concentration Measurement |
| 102       | PM2.5 Concentration Measurement                            | 1066 (0x042A) | Yes                | Yes                 | Instance of Concentration Measurement |
| 103       | Power Source                                               | 47 (0x002F)   | Yes                | Yes                 |                                       |
| 104       | Power Source Configuration                                 | 46 (0x002E)   | No                 | No                  |                                       |
| 105       | Power Topology                                             | 156 (0x009C)  | Yes                | Yes                 |                                       |
| 106       | Pressure Measurement                                       | 1027 (0x0403) | Yes                | Yes                 |                                       |
| 107       | Proximity Ranging                                          | 1075 (0x0433) | Yes                | Yes                 |                                       |
| 108       | Proxy Configuration                                        | 66 (0x0042)   | No                 | No                  |                                       |
| 109       | Proxy Discovery                                            | 67 (0x0043)   | No                 | No                  |                                       |
| 110       | Pulse Width Modulation                                     | 28 (0x001C)   | No                 | No                  |                                       |
| 111       | Pump Configuration and Control                             | 512 (0x0200)  | No                 | No                  |                                       |
| 112       | Push AV Stream Transport                                   | 1365 (0x0555) | Yes                | No                  |                                       |
| 113       | RVC Clean Mode                                             | 85 (0x0055)   | Yes                | Yes                 | Used by Robotic Vacuum Cleaner        |
| 114       | RVC Operational State                                      | 97 (0x0061)   | Yes                | Yes                 |                                       |
| 115       | RVC Run Mode                                               | 84 (0x0054)   | Yes                | Yes                 | Used by Robotic Vacuum Cleaner        |
| 116       | Radon Concentration Measurement                            | 1071 (0x042F) | Yes                | Yes                 | Instance of Concentration Measurement |
| 117       | Refrigerator Alarm                                         | 87 (0x0057)   | No                 | No                  |                                       |
| 118       | Refrigerator And Temperature Controlled Cabinet Mode       | 82 (0x0052)   | Yes                | No                  | Instance of Mode Base                 |
| 119       | Relative Humidity Measurement                              | 1029 (0x0405) | Yes                | Yes                 |                                       |
| 120       | Scenes                                                     | 5 (0x0005)    | No                 | No                  |                                       |
| 121       | Scenes Management                                          | 98 (0x0062)   | Yes                | Yes                 |                                       |
| 122       | Service Area                                               | 336 (0x0150)  | Yes                | Yes                 | Used by Robotic Vacuum Cleaner        |
| 123       | Smoke CO Alarm                                             | 92 (0x005C)   | Yes                | Yes                 |                                       |
| 124       | Software Diagnostics                                       | 52 (0x0034)   | Yes                | Yes                 |                                       |
| 125       | Soil Measurement                                           | 1072 (0x0430) | Yes                | Yes                 |                                       |
| 126       | Soil Moisture Measurement                                  | 1032 (0x0408) | No                 | No                  |                                       |
| 127       | Switch                                                     | 59 (0x003B)   | Yes                | Yes                 |                                       |
| 128       | TLS Certificate Management                                 | 2049 (0x0801) | Yes                | No                  |                                       |
| 129       | TLS Client Management                                      | 2050 (0x0802) | Yes                | No                  |                                       |
| 130       | Target Navigator                                           | 1285 (0x0505) | No                 | No                  |                                       |
| 131       | Temperature Alarm                                          | 100 (0x0064)  | No                 | No                  |                                       |
| 132       | Temperature Control                                        | 86 (0x0056)   | Yes                | Yes                 |                                       |
| 133       | Temperature Controlled Cabinet Topology                    | 75 (0x004B)   | No                 | No                  |                                       |
| 134       | Temperature Measurement                                    | 1026 (0x0402) | Yes                | Yes                 |                                       |
| 135       | Thermostat                                                 | 513 (0x0201)  | Yes                | Yes                 | Room Air Conditioner                  |
| 136       | Thermostat Mode                                            | 99 (0x0063)   | Yes                | No                  | Instance of Mode Base                 |
| 137       | Thermostat User Interface Configuration                    | 516 (0x0204)  | Yes                | Yes                 | Room Air Conditioner                  |
| 138       | Thread Border Router Diagnostics                           | 1108 (0x0454) | No                 | No                  |                                       |
| 139       | Thread Border Router Management                            | 1106 (0x0452) | Yes                | Yes                 |                                       |
| 140       | Thread Network Diagnostics                                 | 53 (0x0035)   | Yes                | Yes                 |                                       |
| 141       | Thread Network Directory                                   | 1107 (0x0453) | Yes                | Yes                 |                                       |
| 142       | Time Format Localization                                   | 44 (0x002C)   | Yes                | No                  |                                       |
| 143       | Time Synchronization                                       | 56 (0x0038)   | Yes                | No                  |                                       |
| 144       | Total Volatile Organic Compounds Concentration Measurement | 1070 (0x042E) | Yes                | Yes                 | Instance of Concentration Measurement |
| 145       | Unit Localization                                          | 45 (0x002D)   | Yes                | No                  |                                       |
| 146       | User Label                                                 | 65 (0x0041)   | Yes                | No                  |                                       |
| 147       | Valid Proxies                                              | 68 (0x0044)   | No                 | No                  |                                       |
| 148       | Valve Configuration and Control                            | 129 (0x0081)  | Yes                | Yes                 | Water Valve                           |
| 149       | Wake On LAN                                                | 1283 (0x0503) | No                 | No                  |                                       |
| 150       | Water Heater Management                                    | 148 (0x0094)  | Yes                | No                  |                                       |
| 151       | Water Heater Mode                                          | 158 (0x009E)  | Yes                | No                  | Instance of Mode Base                 |
| 152       | Water Tank Level Monitoring                                | 121 (0x0079)  | Yes                | No                  | Alias of Resource Monitoring          |
| 153       | WebRTC Transport Provider                                  | 1363 (0x0553) | Yes                | No                  |                                       |
| 154       | WebRTC Transport Requestor                                 | 1364 (0x0554) | Yes                | No                  |                                       |
| 155       | Wi-Fi Network Diagnostics                                  | 54 (0x0036)   | Yes                | Yes                 |                                       |
| 156       | Wi-Fi Network Management                                   | 1105 (0x0451) | Yes                | Yes                 |                                       |
| 157       | Window Covering                                            | 258 (0x0102)  | Yes                | Yes                 | window-covering                       |
| 158       | Zone Management                                            | 1360 (0x0550) | Yes                | No                  |                                       |
| 159       | Smoke Concentration Measurement                            | 1076 (0x0434) | Yes                | Yes                 | Instance of Concentration Measurement |
| **Total** | **159**                                                    |               | **108**            | **73**              |                                       |
