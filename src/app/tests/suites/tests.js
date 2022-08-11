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
        "Test_TC_DD_1_5",
        "Test_TC_DD_1_6",
        "Test_TC_DD_1_7",
        "Test_TC_DD_1_8",
        "Test_TC_DD_1_9",
        "Test_TC_DD_1_10",
        "Test_TC_DD_1_11",
        "Test_TC_DD_1_12",
        "Test_TC_DD_1_13",
        "Test_TC_DD_1_14",
        "Test_TC_DD_1_15",
        "Test_TC_DD_2_1",
        "Test_TC_DD_2_2",
        "Test_TC_DD_3_1",
        "Test_TC_DD_3_2",
        "Test_TC_DD_3_3",
        "Test_TC_DD_3_4",
        "Test_TC_DD_3_5",
        "Test_TC_DD_3_6",
        "Test_TC_DD_3_7",
        "Test_TC_DD_3_8",
        "Test_TC_DD_3_9",
        "Test_TC_DD_3_10",
        "Test_TC_DD_3_11",
        "Test_TC_DD_3_12",
        "Test_TC_DD_3_13",
        "Test_TC_DD_3_14",
        "Test_TC_DD_3_15",
        "Test_TC_DD_3_16",
        "Test_TC_DD_3_17",
        "Test_TC_DD_3_18",
        "Test_TC_DD_3_19",
        "Test_TC_DD_3_20",
        "Test_TC_DD_3_21",
    ];

    const Groups = [
        "TestGroupDemoCommand",
        "TestGroupDemoConfig",
        "Test_TC_G_1_1",
        "Test_TC_G_2_1",
        "Test_TC_G_2_2",
        "Test_TC_G_2_3",
        "Test_TC_G_3_1",
        "Test_TC_G_3_2",
    ];

    const BulkDataExchangeProtocol = [
        "Test_TC_BDX_1_1",
        "Test_TC_BDX_1_2",
        "Test_TC_BDX_1_3",
        "Test_TC_BDX_1_4",
        "Test_TC_BDX_1_5",
        "Test_TC_BDX_1_6",
        "Test_TC_BDX_2_1",
        "Test_TC_BDX_2_2",
        "Test_TC_BDX_2_3",
        "Test_TC_BDX_2_4",
        "Test_TC_BDX_2_5",
    ];

    const bridge = [
        "Test_TC_BR_1",
        "Test_TC_BR_2",
        "Test_TC_BR_3",
        "Test_TC_BR_4",
    ];

    const DeviceAttestation = [
        "Test_TC_DA_1_1",
        "Test_TC_DA_1_2",
        "Test_TC_DA_1_3",
        "Test_TC_DA_1_4",
        "Test_TC_DA_1_5",
        "Test_TC_DA_1_6",
        "Test_TC_DA_1_7",
    ];

    const DeviceManagement = [
        "Test_TC_OPCREDS_1_2",
        "Test_TC_BINFO_2_2",
        "Test_TC_BINFO_2_4",
        "Test_TC_OPCREDS_3_1",
        "Test_TC_OPCREDS_3_2",
        "Test_TC_OPCREDS_3_3",
        "Test_TC_OPCREDS_3_4",
        "Test_TC_CNET_4_1",
        "Test_TC_CNET_4_2",
        "Test_TC_CNET_4_3",
        "Test_TC_CNET_4_4",
        "Test_TC_CNET_4_5",
        "Test_TC_CNET_4_6",
        "Test_TC_CNET_4_9",
        "Test_TC_CNET_4_10",
        "Test_TC_CNET_4_11",
        "Test_TC_CNET_4_12",
        "Test_TC_CNET_4_13",
        "Test_TC_CNET_4_14",
        "Test_TC_CNET_4_15",
        "Test_TC_CNET_4_16",
        "Test_TC_CNET_4_17",
        "Test_TC_CNET_4_18",
        "Test_TC_CNET_4_19",
        "Test_TC_CNET_4_20",
        "Test_TC_CNET_4_21",
        "Test_TC_CNET_4_22",
    ];

    const DiagnosticsLogs = [
        "Test_TC_DLOG_1_1",
        "Test_TC_DLOG_2_1",
        "Test_TC_DLOG_2_2",
        "Test_TC_DLOG_3_1",
    ];

    const Descriptor = [
        "Test_TC_DESC_2_1",
        "Test_TC_DESC_2_2",
    ];

    const EthernetNetworkDiagnostics = [
        "Test_TC_DGETH_1_1",
        "Test_TC_DGETH_3_1",
        "Test_TC_DGETH_3_2",
    ];

    const GeneralCommissioning = [
        "Test_TC_CGEN_2_2",
        "Test_TC_CGEN_2_4",
    ];

    const GeneralDiagnostics = [
        "Test_TC_DGGEN_2_2",
        "Test_TC_DGGEN_2_3",
        "Test_TC_DGGEN_3_1",
    ];

    const Identify = [
        "Test_TC_I_3_1",
        "Test_TC_I_3_2",
    ];

    const IlluminanceMeasurement = [
        "Test_TC_ILL_2_2",
        "Test_TC_ILL_3_1",
    ];

    const InteractionDataModel = [
        "Test_TC_IDM_1_1",
        "Test_TC_IDM_1_2",
        "Test_TC_IDM_2_1",
        "Test_TC_IDM_2_2",
        "Test_TC_IDM_3_1",
        "Test_TC_IDM_3_2",
        "Test_TC_IDM_4_1",
        "Test_TC_IDM_4_2",
        "Test_TC_IDM_4_3",
        "Test_TC_IDM_5_1",
        "Test_TC_IDM_5_2",
        "Test_TC_IDM_6_1",
        "Test_TC_IDM_6_2",
        "Test_TC_IDM_6_3",
        "Test_TC_IDM_6_4",
        "Test_TC_IDM_7_1",
        "Test_TC_IDM_8_1",
    ];

    const MediaControl = [
        "Test_TC_LOWPOWER_2_2",
        "Test_TC_APPLAUNCHER_3_7_1",
        "Test_TC_APPLAUNCHER_3_8_1",
        "Test_TC_APPLAUNCHER_3_9_1",
        "Test_TC_MEDIAINPUT_3_14",
        "Test_TC_MEDIAINPUT_3_15",
        "Test_TC_MEDIAINPUT_3_16",
        "Test_TC_MEDIAINPUT_3_17",
        "Test_TC_WAKEONLAN_4_1",
        "Test_TC_CHANNEL_5_4",
        "Test_TC_CHANNEL_5_5",
        "Test_TC_CHANNEL_5_6",
        "Test_TC_KEYPADINPUT_3_1",
        "Test_TC_MEDIAPLAYBACK_6_5",
        "Test_TC_MEDIAPLAYBACK_6_7",
        "Test_TC_AUDIOOUTPUT_7_3",
        "Test_TC_AUDIOOUTPUT_7_4",
        "Test_TC_CONTENTLAUNCHER_10_3",
        "Test_TC_CONTENTLAUNCHER_10_4",
        "Test_TC_CONTENTLAUNCHER_10_5",
        "Test_TC_CONTENTLAUNCHER_10_7",
        "Test_TC_MC_11_1",
        "Test_TC_MC_11_2",
        "Test_TC_ALOGIN_12_1",
        "Test_TC_ALOGIN_12_2",
    ];

    const MultipleFabrics = [
        "Test_TC_CADMIN_1_1",
        "Test_TC_CADMIN_1_2",
        "Test_TC_CADMIN_1_7",
        "Test_TC_CADMIN_1_8",
        "Test_TC_CADMIN_1_11",
        "Test_TC_CADMIN_1_12",
        "Test_TC_CADMIN_1_14",
        "Test_TC_CADMIN_1_15",
        "Test_TC_CADMIN_1_16",
        "Test_TC_CADMIN_1_17",
        "Test_TC_CADMIN_1_18",
        "Test_TC_CADMIN_1_19",
        "Test_TC_CADMIN_1_20",
        "Test_TC_CADMIN_1_21",
        "Test_TC_CADMIN_1_22",
        // Slow tests that should not run in CI because they take many minutes each
        "Test_TC_CADMIN_1_3",
        "Test_TC_CADMIN_1_4",
        "Test_TC_CADMIN_1_5",
        "Test_TC_CADMIN_1_6",
        "Test_TC_CADMIN_1_9",
        "Test_TC_CADMIN_1_10",
        "Test_TC_CADMIN_1_13",
        "Test_TC_CADMIN_1_23",
        "Test_TC_CADMIN_1_24",
    ];

    const ModeSelect = [
        "Test_TC_MOD_1_2",
        "Test_TC_MOD_1_3",
        "Test_TC_MOD_2_1",
        "Test_TC_MOD_2_2",
        "Test_TC_MOD_3_1",
        "Test_TC_MOD_3_2",
        "Test_TC_MOD_3_3",
        "Test_TC_MOD_3_4",
    ];

    const OTASoftwareUpdate = [
        "Test_TC_SU_1_1",
        "Test_TC_SU_2_1",
        "Test_TC_SU_2_2",
        "Test_TC_SU_2_3",
        "Test_TC_SU_2_4",
        "Test_TC_SU_2_5",
        "Test_TC_SU_2_6",
        "Test_TC_SU_2_7",
        "Test_TC_SU_2_8",
        "Test_TC_SU_3_1",
        "Test_TC_SU_3_2",
        "Test_TC_SU_3_3",
        "Test_TC_SU_3_4",
        "Test_TC_SU_4_1",
        "Test_TC_SU_4_2",
    ];

    const PowerSourceConfiguration = [
        "Test_TC_PSCFG_2_1",
        "Test_TC_PSCFG_2_2",
        "Test_TC_PSCFG_3_1",
    ];

    const SecureChannel = [
        "Test_TC_SC_1_1",
        "Test_TC_SC_1_2",
        "Test_TC_SC_1_3",
        "Test_TC_SC_1_4",
        "Test_TC_SC_2_1",
        "Test_TC_SC_2_2",
        "Test_TC_SC_2_3",
        "Test_TC_SC_2_4",
        "Test_TC_SC_3_1",
        "Test_TC_SC_3_2",
        "Test_TC_SC_3_3",
        "Test_TC_SC_3_4",
        "Test_TC_SC_3_6",
        "Test_TC_SC_4_1",
        "Test_TC_SC_4_3",
        "Test_TC_SC_4_4",
        "Test_TC_SC_4_5",
        "Test_TC_SC_4_6",
        "Test_TC_SC_4_7",
        "Test_TC_SC_4_8",
        "Test_TC_SC_4_9",
        "Test_TC_SC_4_10",
    ];

    const SoftwareDiagnostics = [
        "Test_TC_DGSW_1_1",
        "Test_TC_DGSW_3_1",
        "Test_TC_DGSW_3_2",
    ];

    const WiFiNetworkDiagnostics = [
        "Test_TC_DGWIFI_1_1",
        "Test_TC_DGWIFI_2_2",
        "Test_TC_DGWIFI_3_1",
        "Test_TC_DGWIFI_3_2",
    ];

    const WindowCovering = [
        "Test_TC_WNCV_6_1",
        "Test_TC_WNCV_7_1",
    ];

    const FlowMeasurement = [
        "Test_TC_FLW_2_2",
        "Test_TC_FLW_3_1",
    ];

    const OccupancySensing = [
        "Test_TC_OCC_2_2",
        "Test_TC_OCC_2_3",
        "Test_TC_OCC_2_4",
        "Test_TC_OCC_3_1",
        "Test_TC_OCC_3_2",
    ];

    const PressureMeasurement = [
        "Test_TC_PRS_2_2",
        "Test_TC_PRS_3_1",
    ];

    const PowerSource = [
        "Test_TC_PS_2_2",
        "Test_TC_PS_3_1",
    ];

    const BooleanState = [
        "Test_TC_BOOL_2_2",
        "Test_TC_BOOL_3_1",
    ];

    const ColorControl = [
        "Test_TC_CC_2_2",
        "Test_TC_CC_3_4",
        "Test_TC_CC_4_5",
        "Test_TC_CC_5_4",
        "Test_TC_CC_6_4",
        "Test_TC_CC_7_5",
        "Test_TC_CC_9_4",
        // Slow tests that should not run in CI because they take many minutes each
        "Test_TC_CC_3_1",
        "Test_TC_CC_7_1",
        "Test_TC_CC_9_1",
        "Test_TC_CC_9_2",
        "Test_TC_CC_9_3",
    ];

    const DoorLock = [
        "Test_TC_DRLK_1_1",
        "Test_TC_DRLK_2_1",
        "Test_TC_DRLK_2_6",
        "Test_TC_DRLK_2_8",
        "Test_TC_DRLK_2_10",
        "Test_TC_DRLK_3_1",
        "Test_TC_DRLK_3_2",
        "Test_TC_DRLK_3_3",
    ];

    const LocalizationConfiguration = [
        "Test_TC_LCFG_1_1",
        "Test_TC_LCFG_2_1",
        "Test_TC_LCFG_3_1",
    ];

    const LevelControl = [
        "Test_TC_LVL_2_3",
        "Test_TC_LVL_7_1",
        "Test_TC_LVL_8_1",
    ];

    const OnOff = [
        "Test_TC_OO_2_3",
        "Test_TC_OO_3_1",
        "Test_TC_OO_3_2",
    ];

    const RelativeHumidityMeasurement = [
        "Test_TC_RH_2_2",
        "Test_TC_RH_3_1",
    ];

    const Switch = [
        "Test_TC_SWTCH_2_2",
        "Test_TC_SWTCH_3_1",
        "Test_TC_SWTCH_3_2",
    ];

    const TemperatureMeasurement = [
        "Test_TC_TMP_2_2",
        "Test_TC_TMP_3_1",
    ];

    const Thermostat = [
        "Test_TC_TSTAT_3_1",
        "Test_TC_TSTAT_3_2",
    ];

    const ThermostatUserConfiguration = [
        "Test_TC_TSUIC_3_1",
    ];

    const ThreadNetworkDiagnostics = [
        "Test_TC_DGTHREAD_1_1",
        "Test_TC_DGTHREAD_2_5",
        "Test_TC_DGTHREAD_3_1",
        "Test_TC_DGTHREAD_3_2",
        "Test_TC_DGTHREAD_3_3",
        "Test_TC_DGTHREAD_3_4",
        "Test_TC_DGTHREAD_3_5",
    ];

    const Actions = [
        "Test_TC_ACT_2_1",
        "Test_TC_ACT_2_2",
        "Test_TC_ACT_3_1",
        "Test_TC_ACT_3_2",
    ];

    const TimeFormatLocalization = [
        "Test_TC_LTIME_1_1",
        "Test_TC_LTIME_1_2",
        "Test_TC_LTIME_2_1",
        "Test_TC_LTIME_3_1",
    ];

    const UnitLocalization = [
        "Test_TC_LUNIT_1_1",
        "Test_TC_LUNIT_1_2",
        "Test_TC_LUNIT_2_1",
        "Test_TC_LUNIT_3_1",
    ];

    const FixedLabel = [
        "Test_TC_FLABEL_1_1",
        "Test_TC_FLABEL_2_1",
        "Test_TC_FLABEL_3_1",
    ];

    const Binding = [
        "Test_TC_BIND_1_1",
        "Test_TC_BIND_2_1",
        "Test_TC_BIND_2_2",
        "Test_TC_BIND_2_3",
    ];

    const Scenes = [
        "Test_TC_S_1_1",
        "Test_TC_S_2_1",
        "Test_TC_S_2_2",
        "Test_TC_S_2_3",
        "Test_TC_S_3_1",
    ];

    const PumpConfigurationControl = [
        "Test_TC_PCC_3_1",
    ];

    const AccessControl = [
        "Test_TC_ACL_1_1",
        "Test_TC_ACL_2_1",
        "Test_TC_ACL_2_2",
        "Test_TC_ACL_2_3",
        "Test_TC_ACL_2_4",
        "Test_TC_ACL_2_5",
        "Test_TC_ACL_2_6",
        "Test_TC_ACL_2_7",
        "Test_TC_ACL_2_8",
        "Test_TC_ACL_2_9",
        "Test_TC_ACL_2_10",
    ];

    const UserLabel = [
        "Test_TC_ULABEL_2_1",
        "Test_TC_ULABEL_3_1",
    ];

    const BridgedDeviceBasicInformation = [
        "Test_TC_BRBINFO_1_1",
        "Test_TC_BRBINFO_2_1",
        "Test_TC_BRBINFO_2_2",
        "Test_TC_BRBINFO_2_3",
    ];

    const tests = [
        DeviceDiscovery,
        Groups,
        BulkDataExchangeProtocol,
        bridge,
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
        LocalizationConfiguration,
        LevelControl,
        OnOff,
        RelativeHumidityMeasurement,
        Switch,
        TemperatureMeasurement,
        Thermostat,
        ThermostatUserConfiguration,
        ThreadNetworkDiagnostics,
        Actions,
        TimeFormatLocalization,
        UnitLocalization,
        FixedLabel,
        Binding,
        Scenes,
        PumpConfigurationControl,
        AccessControl,
        UserLabel,
        BridgedDeviceBasicInformation,
    ].flat(1);

    tests.disable = disable.bind(tests);
    return tests;
}

