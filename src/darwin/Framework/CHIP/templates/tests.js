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

function getTests()
{
  const BinaryInput = [
    'Test_TC_BI_1_1',
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
  ];

  const DeviceManagement = [
    'Test_TC_DM_1_1',
    'Test_TC_DM_3_1',
  ];

  const FlowMeasurement = [
    'Test_TC_FLW_1_1',
    'Test_TC_FLW_2_1',
    'Test_TC_FLW_2_2',
  ];

  const OccupancySensing = [
    'Test_TC_OCC_1_1',
    'Test_TC_OCC_2_1',
  ];

  const LevelControl = [
    'Test_TC_LVL_1_1',
    'Test_TC_LVL_2_1',
    'Test_TC_LVL_3_1',
  ];

  const MediaControl = [
    'Test_TC_MC_1_1',
    'Test_TC_MC_3_6',
    'Test_TC_MC_3_7',
    'Test_TC_MC_3_8',
    'Test_TC_MC_3_9',
    'Test_TC_MC_3_10',
    'Test_TC_MC_3_11',
  ];

  const OnOff = [
    'Test_TC_OO_1_1',
    'Test_TC_OO_2_1',
    'Test_TC_OO_2_2',
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
  ];

  const TemperatureMeasurement = [
    'Test_TC_TM_1_1',
    'Test_TC_TM_2_1',
  ];

  const Thermostat = [
    'Test_TC_TSTAT_1_1',
  ];

  const ThermostatUserConfiguration = [
    'Test_TC_TSUIC_1_1',
    'Test_TC_TSUIC_2_1',
  ];

  const ThreadNetworkDiagnostics = [
    'Test_TC_DIAGTH_1_1',
  ];

  const WindowCovering = [
    'Test_TC_WNCV_1_1',
    'Test_TC_WNCV_2_1',
    'Test_TC_WNCV_3_1',
    'Test_TC_WNCV_3_2',
    'Test_TC_WNCV_3_3',
  ];

  const Others = [
    'TestCluster',
    'TestConstraints',
    'TestDelayCommands',
    'TestDescriptorCluster',
    'TestBasicInformation',
    'TestIdentifyCluster',
    'TestOperationalCredentialsCluster',
  ];

  const Subscriptions = [
    'TestSubscribe_OnOff',
  ];

  const tests = [
    BinaryInput, //
    ColorControl, //
    DeviceManagement, //
    FlowMeasurement, //
    LevelControl, //
    MediaControl, //
    OccupancySensing, //
    OnOff, //
    PumpConfigurationControl, //
    RelativeHumidityMeasurement, //
    TemperatureMeasurement, //
    Thermostat, //
    ThermostatUserConfiguration, //
    ThreadNetworkDiagnostics, //
    WindowCovering, //
    Others, //
    Subscriptions, //
  ];
  return tests.flat(1);
}

//
// Module exports
//
exports.getTests = getTests;
