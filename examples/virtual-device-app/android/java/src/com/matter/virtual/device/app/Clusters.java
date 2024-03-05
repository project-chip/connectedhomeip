/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
package com.matter.virtual.device.app;

public class Clusters {

  public static final long ClusterId_PowerConfiguration = 0x00000001;
  public static final long ClusterId_DeviceTemperatureConfiguration = 0x00000002;
  public static final long ClusterId_Identify = 0x00000003;
  public static final long ClusterId_Groups = 0x00000004;
  public static final long ClusterId_ScenesManagement = 0x00000062;
  public static final long ClusterId_OnOff = 0x00000006;
  public static final long ClusterId_OnOffSwitchConfiguration = 0x00000007;
  public static final long ClusterId_LevelControl = 0x00000008;
  public static final long ClusterId_Alarms = 0x00000009;
  public static final long ClusterId_Time = 0x0000000A;
  public static final long ClusterId_BinaryInputBasic = 0x0000000F;
  public static final long ClusterId_PowerProfile = 0x0000001A;
  public static final long ClusterId_ApplianceControl = 0x0000001B;
  public static final long ClusterId_PulseWidthModulation = 0x0000001C;
  public static final long ClusterId_Descriptor = 0x0000001D;
  public static final long ClusterId_Binding = 0x0000001E;
  public static final long ClusterId_AccessControl = 0x0000001F;
  public static final long ClusterId_PollControl = 0x00000020;
  public static final long ClusterId_Actions = 0x00000025;
  public static final long ClusterId_Basic = 0x00000028;
  public static final long ClusterId_OtaSoftwareUpdateProvider = 0x00000029;
  public static final long ClusterId_OtaSoftwareUpdateRequestor = 0x0000002A;
  public static final long ClusterId_LocalizationConfiguration = 0x0000002B;
  public static final long ClusterId_LocalizationTimeFormat = 0x0000002C;
  public static final long ClusterId_LocalizationUnit = 0x0000002D;
  public static final long ClusterId_PowerSourceConfiguration = 0x0000002E;
  public static final long ClusterId_PowerSource = 0x0000002F;
  public static final long ClusterId_GeneralCommissioning = 0x00000030;
  public static final long ClusterId_NetworkCommissioning = 0x00000031;
  public static final long ClusterId_DiagnosticLogs = 0x00000032;
  public static final long ClusterId_GeneralDiagnostics = 0x00000033;
  public static final long ClusterId_SoftwareDiagnostics = 0x00000034;
  public static final long ClusterId_ThreadNetworkDiagnostics = 0x00000035;
  public static final long ClusterId_WiFiNetworkDiagnostics = 0x00000036;
  public static final long ClusterId_EthernetNetworkDiagnostics = 0x00000037;
  public static final long ClusterId_TimeSynchronization = 0x00000038;
  public static final long ClusterId_BridgedDeviceBasic = 0x00000039;
  public static final long ClusterId_Switch = 0x0000003B;
  public static final long ClusterId_AdministratorCommissioning = 0x0000003C;
  public static final long ClusterId_OperationalCredentials = 0x0000003E;
  public static final long ClusterId_GroupKeyManagement = 0x0000003F;
  public static final long ClusterId_FixedLabel = 0x00000040;
  public static final long ClusterId_UserLabel = 0x00000041;
  public static final long ClusterId_ProxyConfiguration = 0x00000042;
  public static final long ClusterId_ProxyDiscovery = 0x00000043;
  public static final long ClusterId_ProxyValid = 0x00000044;
  public static final long ClusterId_BooleanState = 0x00000045;
  public static final long ClusterId_ModeSelect = 0x00000050;
  public static final long ClusterId_ShadeConfiguration = 0x00000100;
  public static final long ClusterId_DoorLock = 0x00000101;
  public static final long ClusterId_WindowCovering = 0x00000102;
  public static final long ClusterId_BarrierControl = 0x00000103;
  public static final long ClusterId_PumpConfigurationAndControl = 0x00000200;
  public static final long ClusterId_Thermostat = 0x00000201;
  public static final long ClusterId_FanControl = 0x00000202;
  public static final long ClusterId_DehumidificationControl = 0x00000203;
  public static final long ClusterId_ThermostatUserInterfaceConfiguration = 0x00000204;
  public static final long ClusterId_ColorControl = 0x00000300;
  public static final long ClusterId_BallastConfiguration = 0x00000301;
  public static final long ClusterId_IlluminanceMeasurement = 0x00000400;
  public static final long ClusterId_TemperatureMeasurement = 0x00000402;
  public static final long ClusterId_PressureMeasurement = 0x00000403;
  public static final long ClusterId_FlowMeasurement = 0x00000404;
  public static final long ClusterId_RelativeHumidityMeasurement = 0x00000405;
  public static final long ClusterId_OccupancySensing = 0x00000406;
  public static final long ClusterId_CarbonMonoxideConcentrationMeasurement = 0x0000040C;
  public static final long ClusterId_CarbonDioxideConcentrationMeasurement = 0x0000040D;
  public static final long ClusterId_EthyleneConcentrationMeasurement = 0x0000040E;
  public static final long ClusterId_EthyleneOxideConcentrationMeasurement = 0x0000040F;
  public static final long ClusterId_HydrogenConcentrationMeasurement = 0x00000410;
  public static final long ClusterId_HydrogenSulphideConcentrationMeasurement = 0x00000411;
  public static final long ClusterId_NitricOxideConcentrationMeasurement = 0x00000412;
  public static final long ClusterId_NitrogenDioxideConcentrationMeasurement = 0x00000413;
  public static final long ClusterId_OxygenConcentrationMeasurement = 0x00000414;
  public static final long ClusterId_OzoneConcentrationMeasurement = 0x00000415;
  public static final long ClusterId_SulfurDioxideConcentrationMeasurement = 0x00000416;
  public static final long ClusterId_DissolvedOxygenConcentrationMeasurement = 0x00000417;
  public static final long ClusterId_BromateConcentrationMeasurement = 0x00000418;
  public static final long ClusterId_ChloraminesConcentrationMeasurement = 0x00000419;
  public static final long ClusterId_ChlorineConcentrationMeasurement = 0x0000041A;
  public static final long ClusterId_FecalColiformAndEColiConcentrationMeasurement = 0x0000041B;
  public static final long ClusterId_FluorideConcentrationMeasurement = 0x0000041C;
  public static final long ClusterId_HaloaceticAcidsConcentrationMeasurement = 0x0000041D;
  public static final long ClusterId_TotalTrihalomethanesConcentrationMeasurement = 0x0000041E;
  public static final long ClusterId_TotalColiformBacteriaConcentrationMeasurement = 0x0000041F;
  public static final long ClusterId_TurbidityConcentrationMeasurement = 0x00000420;
  public static final long ClusterId_CopperConcentrationMeasurement = 0x00000421;
  public static final long ClusterId_LeadConcentrationMeasurement = 0x00000422;
  public static final long ClusterId_ManganeseConcentrationMeasurement = 0x00000423;
  public static final long ClusterId_SulfateConcentrationMeasurement = 0x00000424;
  public static final long ClusterId_BromodichloromethaneConcentrationMeasurement = 0x00000425;
  public static final long ClusterId_BromoformConcentrationMeasurement = 0x00000426;
  public static final long ClusterId_ChlorodibromomethaneConcentrationMeasurement = 0x00000427;
  public static final long ClusterId_ChloroformConcentrationMeasurement = 0x00000428;
  public static final long ClusterId_SodiumConcentrationMeasurement = 0x00000429;
  public static final long ClusterId_IasZone = 0x00000500;
  public static final long ClusterId_IasAce = 0x00000501;
  public static final long ClusterId_IasWd = 0x00000502;
  public static final long ClusterId_WakeOnLan = 0x00000503;
  public static final long ClusterId_Channel = 0x00000504;
  public static final long ClusterId_TargetNavigator = 0x00000505;
  public static final long ClusterId_MediaPlayback = 0x00000506;
  public static final long ClusterId_MediaInput = 0x00000507;
  public static final long ClusterId_LowPower = 0x00000508;
  public static final long ClusterId_KeypadInput = 0x00000509;
  public static final long ClusterId_ContentLauncher = 0x0000050A;
  public static final long ClusterId_AudioOutput = 0x0000050B;
  public static final long ClusterId_ApplicationLauncher = 0x0000050C;
  public static final long ClusterId_ApplicationBasic = 0x0000050D;
  public static final long ClusterId_AccountLogin = 0x0000050E;
  public static final long ClusterId_TestCluster = 0xFFF1FC05;
  public static final long ClusterId_Messaging = 0x00000703;
  public static final long ClusterId_ApplianceIdentification = 0x00000B00;
  public static final long ClusterId_MeterIdentification = 0x00000B01;
  public static final long ClusterId_ApplianceEventsAndAlert = 0x00000B02;
  public static final long ClusterId_ApplianceStatistics = 0x00000B03;
  public static final long ClusterId_ElectricalMeasurement = 0x00000B04;
}