function getTests() {
    const AccessControl = [
        "TestAccessControlCluster",
    ];

    const BooleanState = [
        "Test_TC_BOOL_1_1",
        "Test_TC_BOOL_2_1",
    ];

    const Actions = [
        "Test_TC_ACT_1_1",
    ];

    const ColorControl = [
        "Test_TC_CC_1_1",
        "Test_TC_CC_2_1",
        "Test_TC_CC_3_2",
        "Test_TC_CC_3_3",
        "Test_TC_CC_4_1",
        "Test_TC_CC_4_2",
        "Test_TC_CC_4_3",
        "Test_TC_CC_4_4",
        "Test_TC_CC_5_1",
        "Test_TC_CC_5_2",
        "Test_TC_CC_5_3",
        "Test_TC_CC_6_1",
        "Test_TC_CC_6_2",
        "Test_TC_CC_6_3",
        "Test_TC_CC_7_2",
        "Test_TC_CC_7_3",
        "Test_TC_CC_7_4",
        "Test_TC_CC_8_1",
    ];

    const DeviceManagement = [
        "Test_TC_BINFO_1_1",
        "Test_TC_BINFO_2_1",
        "Test_TC_CNET_1_3",
    ];

    const Descriptor = [
        "Test_TC_DESC_1_1",
    ];

    const EthernetNetworkDiagnostics = [
        "Test_TC_DGETH_2_1",
        "Test_TC_DGETH_2_2",
    ];

    const FlowMeasurement = [
        "Test_TC_FLW_1_1",
        "Test_TC_FLW_2_1",
    ];

    const GeneralCommissioning = [
        "Test_TC_CGEN_1_1",
        "Test_TC_CGEN_2_1",
    ];

    const GeneralDiagnostics = [
        "Test_TC_DGGEN_1_1",
        "Test_TC_DGGEN_2_1",
    ];

    const Identify = [
        "Test_TC_I_1_1",
        "Test_TC_I_2_1",
        "Test_TC_I_2_2",
        "Test_TC_I_2_3",
    ];

    const IlluminanceMeasurement = [
        "Test_TC_ILL_1_1",
        "Test_TC_ILL_2_1",
    ];

    const OccupancySensing = [
        "Test_TC_OCC_1_1",
        "Test_TC_OCC_2_1",
    ];

    const LevelControl = [
        "Test_TC_LVL_1_1",
        "Test_TC_LVL_2_1",
        "Test_TC_LVL_2_2",
        "Test_TC_LVL_3_1",
        "Test_TC_LVL_4_1",
        "Test_TC_LVL_5_1",
        "Test_TC_LVL_6_1",
    ];

    const UserLabel = [
        "Test_TC_ULABEL_1_1",
        "Test_TC_ULABEL_2_2",
        "Test_TC_ULABEL_2_3",
        "Test_TC_ULABEL_2_4",
    ];

    const MediaControl = [
        "Test_TC_LOWPOWER_1_1",
        "Test_TC_KEYPADINPUT_1_2",
        "Test_TC_APPLAUNCHER_1_3",
        "Test_TC_MEDIAINPUT_1_4",
        "Test_TC_WAKEONLAN_1_5",
        "Test_TC_CHANNEL_1_6",
        "Test_TC_MEDIAPLAYBACK_1_7",
        "Test_TC_AUDIOOUTPUT_1_8",
        "Test_TC_TGTNAV_1_9",
        "Test_TC_TGTNAV_8_2",
        "Test_TC_APBSC_1_10",
        "Test_TC_CONTENTLAUNCHER_1_11",
        "Test_TC_ALOGIN_1_12",
        "Test_TC_LOWPOWER_2_1",
        "Test_TC_KEYPADINPUT_3_2",
        "Test_TC_KEYPADINPUT_3_3",
        "Test_TC_APPLAUNCHER_3_5",
        "Test_TC_APPLAUNCHER_3_6",
        "Test_TC_APPLAUNCHER_3_7",
        "Test_TC_APPLAUNCHER_3_8",
        "Test_TC_APPLAUNCHER_3_9",
        "Test_TC_MEDIAINPUT_3_10",
        "Test_TC_MEDIAINPUT_3_11",
        "Test_TC_MEDIAINPUT_3_12",
        "Test_TC_MEDIAINPUT_3_13",
        "Test_TC_CHANNEL_5_1",
        "Test_TC_CHANNEL_5_2",
        "Test_TC_CHANNEL_5_3",
        "Test_TC_MEDIAPLAYBACK_6_1",
        "Test_TC_MEDIAPLAYBACK_6_2",
        "Test_TC_MEDIAPLAYBACK_6_3",
        "Test_TC_MEDIAPLAYBACK_6_4",
        "Test_TC_AUDIOOUTPUT_7_1",
        "Test_TC_AUDIOOUTPUT_7_2",
        "Test_TC_TGTNAV_8_1",
        "Test_TC_APBSC_9_1",
        "Test_TC_CONTENTLAUNCHER_10_1",
    ];

    const ModeSelect = [
        "Test_TC_MOD_1_1",
    ];

    const MultipleFabrics = [
    ];

    const OTASoftwareUpdate = [
        "OTA_SuccessfulTransfer",
    ];

    const OnOff = [
        "Test_TC_OO_1_1",
        "Test_TC_OO_2_1",
        "Test_TC_OO_2_2",
        "Test_TC_OO_2_4",
    ];

    const PowerSource = [
        "Test_TC_PS_1_1",
        "Test_TC_PS_2_1",
    ];

    const PressureMeasurement = [
        "Test_TC_PRS_1_1",
        "Test_TC_PRS_2_1",
    ];

    const PumpConfigurationControl = [
        "Test_TC_PCC_1_1",
        "Test_TC_PCC_2_1",
        "Test_TC_PCC_2_2",
        "Test_TC_PCC_2_3",
        "Test_TC_PCC_2_4",
    ];

    const PowerSourceConfiguration = [
        "Test_TC_PSCFG_1_1",
    ];

    const RelativeHumidityMeasurement = [
        "Test_TC_RH_1_1",
        "Test_TC_RH_2_1",
    ];

    const SecureChannel = [
        "Test_TC_SC_4_2",
    ];

    const Switch = [
        "Test_TC_SWTCH_1_1",
        "Test_TC_SWTCH_2_1",
    ];

    const TemperatureMeasurement = [
        "Test_TC_TMP_1_1",
        "Test_TC_TMP_2_1",
    ];

    const Thermostat = [
        "Test_TC_TSTAT_1_1",
        "Test_TC_TSTAT_2_1",
        "Test_TC_TSTAT_2_2",
    ];

    const ThermostatUserConfiguration = [
        "Test_TC_TSUIC_1_1",
        "Test_TC_TSUIC_2_1",
        "Test_TC_TSUIC_2_2",
    ];

    const ThreadNetworkDiagnostics = [
        "Test_TC_DGTHREAD_2_1",
        "Test_TC_DGTHREAD_2_2",
        "Test_TC_DGTHREAD_2_3",
        "Test_TC_DGTHREAD_2_4",
    ];

    const WiFiNetworkDiagnostics = [
        "Test_TC_DGWIFI_2_1",
        "Test_TC_DGWIFI_2_3",
    ];

    const WindowCovering = [
        "Test_TC_WNCV_1_1",
        "Test_TC_WNCV_2_1",
        "Test_TC_WNCV_2_2",
        "Test_TC_WNCV_2_3",
        "Test_TC_WNCV_2_4",
        "Test_TC_WNCV_2_5",
        "Test_TC_WNCV_3_1",
        "Test_TC_WNCV_3_2",
        "Test_TC_WNCV_3_3",
        "Test_TC_WNCV_3_4",
        "Test_TC_WNCV_3_5",
        "Test_TC_WNCV_4_1",
        "Test_TC_WNCV_4_2",
        "Test_TC_WNCV_4_3",
        "Test_TC_WNCV_4_4",
        "Test_TC_WNCV_4_5",
    ];

    const TV = [
        "TV_TargetNavigatorCluster",
        "TV_AudioOutputCluster",
        "TV_ApplicationLauncherCluster",
        "TV_KeypadInputCluster",
        "TV_AccountLoginCluster",
        "TV_WakeOnLanCluster",
        "TV_ApplicationBasicCluster",
        "TV_MediaPlaybackCluster",
        "TV_ChannelCluster",
        "TV_LowPowerCluster",
        "TV_ContentLauncherCluster",
        "TV_MediaInputCluster",
    ];

    const Others = [
        "TestCASERecovery",
        "TestCluster",
        "TestClusterComplexTypes",
        "TestConstraints",
        "TestDelayCommands",
        "TestEvents",
        "TestDiscovery",
        "TestLogCommands",
        "TestSaveAs",
        "TestConfigVariables",
        "TestDescriptorCluster",
        "TestBasicInformation",
        "TestFabricRemovalWhileSubscribed",
        "TestGeneralCommissioning",
        "TestIdentifyCluster",
        "TestOperationalCredentialsCluster",
        "TestModeSelectCluster",
        "TestSelfFabricRemoval",
        "TestSystemCommands",
        "TestBinding",
        "TestUserLabelCluster",
        "TestUserLabelClusterConstraints",
        "TestArmFailSafe",
        "TestFanControl",
        "TestAccessControlConstraints",
        "TestLevelControlWithOnOffDependency",
        "TestCommissioningWindow",
    ];

    const MultiAdmin = [
        "TestMultiAdmin",
    ];

    const SoftwareDiagnostics = [
        "Test_TC_DGSW_2_1",
        "Test_TC_DGSW_2_2",
        "Test_TC_DGSW_2_3",
    ];

    const Subscriptions = [
        "TestSubscribe_OnOff",
    ];

    const DoorLock = [
        "DL_UsersAndCredentials",
        "DL_LockUnlock",
        "DL_Schedules",
        "Test_TC_DRLK_2_2",
        "Test_TC_DRLK_2_3",
        "Test_TC_DRLK_2_4",
        "Test_TC_DRLK_2_5",
        "Test_TC_DRLK_2_7",
        "Test_TC_DRLK_2_9",
    ];

    const Groups = [
        "TestGroupMessaging",
        "TestGroupsCluster",
        "TestGroupKeyManagementCluster",
    ];

    const tests = [
        AccessControl,
        BooleanState,
        Actions,
        ColorControl,
        DeviceManagement,
        Descriptor,
        EthernetNetworkDiagnostics,
        FlowMeasurement,
        GeneralCommissioning,
        GeneralDiagnostics,
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
