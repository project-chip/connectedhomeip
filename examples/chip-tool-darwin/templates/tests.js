/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

function getManualTests()
{
  return [];
}

function getTests()
{
  const AccessControl = [
    'TestAccessControlCluster',
  ];

  const BinaryInput = [
    'Test_TC_BI_1_1',
    'Test_TC_BI_2_1',
    'Test_TC_BI_2_2',
  ];

  const BooleanState = [
    'Test_TC_BOOL_1_1',
    'Test_TC_BOOL_2_1',
  ];

  const BridgedActions = [
    'Test_TC_BRAC_1_1',
  ];

  const ColorControl = [
    'Test_TC_CC_1_1',
    'Test_TC_CC_2_1',
    'Test_TC_CC_3_1',
    'Test_TC_CC_3_2',
    'Test_TC_CC_3_3',
    'Test_TC_CC_4_1',
    'Test_TC_CC_4_2',
    'Test_TC_CC_4_3',
    'Test_TC_CC_4_4',
    'Test_TC_CC_5_1',
    'Test_TC_CC_5_2',
    'Test_TC_CC_5_3',
    'Test_TC_CC_6_1',
    'Test_TC_CC_6_2',
    'Test_TC_CC_6_3',
    'Test_TC_CC_7_1',
    'Test_TC_CC_7_2',
    'Test_TC_CC_7_3',
    'Test_TC_CC_7_4',
    'Test_TC_CC_8_1',
    'Test_TC_CC_9_1',
    'Test_TC_CC_9_2',
    'Test_TC_CC_9_3',
  ];

  const DeviceDiscovery = [
    'Test_TC_DD_1_5',
    'Test_TC_DD_1_6',
    'Test_TC_DD_1_7',
    'Test_TC_DD_1_8',
    'Test_TC_DD_1_9',
  ];

  const DeviceManagement = [
    'Test_TC_DM_1_1',
    'Test_TC_DM_3_1',
  ];

  const ElectricalMeasurement = [
    'Test_TC_EMR_1_1',
  ];

  const EthernetNetworkDiagnostics = [
    'Test_TC_ETHDIAG_1_1',
    'Test_TC_ETHDIAG_2_1',
  ];

  const FlowMeasurement = [
    'Test_TC_FLW_1_1',
    'Test_TC_FLW_2_1',
    'Test_TC_FLW_2_2',
  ];

  const IlluminanceMeasurement = [
    'Test_TC_ILL_1_1',
  ];

  const OccupancySensing = [
    'Test_TC_OCC_1_1',
    'Test_TC_OCC_2_1',
    'Test_TC_OCC_2_2',
  ];

  const LevelControl = [
    'Test_TC_LVL_1_1',
    'Test_TC_LVL_2_1',
    'Test_TC_LVL_2_2',
    'Test_TC_LVL_3_1',
    'Test_TC_LVL_4_1',
    'Test_TC_LVL_5_1',
    'Test_TC_LVL_6_1',
  ];

  const MediaControl = [
    'Test_TC_MC_1_1',
    'Test_TC_MC_2_1',
    'Test_TC_MC_3_1',
    'Test_TC_MC_3_2',
    'Test_TC_MC_3_3',
    'Test_TC_MC_3_4',
    'Test_TC_MC_3_5',
    'Test_TC_MC_3_6',
    'Test_TC_MC_3_7',
    'Test_TC_MC_3_8',
    'Test_TC_MC_3_9',
    'Test_TC_MC_3_10',
    'Test_TC_MC_3_11',
    'Test_TC_MC_5_1',
    'Test_TC_MC_5_2',
    'Test_TC_MC_5_3',
    'Test_TC_MC_6_1',
    'Test_TC_MC_6_2',
    'Test_TC_MC_6_3',
    'Test_TC_MC_6_4',
    'Test_TC_MC_7_1',
    'Test_TC_MC_7_2',
    'Test_TC_MC_8_1',
    'Test_TC_MC_9_1',
  ];

  const OnOff = [
    'Test_TC_OO_1_1',
    'Test_TC_OO_2_1',
    'Test_TC_OO_2_2',
    'Test_TC_OO_2_3',
  ];

  const PowerSource = [
    'Test_TC_PS_1_1',
  ];

  const PressureMeasurement = [
    'Test_TC_PRS_1_1',
    'Test_TC_PRS_2_1',
  ];

  const PumpConfigurationControl = [
    'Test_TC_PCC_1_1',
    'Test_TC_PCC_2_1',
    'Test_TC_PCC_2_2',
    'Test_TC_PCC_2_3',
  ];

  const RelativeHumidityMeasurement = [
    'Test_TC_RH_1_1',
    'Test_TC_RH_2_1',
    'Test_TC_RH_2_2',
  ];

  const Switch = [
    'Test_TC_SWTCH_2_1',
    'Test_TC_SWTCH_2_2',
  ];

  const TemperatureMeasurement = [
    'Test_TC_TM_1_1',
    'Test_TC_TM_2_1',
    'Test_TC_TM_2_2',
  ];

  const Thermostat = [
    'Test_TC_TSTAT_1_1',
    'Test_TC_TSTAT_2_1',
    'Test_TC_TSTAT_2_2',
  ];

  const ThermostatUserConfiguration = [
    'Test_TC_TSUIC_1_1',
    'Test_TC_TSUIC_2_1',
    'Test_TC_TSUIC_2_2',
  ];

  const ThreadNetworkDiagnostics = [
    'Test_TC_DIAG_TH_NW_1_1',
  ];

  const WiFiNetworkDiagnostics = [
    'Test_TC_WIFIDIAG_1_1',
    'Test_TC_WIFIDIAG_3_1',
  ];

  const WindowCovering = [
    // WindowCovering is make uses of multiples "subscribeAttribute", but it triggers
    // some failures on darwin supposely because the ReadClient stays open for the
    // whole duration of the tests and that goes past some internal limits.
    // Because of this, some of the tests are disabled on darwin.
    'Test_TC_WNCV_1_1',
    'Test_TC_WNCV_2_1',
    'Test_TC_WNCV_2_2',
    'Test_TC_WNCV_2_4',
    'Test_TC_WNCV_2_5',
    //'Test_TC_WNCV_3_1',
    //'Test_TC_WNCV_3_2',
    //'Test_TC_WNCV_3_3',
    'Test_TC_WNCV_3_4',
    'Test_TC_WNCV_3_5',
    'Test_TC_WNCV_4_3',
    'Test_TC_WNCV_4_4',
  ];

  const Others = [
    'TestCluster',
    'TestSaveAs',
    'TestConstraints',
    'TestDelayCommands',
    'TestDescriptorCluster',
    'TestBasicInformation',
    'TestGroupsCluster',
    'TestGroupKeyManagementCluster',
    'TestIdentifyCluster',
    'TestLogCommands',
    'TestOperationalCredentialsCluster',
    'TestBinding',
  ];

  const SoftwareDiagnostics = [
    'Test_TC_SWDIAG_1_1',
    'Test_TC_SWDIAG_2_1',
    'Test_TC_SWDIAG_3_1',
  ];

  const Subscriptions = [
    'TestSubscribe_OnOff',
  ];

  const tests = [
    AccessControl, //
    BinaryInput, //
    BooleanState, //
    BridgedActions, //
    ColorControl, //
    DeviceDiscovery, //
    DeviceManagement, //
    ElectricalMeasurement, //
    EthernetNetworkDiagnostics, //
    FlowMeasurement, //
    IlluminanceMeasurement, //
    LevelControl, //
    MediaControl, //
    OccupancySensing, //
    OnOff, //
    PowerSource, //
    PressureMeasurement, //
    PumpConfigurationControl, //
    RelativeHumidityMeasurement, //
    Switch, //
    TemperatureMeasurement, //
    Thermostat, //
    ThermostatUserConfiguration, //
    ThreadNetworkDiagnostics, //
    WiFiNetworkDiagnostics, //
    WindowCovering, //
    Others, //
    SoftwareDiagnostics, //
    Subscriptions, //
  ];
  return tests.flat(1);
}

//
// Module exports
//
exports.getTests       = getTests;
exports.getManualTests = getManualTests;
