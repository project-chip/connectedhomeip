# Matter Clusters Implementation Status (159 total)

**Updated as of**: 2026-06-12 (Matter Specification SHA: `5a31ae2acb487bea09286243ccb5ad4ca9d3ef08`)

## Implementation Progress Summary

| Metric | Count | Description |
| :--- | :---: | :--- |
| **Total Spec-Defined Clusters** | **159** | Cataloged from the Matter Specification |
| **Code-Driven in SDK** | **86** | Migrated to the code-driven data model |
| **Used in All-Devices App** | **42** | Supported in the all-devices reference simulator |
| **Pending Migration** | **73** | Ember-based/codegen-driven clusters requiring migration to code-driven |

## Discovery & Updating Methodology

To update or validate this list manually, follow these steps:

1. **Verify Specification Catalog**:
   - Refer to the master spec cluster sheet: `docs/ids_and_codes/spec_clusters.md` (updated automatically).
   - This provides the cluster names, decimal IDs, and hex IDs.

2. **Verify Code-Driven in SDK Status**:
   - Search/grep to locate all cluster classes implementing the code-driven model. For example, if using ripgrep (rg):
     `rg -n "public\s+(chip::app::)?DefaultServerCluster" src/app/`
     and
     `rg -n "public\s+(chip::app::)?ServerClusterInterface" src/app/`
   - Every C++ class found represents a code-driven cluster. Map its cluster ID to this table.
   - Note: Generic/templated implementations (e.g. `ConcentrationMeasurementCluster` in `src/app/clusters/concentration-measurement-server/` or `ResourceMonitoringCluster` in `src/app/clusters/resource-monitoring-server/`) implement multiple distinct spec-defined clusters (e.g. Carbon Dioxide, HEPA Filter Monitoring). All mapped clusters should be marked as "Yes".

3. **Verify Used in All-Devices**:
   - Search/grep to find which clusters are dynamically registered in the all-devices-app. For example, if using ripgrep (rg):
     `rg "LazyRegisteredServerCluster<|RegisteredServerCluster<" examples/all-devices-app/`
   - Check where these are registered. They are typically added to the endpoint in the device implementation files under `examples/all-devices-app/all-devices-common/devices/` (e.g. `provider.AddCluster(...)`).
   - Mark as "Yes" only if they are actively instantiated and added to an endpoint.

## Cluster Implementation Details

