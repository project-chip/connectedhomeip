/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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

function disable(testName)
{
  const index = this.indexOf(testName);
  if (index == -1) {
    const errStr = `Test ${testName}  does not exists.`;
    throw new Error(errStr);
  }

  this.splice(index, 1);
}

// clang-format off

function getManualTests() {
  const DeviceDiscovery = [
    'Test_TC_DD_1_5',
    'Test_TC_DD_1_6',
    'Test_TC_DD_1_7',
    'Test_TC_DD_1_8',
    'Test_TC_DD_1_9',
    'Test_TC_DD_1_10',
    'Test_TC_DD_1_11',
    'Test_TC_DD_1_12',
    'Test_TC_DD_2_1',
    'Test_TC_DD_2_2',
    'Test_TC_DD_3_1',
    'Test_TC_DD_3_2',
    'Test_TC_DD_3_3',
    'Test_TC_DD_3_4',
    'Test_TC_DD_3_5',
    'Test_TC_DD_3_6',
    'Test_TC_DD_3_7',
    'Test_TC_DD_3_8',
    'Test_TC_DD_3_9',
    'Test_TC_DD_3_10',
    'Test_TC_DD_3_11',
    'Test_TC_DD_3_12',
    'Test_TC_DD_3_13',
    'Test_TC_DD_3_14',
    'Test_TC_DD_4_1',
  ];

  const Groups = [
    'TestGroupDemoCommand',
    'TestGroupDemoConfig',
  ];

  const BulkDataExchangeProtocol = [
    'Test_TC_BDX_1_1',
    'Test_TC_BDX_1_2',
    'Test_TC_BDX_1_3',
    'Test_TC_BDX_1_4',
    'Test_TC_BDX_1_5',
    'Test_TC_BDX_1_6',
    'Test_TC_BDX_2_1',
    'Test_TC_BDX_2_2',
    'Test_TC_BDX_2_3',
    'Test_TC_BDX_2_4',
    'Test_TC_BDX_2_5',
  ];

  const bridge = [
    'Test_TC_BR_1',
    'Test_TC_BR_2',
    'Test_TC_BR_3',
  ];

  const BridgedActions = [
    'Test_TC_BRAC_2_1',
    'Test_TC_BRAC_2_2',
    'Test_TC_BRAC_3_1',
  ];

  const DeviceAttestation = [
    'Test_TC_DA_1_3',
  ];

  const DeviceManagement = [
    'Test_TC_DM_1_2',
    'Test_TC_DM_1_4',
    'Test_TC_DM_2_1',
    'Test_TC_DM_2_4',
    'Test_TC_DM_3_2',
    'Test_TC_DM_3_4',
    'Test_TC_DM_4_1',
    'Test_TC_DM_4_2',
    'Test_TC_DM_4_3',
    'Test_TC_DM_4_4',
    'Test_TC_DM_4_5',
    'Test_TC_DM_4_6',
    'Test_TC_DM_4_7',
    'Test_TC_DM_4_8',
    'Test_TC_DM_4_9',
    'Test_TC_DM_4_10',
    'Test_TC_DM_4_11',
    'Test_TC_DM_4_12',
    'Test_TC_DM_4_13',
    'Test_TC_DM_4_14',
    'Test_TC_DM_4_15',
    'Test_TC_DM_4_16',
    'Test_TC_DM_4_17',
    'Test_TC_DM_4_18',
    'Test_TC_DM_4_19',
    'Test_TC_DM_4_20',
    'Test_TC_DM_4_21',
  ];

  const DiagnosticsLogs = [
    'Test_TC_Diag_Log_2_1',
    'Test_TC_Diag_Log_2_2',
    'Test_TC_Diag_Log_2_3',
  ];

  const Descriptor = [
    'Test_TC_DESC_1_1',
  ];

  const EthernetNetworkDiagnostics = [
    'Test_TC_ETHDIAG_1_2',
  ];

  const GeneralCommissioning = [
    'Test_TC_GC_2_2',
    'Test_TC_GC_2_3',
    'Test_TC_GC_2_4',
  ];

  const GeneralDiagnostics = [
    'Test_TC_GENDIAG_1_1',
    'Test_TC_GENDIAG_1_2',
    'Test_TC_GENDIAG_2_1',
  ];

  const Identify = [
    'Test_TC_I_2_2',
    'Test_TC_I_3_1',
    'Test_TC_I_3_2',
  ];

  const IlluminanceMeasurement = [
    'Test_TC_ILL_2_2',
    'Test_TC_ILL_3_1',
  ];

  const InteractionDataModel = [
    'Test_TC_IDM_1_1',
    'Test_TC_IDM_1_2',
    'Test_TC_IDM_2_1',
    'Test_TC_IDM_2_2',
    'Test_TC_IDM_3_1',
    'Test_TC_IDM_3_2',
    'Test_TC_IDM_4_1',
    'Test_TC_IDM_4_2',
    'Test_TC_IDM_5_1',
    'Test_TC_IDM_5_2',
    'Test_TC_IDM_6_1',
    'Test_TC_IDM_6_2',
    'Test_TC_IDM_6_3',
    'Test_TC_IDM_6_4',
    'Test_TC_IDM_7_1',
  ];

  const MediaControl = [
    'Test_TC_MC_2_2',
    'Test_TC_MC_4_1',
    'Test_TC_MC_6_1',
    'Test_TC_MC_6_2',
    'Test_TC_MC_6_3',
    'Test_TC_MC_6_4',
    'Test_TC_MC_8_2',
    'Test_TC_MC_9_2',
    'Test_TC_MC_10_2',
    'Test_TC_MC_10_3',
    'Test_TC_MC_10_4',
    'Test_TC_MC_10_5',
    'Test_TC_MC_10_6',
    'Test_TC_MC_10_7',
  ];

  const MultipleFabrics = [
    'Test_TC_MF_1_1',
    'Test_TC_MF_1_2',
    'Test_TC_MF_1_7',
    'Test_TC_MF_1_8',
    'Test_TC_MF_1_9',
    'Test_TC_MF_1_10',
    'Test_TC_MF_1_11',
    'Test_TC_MF_1_12',
    'Test_TC_MF_1_13',
    'Test_TC_MF_1_14',
    'Test_TC_MF_1_16',
    'Test_TC_MF_1_17',
    'Test_TC_MF_1_18',
    'Test_TC_MF_1_19',
    'Test_TC_MF_1_20',
    'Test_TC_MF_1_21',
    'Test_TC_MF_1_22',
    'Test_TC_MF_1_23',
    'Test_TC_MF_1_24',
    'Test_TC_MF_1_25',
    'Test_TC_MF_1_26',
    'Test_TC_MF_1_27',
    'Test_TC_MF_1_28',
  ];

  const ModeSelect = [
    'Test_TC_MOD_1_2',
    'Test_TC_MOD_1_3',
    'Test_TC_MOD_2_1',
    'Test_TC_MOD_2_2',
    'Test_TC_MOD_3_1',
    'Test_TC_MOD_3_2',
    'Test_TC_MOD_3_3',
  ];

  const OTASoftwareUpdate = [
    'Test_TC_SU_1_1',
    'Test_TC_SU_2_1',
    'Test_TC_SU_2_2',
    'Test_TC_SU_2_3',
    'Test_TC_SU_2_4',
    'Test_TC_SU_2_5',
    'Test_TC_SU_2_6',
    'Test_TC_SU_2_7',
    'Test_TC_SU_3_1',
    'Test_TC_SU_3_2',
    'Test_TC_SU_3_3',
    'Test_TC_SU_3_4',
    'Test_TC_SU_4_1',
    'Test_TC_SU_4_2',
  ];

  const PowerSourceConfiguration = [
    'Test_TC_PSCFG_2_1',
    'Test_TC_PSCFG_2_2',
    'Test_TC_PSCFG_3_1',
  ];

  const SecureChannel = [
    'Test_TC_SC_4_1',
    'Test_TC_SC_4_3',
    'Test_TC_SC_4_4',
    'Test_TC_SC_4_5',
    'Test_TC_SC_4_6',
    'Test_TC_SC_4_7',
    'Test_TC_SC_4_8',
    'Test_TC_SC_4_9',
    'Test_TC_SC_4_10',
  ];

  const SoftwareDiagnostics = [
    'Test_TC_SWDIAG_1_2',
  ];

  const WiFiNetworkDiagnostics = [
    'Test_TC_WIFIDIAG_1_2',
    'Test_TC_WIFIDIAG_2_1',
  ];

  const WindowCovering = [
    'Test_TC_WNCV_6_1',
    'Test_TC_WNCV_7_1',
  ];

  const FlowMeasurement = [
    'Test_TC_FLW_3_1',
  ];

  const OccupancySensing = [
    'Test_TC_OCC_2_2',
    'Test_TC_OCC_2_3',
    'Test_TC_OCC_2_4',
    'Test_TC_OCC_3_1',
    'Test_TC_OCC_3_2',
  ];

  const PressureMeasurement = [
    'Test_TC_PRS_2_2',
    'Test_TC_PRS_2_3',
    'Test_TC_PRS_3_1',
  ];

  const PowerSource = [
    'Test_TC_PS_2_2',
    'Test_TC_PS_3_1',
  ];

  const BooleanState = [
    'Test_TC_BOOL_2_2',
    'Test_TC_BOOL_3_1',
  ];

  const ColorControl = [
    'Test_TC_CC_2_2',
    'Test_TC_CC_3_4',
    'Test_TC_CC_4_5',
    'Test_TC_CC_5_4',
    'Test_TC_CC_6_4',
    'Test_TC_CC_7_5',
    'Test_TC_CC_9_4',
  ];

  const DoorLock = [
    'Test_TC_DL_2_1',
    'Test_TC_DL_2_5',
    'Test_TC_DL_2_6',
    'Test_TC_DL_2_7',
    'Test_TC_DL_2_10',
    'Test_TC_DL_2_13',
    'Test_TC_DL_2_14',
    'Test_TC_DL_2_15',
    'Test_TC_DL_2_16',
    'Test_TC_DL_2_17',
  ];

  const UserLabel = [
    'Test_TC_LC_1_1',
    'Test_TC_LC_2_3',
    'Test_TC_LC_2_4',
    'Test_TC_LC_2_5',
  ];

  const LocalizationConfiguration = [
    'Test_TC_LO_1_1',
  ];

  const LevelControl = [
    'Test_TC_LVL_2_3',
  ];

  const NetworkCommissioning = [
    'Test_TC_NC_1_1',
    'Test_TC_NC_1_2',
    'Test_TC_NC_1_3',
    'Test_TC_NC_1_4',
    'Test_TC_NC_1_5',
    'Test_TC_NC_1_6',
    'Test_TC_NC_1_7',
    'Test_TC_NC_1_8',
    'Test_TC_NC_1_9',
    'Test_TC_NC_1_10',
    'Test_TC_NC_1_11',
    'Test_TC_NC_1_12',
    'Test_TC_NC_1_13',
    'Test_TC_NC_1_14',
    'Test_TC_NC_1_15',
    'Test_TC_NC_1_16',
    'Test_TC_NC_1_17',
    'Test_TC_NC_1_18',
  ];

  const OnOff = [
    'Test_TC_OO_3_1',
    'Test_TC_OO_3_2',
  ];

  const RelativeHumidityMeasurement = [
    'Test_TC_RH_3_1',
  ];

  const Switch = [
    'Test_TC_SWTCH_1_1',
    'Test_TC_SWTCH_3_1',
  ];

  const TemperatureMeasurement = [
    'Test_TC_TM_3_1',
  ];

  const Thermostat = [
    'Test_TC_TSTAT_3_1',
    'Test_TC_TSTAT_3_2',
  ];

  const ThermostatUserConfiguration = [
    'Test_TC_TSUIC_3_1',
  ];

  const ThreadNetworkDiagnostics = [
    'Test_TC_DIAG_TH_NW_2_6',
    'Test_TC_DIAG_TH_NW_2_7',
    'Test_TC_DIAG_TH_NW_2_8',
    'Test_TC_DIAG_TH_NW_2_9',
  ];


  const tests = [
    DeviceDiscovery,
    Groups,
    BulkDataExchangeProtocol,
    bridge,
    BridgedActions,
    DeviceAttestation,
    DeviceManagement,
    DiagnosticsLogs,
    Descriptor,
    EthernetNetworkDiagnostics,
    GeneralCommissioning,
    GeneralDiagnostics,
    Identify,
    IlluminanceMeasurement,
    InteractionDataModel,
    MediaControl,
    MultipleFabrics,
    ModeSelect,
    OTASoftwareUpdate,
    PowerSourceConfiguration,
    SecureChannel,
    SoftwareDiagnostics,
    WiFiNetworkDiagnostics,
    WindowCovering,
    FlowMeasurement,
    OccupancySensing,
    PressureMeasurement,
    PowerSource,
    BooleanState,
    ColorControl,
    DoorLock,
    UserLabel,
    LocalizationConfiguration,
    LevelControl,
    NetworkCommissioning,
    OnOff,
    RelativeHumidityMeasurement,
    Switch,
    TemperatureMeasurement,
    Thermostat,
    ThermostatUserConfiguration,
    ThreadNetworkDiagnostics,
  ].flat(1);

  tests.disable = disable.bind(tests);
  return tests;
}

