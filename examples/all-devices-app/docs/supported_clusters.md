# Matter Clusters Implementation Status (159 total)

**Updated as of**: 2026-06-12 (Matter Specification SHA:
`5a31ae2acb487bea09286243ccb5ad4ca9d3ef08`)

## Discovery & Updating Methodology

To update or validate this list manually, follow these steps:

1. **Verify Specification Catalog**:

    - Refer to the master spec cluster sheet:
      `docs/ids_and_codes/spec_clusters.md` (updated automatically).
    - This provides the cluster names, decimal IDs, and hex IDs.

2. **Verify Code-Driven in SDK Status**:

    - Search/grep to locate all cluster classes implementing the code-driven
      model. For example, if using ripgrep (rg):
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
      all-devices-app. For example, if using ripgrep (rg):
      `rg "LazyRegisteredServerCluster<|RegisteredServerCluster<" examples/all-devices-app/`
    - Check where these are registered. They are typically added to the endpoint
      in the device implementation files under
      `examples/all-devices-app/all-devices-common/devices/` (e.g.
      `provider.AddCluster(...)`).
    - Mark as "Yes" only if they are actively instantiated and added to an
      endpoint.

## Cluster Implementation Details

| #   | Cluster Name                                               | ID            | Code-Driven in SDK | Used in All-Devices | Notes                                 |
| --- | ---------------------------------------------------------- | ------------- | ------------------ | ------------------- | ------------------------------------- |
| 1   | AV Analysis                                                | 1367 (0x0557) | No                 | No                  |                                       |
| 2   | Access Control                                             | 31 (0x001F)   | Yes (1)            | Yes (1)             |                                       |
| 3   | Account Login                                              | 1294 (0x050E) | No                 | No                  |                                       |
| 4   | Actions                                                    | 37 (0x0025)   | Yes (2)            | No                  |                                       |
| 5   | Activated Carbon Filter Monitoring                         | 114 (0x0072)  | Yes (3)            | No                  | Alias of Resource Monitoring          |
| 6   | Administrator Commissioning                                | 60 (0x003C)   | Yes (4)            | Yes (2)             |                                       |
| 7   | Air Quality                                                | 91 (0x005B)   | Yes (5)            | Yes (3)             |                                       |
| 8   | Ambient Context Sensing                                    | 1073 (0x0431) | No                 | No                  |                                       |
| 9   | Ambient Sensing Union                                      | 1074 (0x0432) | No                 | No                  |                                       |
| 10  | Application Basic                                          | 1293 (0x050D) | No                 | No                  |                                       |
| 11  | Application Launcher                                       | 1292 (0x050C) | No                 | No                  |                                       |
| 12  | Audio Control                                              | 1298 (0x0512) | No                 | No                  |                                       |
| 13  | Audio Output                                               | 1291 (0x050B) | No                 | No                  |                                       |
| 14  | Ballast Configuration                                      | 769 (0x0301)  | No                 | No                  |                                       |
| 15  | Basic Information                                          | 40 (0x0028)   | Yes (6)            | Yes (4)             |                                       |
| 16  | Binding                                                    | 30 (0x001E)   | Yes (7)            | No                  |                                       |
| 17  | Boolean State                                              | 69 (0x0045)   | Yes (8)            | Yes (5)             |                                       |
| 18  | Boolean State Configuration                                | 128 (0x0080)  | Yes (9)            | No                  |                                       |
| 19  | Bridged Device Basic Information                           | 57 (0x0039)   | Yes (10)           | Yes (6)             |                                       |
| 20  | Camera AV Settings User Level Management                   | 1362 (0x0552) | Yes (11)           | No                  |                                       |
| 21  | Camera AV Stream Management                                | 1361 (0x0551) | Yes (12)           | No                  |                                       |
| 22  | Carbon Dioxide Concentration Measurement                   | 1037 (0x040D) | Yes (13)           | Yes (7)             | Instance of Concentration Measurement |
| 23  | Carbon Monoxide Concentration Measurement                  | 1036 (0x040C) | Yes (14)           | Yes (8)             | Instance of Concentration Measurement |
| 24  | Channel                                                    | 1284 (0x0504) | No                 | No                  |                                       |
| 25  | Chime                                                      | 1366 (0x0556) | Yes (15)           | Yes (9)             |                                       |
| 26  | Closure Control                                            | 260 (0x0104)  | Yes (16)           | No                  |                                       |
| 27  | Closure Dimension                                          | 261 (0x0105)  | Yes (17)           | No                  |                                       |
| 28  | Color Control                                              | 768 (0x0300)  | No                 | No                  |                                       |
| 29  | Commissioner Control                                       | 1873 (0x0751) | Yes (18)           | No                  |                                       |
| 30  | Commissioning Proxy                                        | 1109 (0x0455) | No                 | No                  |                                       |
| 31  | Commodity Metering                                         | 2823 (0x0B07) | No                 | No                  |                                       |
| 32  | Commodity Price                                            | 149 (0x0095)  | No                 | No                  |                                       |
| 33  | Commodity Tariff                                           | 1792 (0x0700) | No                 | No                  |                                       |
| 34  | Content App Observer                                       | 1296 (0x0510) | No                 | No                  |                                       |
| 35  | Content Control                                            | 1295 (0x050F) | No                 | No                  |                                       |
| 36  | Content Launcher                                           | 1290 (0x050A) | No                 | No                  |                                       |
| 37  | Demand Response Load Control                               | 150 (0x0096)  | No                 | No                  |                                       |
| 38  | Descriptor                                                 | 29 (0x001D)   | Yes (19)           | Yes (10)            |                                       |
| 39  | Device Energy Management                                   | 152 (0x0098)  | Yes (20)           | No                  |                                       |
| 40  | Device Energy Management Mode                              | 159 (0x009F)  | No                 | No                  | Instance of Mode Base                 |
| 41  | Diagnostic Logs                                            | 50 (0x0032)   | Yes (21)           | No                  |                                       |
| 42  | Dishwasher Alarm                                           | 93 (0x005D)   | No                 | No                  |                                       |
| 43  | Dishwasher Mode                                            | 89 (0x0059)   | No                 | No                  | Instance of Mode Base                 |
| 44  | Door Lock                                                  | 257 (0x0101)  | No                 | No                  |                                       |
| 45  | Dynamic Lighting                                           | 773 (0x0305)  | No                 | No                  |                                       |
| 46  | Ecosystem Information                                      | 1872 (0x0750) | No                 | No                  |                                       |
| 47  | Electrical Alarm                                           | 161 (0x00A1)  | No                 | No                  |                                       |
| 48  | Electrical Distribution                                    | 162 (0x00A2)  | No                 | No                  |                                       |
| 49  | Electrical Energy Measurement                              | 145 (0x0091)  | Yes (22)           | No                  |                                       |
| 50  | Electrical Grid Conditions                                 | 160 (0x00A0)  | No                 | No                  |                                       |
| 51  | Electrical Power Measurement                               | 144 (0x0090)  | Yes (23)           | No                  |                                       |
| 52  | Electrical Protection Alarm                                | 163 (0x00A3)  | No                 | No                  |                                       |
| 53  | Energy EVSE                                                | 153 (0x0099)  | Yes (24)           | No                  |                                       |
| 54  | Energy EVSE Mode                                           | 157 (0x009D)  | No                 | No                  | Instance of Mode Base                 |
| 55  | Energy Preference                                          | 155 (0x009B)  | No                 | No                  |                                       |
| 56  | Ethernet Network Diagnostics                               | 55 (0x0037)   | Yes (25)           | No                  |                                       |
| 57  | Fan Control                                                | 514 (0x0202)  | Yes (26)           | Yes (11)            |                                       |
| 58  | Fixed Label                                                | 64 (0x0040)   | Yes (27)           | No                  |                                       |
| 59  | Flow Measurement                                           | 1028 (0x0404) | Yes (28)           | No                  |                                       |
| 60  | Formaldehyde Concentration Measurement                     | 1067 (0x042B) | Yes (29)           | Yes (12)            | Instance of Concentration Measurement |
| 61  | General Commissioning                                      | 48 (0x0030)   | Yes (30)           | Yes (13)            |                                       |
| 62  | General Diagnostics                                        | 51 (0x0033)   | Yes (31)           | Yes (14)            |                                       |
| 63  | Group Key Management                                       | 63 (0x003F)   | Yes (32)           | Yes (15)            |                                       |
| 64  | Groupcast                                                  | 101 (0x0065)  | Yes (33)           | Yes (16)            |                                       |
| 65  | Groups                                                     | 4 (0x0004)    | Yes (34)           | Yes (17)            |                                       |
| 66  | HEPA Filter Monitoring                                     | 113 (0x0071)  | Yes (35)           | No                  | Alias of Resource Monitoring          |
| 67  | Humidistat                                                 | 517 (0x0205)  | Yes (36)           | No                  |                                       |
| 68  | ICD Management                                             | 70 (0x0046)   | Yes (37)           | No                  |                                       |
| 69  | Identify                                                   | 3 (0x0003)    | Yes (38)           | Yes (18)            |                                       |
| 70  | Illuminance Measurement                                    | 1024 (0x0400) | Yes (39)           | No                  |                                       |
| 71  | Joint Fabric Administrator                                 | 1875 (0x0753) | No                 | No                  |                                       |
| 72  | Joint Fabric Datastore                                     | 1874 (0x0752) | No                 | No                  |                                       |
| 73  | Keypad Input                                               | 1289 (0x0509) | No                 | No                  |                                       |
| 74  | Laundry Dryer Controls                                     | 74 (0x004A)   | No                 | No                  |                                       |
| 75  | Laundry Washer Controls                                    | 83 (0x0053)   | No                 | No                  |                                       |
| 76  | Laundry Washer Mode                                        | 81 (0x0051)   | No                 | No                  | Instance of Mode Base                 |
| 77  | Leaf Wetness Measurement                                   | 1031 (0x0407) | No                 | No                  |                                       |
| 78  | Level Control                                              | 8 (0x0008)    | Yes (40)           | Yes (19)            |                                       |
| 79  | Localization Configuration                                 | 43 (0x002B)   | Yes (41)           | No                  |                                       |
| 80  | Low Power                                                  | 1288 (0x0508) | No                 | No                  |                                       |
| 81  | Media File Management                                      | 1297 (0x0511) | No                 | No                  |                                       |
| 82  | Media Input                                                | 1287 (0x0507) | No                 | No                  |                                       |
| 83  | Media Playback                                             | 1286 (0x0506) | No                 | No                  |                                       |
| 84  | Messages                                                   | 151 (0x0097)  | No                 | No                  |                                       |
| 85  | Meter Identification                                       | 2822 (0x0B06) | No                 | No                  |                                       |
| 86  | Microwave Oven Control                                     | 95 (0x005F)   | Yes (42)           | No                  |                                       |
| 87  | Microwave Oven Mode                                        | 94 (0x005E)   | No                 | No                  | Instance of Mode Base                 |
| 88  | Mode Select                                                | 80 (0x0050)   | No                 | No                  |                                       |
| 89  | Network Commissioning                                      | 49 (0x0031)   | Yes (43)           | Yes (20)            |                                       |
| 90  | Network Identity Management                                | 1104 (0x0450) | Yes (44)           | No                  |                                       |
| 91  | Nitrogen Dioxide Concentration Measurement                 | 1043 (0x0413) | Yes (45)           | Yes (21)            | Instance of Concentration Measurement |
| 92  | OTA Software Update Provider                               | 41 (0x0029)   | Yes (46)           | No                  |                                       |
| 93  | OTA Software Update Requestor                              | 42 (0x002A)   | Yes (47)           | No                  |                                       |
| 94  | Occupancy Sensing                                          | 1030 (0x0406) | Yes (48)           | Yes (22)            |                                       |
| 95  | On/Off                                                     | 6 (0x0006)    | Yes (49)           | Yes (23)            |                                       |
| 96  | Operational Credentials                                    | 62 (0x003E)   | Yes (50)           | Yes (24)            |                                       |
| 97  | Operational State                                          | 96 (0x0060)   | No                 | No                  |                                       |
| 98  | Oven Cavity Operational State                              | 72 (0x0048)   | No                 | No                  |                                       |
| 99  | Oven Mode                                                  | 73 (0x0049)   | No                 | No                  | Instance of Mode Base                 |
| 100 | Ozone Concentration Measurement                            | 1045 (0x0415) | Yes (51)           | Yes (25)            | Instance of Concentration Measurement |
| 101 | PM1 Concentration Measurement                              | 1068 (0x042C) | Yes (52)           | Yes (26)            | Instance of Concentration Measurement |
| 102 | PM10 Concentration Measurement                             | 1069 (0x042D) | Yes (53)           | Yes (27)            | Instance of Concentration Measurement |
| 103 | PM2.5 Concentration Measurement                            | 1066 (0x042A) | Yes (54)           | Yes (28)            | Instance of Concentration Measurement |
| 104 | Power Source                                               | 47 (0x002F)   | Yes (55)           | Yes (29)            |                                       |
| 105 | Power Source Configuration                                 | 46 (0x002E)   | No                 | No                  |                                       |
| 106 | Power Topology                                             | 156 (0x009C)  | Yes (56)           | No                  |                                       |
| 107 | Pressure Measurement                                       | 1027 (0x0403) | Yes (57)           | No                  |                                       |
| 108 | Proximity Ranging                                          | 1075 (0x0433) | Yes (58)           | Yes (30)            |                                       |
| 109 | Proxy Configuration                                        | 66 (0x0042)   | No                 | No                  |                                       |
| 110 | Proxy Discovery                                            | 67 (0x0043)   | No                 | No                  |                                       |
| 111 | Pulse Width Modulation                                     | 28 (0x001C)   | No                 | No                  |                                       |
| 112 | Pump Configuration and Control                             | 512 (0x0200)  | No                 | No                  |                                       |
| 113 | Push AV Stream Transport                                   | 1365 (0x0555) | Yes (59)           | No                  |                                       |
| 114 | RVC Clean Mode                                             | 85 (0x0055)   | No                 | No                  | Instance of Mode Base                 |
| 115 | RVC Operational State                                      | 97 (0x0061)   | No                 | No                  |                                       |
| 116 | RVC Run Mode                                               | 84 (0x0054)   | No                 | No                  | Instance of Mode Base                 |
| 117 | Radon Concentration Measurement                            | 1071 (0x042F) | Yes (60)           | Yes (31)            | Instance of Concentration Measurement |
| 118 | Refrigerator Alarm                                         | 87 (0x0057)   | No                 | No                  |                                       |
| 119 | Refrigerator And Temperature Controlled Cabinet Mode       | 82 (0x0052)   | No                 | No                  | Instance of Mode Base                 |
| 120 | Relative Humidity Measurement                              | 1029 (0x0405) | Yes (61)           | No                  |                                       |
| 121 | Scenes                                                     | 5 (0x0005)    | No                 | No                  |                                       |
| 122 | Scenes Management                                          | 98 (0x0062)   | Yes (62)           | Yes (32)            |                                       |
| 123 | Service Area                                               | 336 (0x0150)  | No                 | No                  |                                       |
| 124 | Smoke CO Alarm                                             | 92 (0x005C)   | Yes (63)           | Yes (33)            |                                       |
| 125 | Software Diagnostics                                       | 52 (0x0034)   | Yes (64)           | Yes (34)            |                                       |
| 126 | Soil Measurement                                           | 1072 (0x0430) | Yes (65)           | Yes (35)            |                                       |
| 127 | Soil Moisture Measurement                                  | 1032 (0x0408) | No                 | No                  |                                       |
| 128 | Switch                                                     | 59 (0x003B)   | Yes (66)           | No                  |                                       |
| 129 | TLS Certificate Management                                 | 2049 (0x0801) | Yes (67)           | No                  |                                       |
| 130 | TLS Client Management                                      | 2050 (0x0802) | Yes (68)           | No                  |                                       |
| 131 | Target Navigator                                           | 1285 (0x0505) | No                 | No                  |                                       |
| 132 | Temperature Alarm                                          | 100 (0x0064)  | No                 | No                  |                                       |
| 133 | Temperature Control                                        | 86 (0x0056)   | Yes (69)           | No                  |                                       |
| 134 | Temperature Controlled Cabinet Topology                    | 75 (0x004B)   | No                 | No                  |                                       |
| 135 | Temperature Measurement                                    | 1026 (0x0402) | Yes (70)           | Yes (36)            |                                       |
| 136 | Thermostat                                                 | 513 (0x0201)  | No                 | No                  |                                       |
| 137 | Thermostat Mode                                            | 99 (0x0063)   | No                 | No                  | Instance of Mode Base                 |
| 138 | Thermostat User Interface Configuration                    | 516 (0x0204)  | No                 | No                  |                                       |
| 139 | Thread Border Router Diagnostics                           | 1108 (0x0454) | No                 | No                  |                                       |
| 140 | Thread Border Router Management                            | 1106 (0x0452) | Yes (71)           | Yes (37)            |                                       |
| 141 | Thread Network Diagnostics                                 | 53 (0x0035)   | Yes (72)           | Yes (38)            |                                       |
| 142 | Thread Network Directory                                   | 1107 (0x0453) | Yes (73)           | Yes (39)            |                                       |
| 143 | Time Format Localization                                   | 44 (0x002C)   | Yes (74)           | No                  |                                       |
| 144 | Time Synchronization                                       | 56 (0x0038)   | Yes (75)           | No                  |                                       |
| 145 | Total Volatile Organic Compounds Concentration Measurement | 1070 (0x042E) | Yes (76)           | Yes (40)            | Instance of Concentration Measurement |
| 146 | Unit Localization                                          | 45 (0x002D)   | Yes (77)           | No                  |                                       |
| 147 | User Label                                                 | 65 (0x0041)   | Yes (78)           | No                  |                                       |
| 148 | Valid Proxies                                              | 68 (0x0044)   | No                 | No                  |                                       |
| 149 | Valve Configuration and Control                            | 129 (0x0081)  | Yes (79)           | No                  |                                       |
| 150 | Wake On LAN                                                | 1283 (0x0503) | No                 | No                  |                                       |
| 151 | Water Heater Management                                    | 148 (0x0094)  | Yes (80)           | No                  |                                       |
| 152 | Water Heater Mode                                          | 158 (0x009E)  | No                 | No                  | Instance of Mode Base                 |
| 153 | Water Tank Level Monitoring                                | 121 (0x0079)  | Yes (81)           | No                  | Alias of Resource Monitoring          |
| 154 | WebRTC Transport Provider                                  | 1363 (0x0553) | Yes (82)           | No                  |                                       |
| 155 | WebRTC Transport Requestor                                 | 1364 (0x0554) | Yes (83)           | No                  |                                       |
| 156 | Wi-Fi Network Diagnostics                                  | 54 (0x0036)   | Yes (84)           | Yes (41)            |                                       |
| 157 | Wi-Fi Network Management                                   | 1105 (0x0451) | Yes (85)           | Yes (42)            |                                       |
| 158 | Window Covering                                            | 258 (0x0102)  | No                 | No                  |                                       |
| 159 | Zone Management                                            | 1360 (0x0550) | Yes (86)           | No                  |                                       |