| Cluster Name                                               | ID            | Code-Driven in SDK | Used in All-Devices | Notes                                 |
| ---------------------------------------------------------- | ------------- | ------------------ | ------------------- | ------------------------------------- |
| AV Analysis                                                | 1367 (0x0557) | No                 | No                  |                                       |
| Access Control                                             | 31 (0x001F)   | Yes                | Yes                 |                                       |
| Account Login                                              | 1294 (0x050E) | No                 | No                  |                                       |
| Actions                                                    | 37 (0x0025)   | Yes                | No                  |                                       |
| Activated Carbon Filter Monitoring                         | 114 (0x0072)  | Yes                | No                  | Alias of Resource Monitoring          |
| Administrator Commissioning                                | 60 (0x003C)   | Yes                | Yes                 |                                       |
| Air Quality                                                | 91 (0x005B)   | Yes                | Yes                 |                                       |
| Ambient Context Sensing                                    | 1073 (0x0431) | No                 | No                  |                                       |
| Ambient Sensing Union                                      | 1074 (0x0432) | No                 | No                  |                                       |
| Application Basic                                          | 1293 (0x050D) | No                 | No                  |                                       |
| Application Launcher                                       | 1292 (0x050C) | No                 | No                  |                                       |
| Audio Control                                              | 1298 (0x0512) | No                 | No                  |                                       |
| Audio Output                                               | 1291 (0x050B) | No                 | No                  |                                       |
| Ballast Configuration                                      | 769 (0x0301)  | No                 | No                  |                                       |
| Basic Information                                          | 40 (0x0028)   | Yes                | Yes                 |                                       |
| Binding                                                    | 30 (0x001E)   | Yes                | No                  |                                       |
| Boolean State                                              | 69 (0x0045)   | Yes                | Yes                 |                                       |
| Boolean State Configuration                                | 128 (0x0080)  | Yes                | No                  |                                       |
| Bridged Device Basic Information                           | 57 (0x0039)   | Yes                | Yes                 |                                       |
| Camera AV Settings User Level Management                   | 1362 (0x0552) | Yes                | No                  |                                       |
| Camera AV Stream Management                                | 1361 (0x0551) | Yes                | No                  |                                       |
| Carbon Dioxide Concentration Measurement                   | 1037 (0x040D) | Yes                | Yes                 | Instance of Concentration Measurement |
| Carbon Monoxide Concentration Measurement                  | 1036 (0x040C) | Yes                | Yes                 | Instance of Concentration Measurement |
| Channel                                                    | 1284 (0x0504) | No                 | No                  |                                       |
| Chime                                                      | 1366 (0x0556) | Yes                | Yes                 |                                       |
| Closure Control                                            | 260 (0x0104)  | Yes                | No                  |                                       |
| Closure Dimension                                          | 261 (0x0105)  | Yes                | No                  |                                       |
| Color Control                                              | 768 (0x0300)  | No                 | No                  |                                       |
| Commissioner Control                                       | 1873 (0x0751) | Yes                | No                  |                                       |
| Commissioning Proxy                                        | 1109 (0x0455) | No                 | No                  |                                       |
| Commodity Metering                                         | 2823 (0x0B07) | No                 | No                  |                                       |
| Commodity Price                                            | 149 (0x0095)  | No                 | No                  |                                       |
| Commodity Tariff                                           | 1792 (0x0700) | No                 | No                  |                                       |
| Content App Observer                                       | 1296 (0x0510) | No                 | No                  |                                       |
| Content Control                                            | 1295 (0x050F) | No                 | No                  |                                       |
| Content Launcher                                           | 1290 (0x050A) | No                 | No                  |                                       |
| Demand Response Load Control                               | 150 (0x0096)  | No                 | No                  |                                       |
| Descriptor                                                 | 29 (0x001D)   | Yes                | Yes                 |                                       |
| Device Energy Management                                   | 152 (0x0098)  | Yes                | No                  |                                       |
| Device Energy Management Mode                              | 159 (0x009F)  | No                 | No                  | Instance of Mode Base                 |
| Diagnostic Logs                                            | 50 (0x0032)   | Yes                | No                  |                                       |
| Dishwasher Alarm                                           | 93 (0x005D)   | No                 | No                  |                                       |
| Dishwasher Mode                                            | 89 (0x0059)   | No                 | No                  | Instance of Mode Base                 |
| Door Lock                                                  | 257 (0x0101)  | No                 | No                  |                                       |
| Dynamic Lighting                                           | 773 (0x0305)  | No                 | No                  |                                       |
| Ecosystem Information                                      | 1872 (0x0750) | No                 | No                  |                                       |
| Electrical Alarm                                           | 161 (0x00A1)  | No                 | No                  |                                       |
| Electrical Distribution                                    | 162 (0x00A2)  | No                 | No                  |                                       |
| Electrical Energy Measurement                              | 145 (0x0091)  | Yes                | No                  |                                       |
| Electrical Grid Conditions                                 | 160 (0x00A0)  | No                 | No                  |                                       |
| Electrical Power Measurement                               | 144 (0x0090)  | Yes                | No                  |                                       |
| Electrical Protection Alarm                                | 163 (0x00A3)  | No                 | No                  |                                       |
| Energy EVSE                                                | 153 (0x0099)  | Yes                | No                  |                                       |
| Energy EVSE Mode                                           | 157 (0x009D)  | No                 | No                  | Instance of Mode Base                 |
| Energy Preference                                          | 155 (0x009B)  | No                 | No                  |                                       |
| Ethernet Network Diagnostics                               | 55 (0x0037)   | Yes                | No                  |                                       |
| Fan Control                                                | 514 (0x0202)  | Yes                | Yes                 |                                       |
| Fixed Label                                                | 64 (0x0040)   | Yes                | No                  |                                       |
| Flow Measurement                                           | 1028 (0x0404) | Yes                | No                  |                                       |
| Formaldehyde Concentration Measurement                     | 1067 (0x042B) | Yes                | Yes                 | Instance of Concentration Measurement |
| General Commissioning                                      | 48 (0x0030)   | Yes                | Yes                 |                                       |
| General Diagnostics                                        | 51 (0x0033)   | Yes                | Yes                 |                                       |
| Group Key Management                                       | 63 (0x003F)   | Yes                | Yes                 |                                       |
| Groupcast                                                  | 101 (0x0065)  | Yes                | Yes                 |                                       |
| Groups                                                     | 4 (0x0004)    | Yes                | Yes                 |                                       |
| HEPA Filter Monitoring                                     | 113 (0x0071)  | Yes                | No                  | Alias of Resource Monitoring          |
| Humidistat                                                 | 517 (0x0205)  | Yes                | No                  |                                       |
| ICD Management                                             | 70 (0x0046)   | Yes                | No                  |                                       |
| Identify                                                   | 3 (0x0003)    | Yes                | Yes                 |                                       |
| Illuminance Measurement                                    | 1024 (0x0400) | Yes                | No                  |                                       |
| Joint Fabric Administrator                                 | 1875 (0x0753) | No                 | No                  |                                       |
| Joint Fabric Datastore                                     | 1874 (0x0752) | No                 | No                  |                                       |
| Keypad Input                                               | 1289 (0x0509) | No                 | No                  |                                       |
| Laundry Dryer Controls                                     | 74 (0x004A)   | No                 | No                  |                                       |
| Laundry Washer Controls                                    | 83 (0x0053)   | No                 | No                  |                                       |
| Laundry Washer Mode                                        | 81 (0x0051)   | No                 | No                  | Instance of Mode Base                 |
| Leaf Wetness Measurement                                   | 1031 (0x0407) | No                 | No                  |                                       |
| Level Control                                              | 8 (0x0008)    | Yes                | Yes                 |                                       |
| Localization Configuration                                 | 43 (0x002B)   | Yes                | No                  |                                       |
| Low Power                                                  | 1288 (0x0508) | No                 | No                  |                                       |
| Media File Management                                      | 1297 (0x0511) | No                 | No                  |                                       |
| Media Input                                                | 1287 (0x0507) | No                 | No                  |                                       |
| Media Playback                                             | 1286 (0x0506) | No                 | No                  |                                       |
| Messages                                                   | 151 (0x0097)  | No                 | No                  |                                       |
| Meter Identification                                       | 2822 (0x0B06) | No                 | No                  |                                       |
| Microwave Oven Control                                     | 95 (0x005F)   | Yes                | No                  |                                       |
| Microwave Oven Mode                                        | 94 (0x005E)   | No                 | No                  | Instance of Mode Base                 |
| Mode Select                                                | 80 (0x0050)   | No                 | No                  |                                       |
| Network Commissioning                                      | 49 (0x0031)   | Yes                | Yes                 |                                       |
| Network Identity Management                                | 1104 (0x0450) | Yes                | No                  |                                       |
| Nitrogen Dioxide Concentration Measurement                 | 1043 (0x0413) | Yes                | Yes                 | Instance of Concentration Measurement |
| OTA Software Update Provider                               | 41 (0x0029)   | Yes                | No                  |                                       |
| OTA Software Update Requestor                              | 42 (0x002A)   | Yes                | No                  |                                       |
| Occupancy Sensing                                          | 1030 (0x0406) | Yes                | Yes                 |                                       |
| On/Off                                                     | 6 (0x0006)    | Yes                | Yes                 |                                       |
| Operational Credentials                                    | 62 (0x003E)   | Yes                | Yes                 |                                       |
| Operational State                                          | 96 (0x0060)   | No                 | No                  |                                       |
| Oven Cavity Operational State                              | 72 (0x0048)   | No                 | No                  |                                       |
| Oven Mode                                                  | 73 (0x0049)   | No                 | No                  | Instance of Mode Base                 |
| Ozone Concentration Measurement                            | 1045 (0x0415) | Yes                | Yes                 | Instance of Concentration Measurement |
| PM1 Concentration Measurement                              | 1068 (0x042C) | Yes                | Yes                 | Instance of Concentration Measurement |
| PM10 Concentration Measurement                             | 1069 (0x042D) | Yes                | Yes                 | Instance of Concentration Measurement |
| PM2.5 Concentration Measurement                            | 1066 (0x042A) | Yes                | Yes                 | Instance of Concentration Measurement |
| Power Source                                               | 47 (0x002F)   | Yes                | Yes                 |                                       |
| Power Source Configuration                                 | 46 (0x002E)   | No                 | No                  |                                       |
| Power Topology                                             | 156 (0x009C)  | Yes                | No                  |                                       |
| Pressure Measurement                                       | 1027 (0x0403) | Yes                | No                  |                                       |
| Proximity Ranging                                          | 1075 (0x0433) | Yes                | Yes                 |                                       |
| Proxy Configuration                                        | 66 (0x0042)   | No                 | No                  |                                       |
| Proxy Discovery                                            | 67 (0x0043)   | No                 | No                  |                                       |
| Pulse Width Modulation                                     | 28 (0x001C)   | No                 | No                  |                                       |
| Pump Configuration and Control                             | 512 (0x0200)  | No                 | No                  |                                       |
| Push AV Stream Transport                                   | 1365 (0x0555) | Yes                | No                  |                                       |
| RVC Clean Mode                                             | 85 (0x0055)   | No                 | No                  | Instance of Mode Base                 |
| RVC Operational State                                      | 97 (0x0061)   | No                 | No                  |                                       |
| RVC Run Mode                                               | 84 (0x0054)   | No                 | No                  | Instance of Mode Base                 |
| Radon Concentration Measurement                            | 1071 (0x042F) | Yes                | Yes                 | Instance of Concentration Measurement |
| Refrigerator Alarm                                         | 87 (0x0057)   | No                 | No                  |                                       |
| Refrigerator And Temperature Controlled Cabinet Mode       | 82 (0x0052)   | No                 | No                  | Instance of Mode Base                 |
| Relative Humidity Measurement                              | 1029 (0x0405) | Yes                | No                  |                                       |
| Scenes                                                     | 5 (0x0005)    | No                 | No                  |                                       |
| Scenes Management                                          | 98 (0x0062)   | Yes                | Yes                 |                                       |
| Service Area                                               | 336 (0x0150)  | No                 | No                  |                                       |
| Smoke CO Alarm                                             | 92 (0x005C)   | Yes                | Yes                 |                                       |
| Software Diagnostics                                       | 52 (0x0034)   | Yes                | Yes                 |                                       |
| Soil Measurement                                           | 1072 (0x0430) | Yes                | Yes                 |                                       |
| Soil Moisture Measurement                                  | 1032 (0x0408) | No                 | No                  |                                       |
| Switch                                                     | 59 (0x003B)   | Yes                | No                  |                                       |
| TLS Certificate Management                                 | 2049 (0x0801) | Yes                | No                  |                                       |
| TLS Client Management                                      | 2050 (0x0802) | Yes                | No                  |                                       |
| Target Navigator                                           | 1285 (0x0505) | No                 | No                  |                                       |
| Temperature Alarm                                          | 100 (0x0064)  | No                 | No                  |                                       |
| Temperature Control                                        | 86 (0x0056)   | Yes                | No                  |                                       |
| Temperature Controlled Cabinet Topology                    | 75 (0x004B)   | No                 | No                  |                                       |
| Temperature Measurement                                    | 1026 (0x0402) | Yes                | Yes                 |                                       |
| Thermostat                                                 | 513 (0x0201)  | No                 | No                  |                                       |
| Thermostat Mode                                            | 99 (0x0063)   | No                 | No                  | Instance of Mode Base                 |
| Thermostat User Interface Configuration                    | 516 (0x0204)  | No                 | No                  |                                       |
| Thread Border Router Diagnostics                           | 1108 (0x0454) | No                 | No                  |                                       |
| Thread Border Router Management                            | 1106 (0x0452) | Yes                | Yes                 |                                       |
| Thread Network Diagnostics                                 | 53 (0x0035)   | Yes                | Yes                 |                                       |
| Thread Network Directory                                   | 1107 (0x0453) | Yes                | Yes                 |                                       |
| Time Format Localization                                   | 44 (0x002C)   | Yes                | No                  |                                       |
| Time Synchronization                                       | 56 (0x0038)   | Yes                | No                  |                                       |
| Total Volatile Organic Compounds Concentration Measurement | 1070 (0x042E) | Yes                | Yes                 | Instance of Concentration Measurement |
| Unit Localization                                          | 45 (0x002D)   | Yes                | No                  |                                       |
| User Label                                                 | 65 (0x0041)   | Yes                | No                  |                                       |
| Valid Proxies                                              | 68 (0x0044)   | No                 | No                  |                                       |
| Valve Configuration and Control                            | 129 (0x0081)  | Yes                | No                  |                                       |
| Wake On LAN                                                | 1283 (0x0503) | No                 | No                  |                                       |
| Water Heater Management                                    | 148 (0x0094)  | Yes                | No                  |                                       |
| Water Heater Mode                                          | 158 (0x009E)  | No                 | No                  | Instance of Mode Base                 |
| Water Tank Level Monitoring                                | 121 (0x0079)  | Yes                | No                  | Alias of Resource Monitoring          |
| WebRTC Transport Provider                                  | 1363 (0x0553) | Yes                | No                  |                                       |
| WebRTC Transport Requestor                                 | 1364 (0x0554) | Yes                | No                  |                                       |
| Wi-Fi Network Diagnostics                                  | 54 (0x0036)   | Yes                | Yes                 |                                       |
| Wi-Fi Network Management                                   | 1105 (0x0451) | Yes                | Yes                 |                                       |
| Window Covering                                            | 258 (0x0102)  | No                 | No                  |                                       |
| Zone Management                                            | 1360 (0x0550) | Yes                | No                  |                                       |