function getTests() {
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

  const DeviceManagement = [
    'Test_TC_DM_1_1',
    'Test_TC_DM_3_1',
    'Test_TC_DM_2_2',
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

  const GeneralCommissioning = [
    'Test_TC_GC_1_1',
    'Test_TC_GC_2_1',
  ];

  const Identify = [
    'Test_TC_I_1_1',
    'Test_TC_I_2_1',
    'Test_TC_I_2_3',
  ];

  const IlluminanceMeasurement = [
    'Test_TC_ILL_1_1',
    'Test_TC_ILL_2_1',
  ];

  const OccupancySensing = [
    'Test_TC_OCC_1_1',
    'Test_TC_OCC_2_1',
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

  const UserLabel = [
    'Test_TC_LC_1_2',
  ];

  const MediaControl = [
    'Test_TC_MC_1_1',
    'Test_TC_MC_1_2',
    'Test_TC_MC_1_3',
    'Test_TC_MC_1_4',
    'Test_TC_MC_1_5',
    'Test_TC_MC_1_6',
    'Test_TC_MC_1_7',
    'Test_TC_MC_1_8',
    'Test_TC_MC_1_9',
    'Test_TC_MC_1_10',
    'Test_TC_MC_1_11',
    'Test_TC_MC_1_12',
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
    'Test_TC_MC_3_12',
    'Test_TC_MC_3_13',
    'Test_TC_MC_5_1',
    'Test_TC_MC_5_2',
    'Test_TC_MC_5_3',
    'Test_TC_MC_7_1',
    'Test_TC_MC_7_2',
    'Test_TC_MC_8_1',
    'Test_TC_MC_9_1',
    'Test_TC_MC_10_1',
  ];

  const ModeSelect = [
    'Test_TC_MOD_1_1',
  ];

  const MultipleFabrics = [
    'Test_TC_MF_1_3',
    'Test_TC_MF_1_4',
    'Test_TC_MF_1_5',
    'Test_TC_MF_1_6',
    'Test_TC_MF_1_15',
  ];

  const OTASoftwareUpdate = [
    'OTA_SuccessfulTransfer',
  ];

  const OnOff = [
    'Test_TC_OO_1_1',
    'Test_TC_OO_2_1',
    'Test_TC_OO_2_2',
    'Test_TC_OO_2_3',
    'Test_TC_OO_2_4',
  ];

  const PowerSource = [
    'Test_TC_PS_1_1',
    'Test_TC_PS_2_1',
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
    'Test_TC_PCC_2_4',
  ];

  const PowerSourceConfiguration = [
    'Test_TC_PSCFG_1_1',
  ];

  const RelativeHumidityMeasurement = [
    'Test_TC_RH_1_1',
    'Test_TC_RH_2_1',
    'Test_TC_RH_2_2',
  ];

  const SecureChannel = [
    'Test_TC_SC_4_2',
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
    'Test_TC_DIAG_TH_NW_1_2',
    'Test_TC_DIAG_TH_NW_2_2',
    'Test_TC_DIAG_TH_NW_2_3',
  ];

  const WiFiNetworkDiagnostics = [
    'Test_TC_WIFIDIAG_1_1',
    'Test_TC_WIFIDIAG_3_1',
  ];

  const WindowCovering = [
    'Test_TC_WNCV_1_1',
    'Test_TC_WNCV_2_1',
    'Test_TC_WNCV_2_2',
    'Test_TC_WNCV_2_3',
    'Test_TC_WNCV_2_4',
    'Test_TC_WNCV_2_5',
    'Test_TC_WNCV_3_1',
    'Test_TC_WNCV_3_2',
    'Test_TC_WNCV_3_3',
    'Test_TC_WNCV_3_4',
    'Test_TC_WNCV_3_5',
    'Test_TC_WNCV_4_1',
    'Test_TC_WNCV_4_2',
    'Test_TC_WNCV_4_3',
    'Test_TC_WNCV_4_4',
    'Test_TC_WNCV_4_5',
  ];

  const TV = [
    'TV_TargetNavigatorCluster',
    'TV_AudioOutputCluster',
    'TV_ApplicationLauncherCluster',
    'TV_KeypadInputCluster',
    'TV_AccountLoginCluster',
    'TV_WakeOnLanCluster',
    'TV_ApplicationBasicCluster',
    'TV_MediaPlaybackCluster',
    'TV_ChannelCluster',
    'TV_LowPowerCluster',
    'TV_ContentLauncherCluster',
    'TV_MediaInputCluster',
  ];

  const Others = [
    'TestCluster',
    'TestClusterComplexTypes',
    'TestConstraints',
    'TestDelayCommands',
    'TestEvents',
    'TestDiscovery',
    'TestLogCommands',
    'TestSaveAs',
    'TestConfigVariables',
    'TestDescriptorCluster',
    'TestBasicInformation',
    'TestGeneralCommissioning',
    'TestIdentifyCluster',
    'TestOperationalCredentialsCluster',
    'TestModeSelectCluster',
    'TestSelfFabricRemoval',
    'TestSystemCommands',
    'TestBinding',
    'TestUserLabelCluster',
    'TestArmFailSafe',
    'TestFanControl',
  ];

  const MultiAdmin = [
    'TestMultiAdmin',
  ];

  const SoftwareDiagnostics = [
    'Test_TC_SWDIAG_1_1',
    'Test_TC_SWDIAG_2_1',
    'Test_TC_SWDIAG_3_1',
  ];

  const Subscriptions = [
    'TestSubscribe_OnOff',
  ];

  const DoorLock = [
    'DL_UsersAndCredentials',
    'DL_LockUnlock',
    'DL_Schedules',
    'Test_TC_DL_2_2',
    'Test_TC_DL_2_3',
    'Test_TC_DL_2_4',
    'Test_TC_DL_2_8',
    'Test_TC_DL_2_11',
  ];

  const Groups = [
    'TestGroupMessaging',
    'TestGroupsCluster',
    'TestGroupKeyManagementCluster',
  ];

  const tests = [
    AccessControl,
    BinaryInput,
    BooleanState,
    BridgedActions,
    ColorControl,
    DeviceManagement,
    ElectricalMeasurement,
    EthernetNetworkDiagnostics,
    FlowMeasurement,
    GeneralCommissioning,
    Identify,
    IlluminanceMeasurement,
    LevelControl,
    MediaControl,
    ModeSelect,
    MultipleFabrics,
    OTASoftwareUpdate,
    OccupancySensing,
    OnOff,
    PowerSource,
    PressureMeasurement,
    PumpConfigurationControl,
    PowerSourceConfiguration,
    RelativeHumidityMeasurement,
    SecureChannel,
    Switch,
    TemperatureMeasurement,
    Thermostat,
    ThermostatUserConfiguration,
    ThreadNetworkDiagnostics,
    UserLabel,
    WiFiNetworkDiagnostics,
    WindowCovering,
    TV,
    Others,
    MultiAdmin,
    SoftwareDiagnostics,
    Subscriptions,
    DoorLock,
    Groups,
  ].flat(1);

  tests.disable = disable.bind(tests);
  return tests;
}

// clang-format on

//
// Module exports
//
exports.getTests       = getTests;
exports.getManualTests = getManualTests;
