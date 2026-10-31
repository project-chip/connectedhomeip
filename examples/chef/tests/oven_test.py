#
#    Copyright (c) 2026 Project CHIP Authors
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

import logging

from attributes_service import attributes_service_pb2
from mobly import asserts
from pw_hdlc import rpc
from pw_system.device_connection import create_device_serial_or_socket_connection

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest, TestStep
from matter.testing.runner import default_matter_test_main
from matter.tlv import TLVReader

logger = logging.getLogger(__name__)


class TC_OVEN(MatterBaseTest):
    """Tests for chef oven device (rootnode_oven_temperaturecontrolledcabinet_cooktop_cooksurface_738dd18832)."""

    _OVEN_ENDPOINT = 1
    _CABINET_ENDPOINT = 2
    _COOKTOP_ENDPOINT = 3
    _COOK_SURFACE_ENDPOINT = 4

    _PW_RPC_SOCKET_ADDR = "0.0.0.0:33000"
    _PW_RPC_BAUD_RATE = 115200

    # OvenMode constants (Endpoint 2)
    _MODE_BAKE = 0
    _MODE_CONVECTION = 1
    _MODE_GRILL = 2
    _MODE_ROAST = 3
    _MODE_CLEAN = 4
    _MODE_CONVECTION_BAKE = 5
    _MODE_CONVECTION_ROAST = 6
    _MODE_WARMING = 7
    _MODE_PROOFING = 8
    _INVALID_OVEN_MODE = 99
    _EXPECTED_OVEN_SUPPORTED_MODES = [
        Clusters.Objects.OvenMode.Structs.ModeOptionStruct(
            label="Bake",
            mode=_MODE_BAKE,
            modeTags=[Clusters.Objects.OvenMode.Structs.ModeTagStruct(value=Clusters.Objects.OvenMode.Enums.ModeTag.kBake)],
        ),
        Clusters.Objects.OvenMode.Structs.ModeOptionStruct(
            label="Convection",
            mode=_MODE_CONVECTION,
            modeTags=[Clusters.Objects.OvenMode.Structs.ModeTagStruct(value=Clusters.Objects.OvenMode.Enums.ModeTag.kConvection)],
        ),
        Clusters.Objects.OvenMode.Structs.ModeOptionStruct(
            label="Grill",
            mode=_MODE_GRILL,
            modeTags=[Clusters.Objects.OvenMode.Structs.ModeTagStruct(value=Clusters.Objects.OvenMode.Enums.ModeTag.kGrill)],
        ),
        Clusters.Objects.OvenMode.Structs.ModeOptionStruct(
            label="Roast",
            mode=_MODE_ROAST,
            modeTags=[Clusters.Objects.OvenMode.Structs.ModeTagStruct(value=Clusters.Objects.OvenMode.Enums.ModeTag.kRoast)],
        ),
        Clusters.Objects.OvenMode.Structs.ModeOptionStruct(
            label="Clean",
            mode=_MODE_CLEAN,
            modeTags=[Clusters.Objects.OvenMode.Structs.ModeTagStruct(value=Clusters.Objects.OvenMode.Enums.ModeTag.kClean)],
        ),
        Clusters.Objects.OvenMode.Structs.ModeOptionStruct(
            label="Convection Bake",
            mode=_MODE_CONVECTION_BAKE,
            modeTags=[
                Clusters.Objects.OvenMode.Structs.ModeTagStruct(value=Clusters.Objects.OvenMode.Enums.ModeTag.kConvectionBake)
            ],
        ),
        Clusters.Objects.OvenMode.Structs.ModeOptionStruct(
            label="Convection Roast",
            mode=_MODE_CONVECTION_ROAST,
            modeTags=[
                Clusters.Objects.OvenMode.Structs.ModeTagStruct(value=Clusters.Objects.OvenMode.Enums.ModeTag.kConvectionRoast)
            ],
        ),
        Clusters.Objects.OvenMode.Structs.ModeOptionStruct(
            label="Warming",
            mode=_MODE_WARMING,
            modeTags=[Clusters.Objects.OvenMode.Structs.ModeTagStruct(value=Clusters.Objects.OvenMode.Enums.ModeTag.kWarming)],
        ),
        Clusters.Objects.OvenMode.Structs.ModeOptionStruct(
            label="Proofing",
            mode=_MODE_PROOFING,
            modeTags=[Clusters.Objects.OvenMode.Structs.ModeTagStruct(value=Clusters.Objects.OvenMode.Enums.ModeTag.kProofing)],
        ),
    ]

    # OvenCavityOperationalState constants (Endpoint 2)
    _PHASE_PRE_HEATING = 0
    _EXPECTED_PHASES = ["pre-heating", "pre-heated", "cooling down"]

    # TemperatureControl constants on Cabinet (Endpoint 2, hundredths of a degree C)
    _CABINET_TEMP_CONTROL_FEATURE_MAP = Clusters.Objects.TemperatureControl.Bitmaps.Feature.kTemperatureNumber
    _CABINET_MIN_TEMP = 7600
    _CABINET_MAX_TEMP = 28800
    _CABINET_DEFAULT_SETPOINT = 7600
    _CABINET_TARGET_SETPOINT = 18000
    _CABINET_INVALID_SETPOINT = 30000

    # TemperatureMeasurement constants on Cabinet (Endpoint 2, hundredths of a degree C)
    _CABINET_MIN_MEASURED_TEMP = 3000
    _CABINET_MAX_MEASURED_TEMP = 30000

    # OnOff constants on Cooktop (Endpoint 3) and CookSurface (Endpoint 4)
    _ON_OFF_OFF_ONLY_FEATURE_MAP = Clusters.Objects.OnOff.Bitmaps.Feature.kOffOnly

    # TemperatureControl constants on CookSurface (Endpoint 4)
    _COOK_SURFACE_TEMP_CONTROL_FEATURE_MAP = Clusters.Objects.TemperatureControl.Bitmaps.Feature.kTemperatureLevel
    _COOK_SURFACE_SUPPORTED_TEMP_LEVELS = ["Low", "Medium", "High"]

    # Semantic tags
    _CABINET_TAG_LIST = [
        Clusters.Objects.Globals.Structs.SemanticTagStruct(
            mfgCode=NullValue, namespaceID=0x08, tag=0x02, label="top"
        )
    ]
    _COOK_SURFACE_TAG_LIST = [
        Clusters.Objects.Globals.Structs.SemanticTagStruct(
            mfgCode=NullValue, namespaceID=0x08, tag=0x00, label="left"
        )
    ]

    def desc_TC_OVEN(self) -> str:
        return "[TC_OVEN] chef oven multi-endpoint functionality test."

    def steps_TC_OVEN(self):
        return [
            TestStep(1, "Commissioning already done.", is_commissioning=True),
            TestStep(2, "Verify endpoint composition and semantic tags across all endpoints."),
            TestStep(3, "Test Identify cluster on Oven (EP 1) and Cooktop (EP 3)."),
            TestStep(4, "Test OvenMode cluster on Temperature Controlled Cabinet (EP 2)."),
            TestStep(5, "Test OvenCavityOperationalState cluster on Temperature Controlled Cabinet (EP 2)."),
            TestStep(6, "Test TemperatureControl cluster on Cabinet (EP 2) and CookSurface (EP 4)."),
            TestStep(7, "Test TemperatureMeasurement cluster on Cabinet (EP 2) and CookSurface (EP 4)."),
            TestStep(8, "Test Off-only OnOff cluster on Cooktop (EP 3) and CookSurface (EP 4)."),
        ]

    def _init_pw_rpc(self):
        self._pw_rpc_connection = create_device_serial_or_socket_connection(
            device="",
            baudrate=self._PW_RPC_BAUD_RATE,
            token_databases=[],
            socket_addr=self._PW_RPC_SOCKET_ADDR,
            compiled_protos=[attributes_service_pb2],
            rpc_logging=True,
            channel_id=rpc.DEFAULT_CHANNEL_ID,
            hdlc_encoding=True,
            device_tracing=False,
        )
        self._pw_rpc_device = self._pw_rpc_connection.__enter__()

    def teardown_test(self):
        if getattr(self, "_pw_rpc_connection", None) is not None:
            self._pw_rpc_connection.__exit__(None, None, None)
            self._pw_rpc_connection = None
            self._pw_rpc_device = None
        super().teardown_test()

    # -------------------------------------------------------------------------
    # Matter IM Helper Methods
    # -------------------------------------------------------------------------

    async def _read_parts_list(self, endpoint: int):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=Clusters.Objects.Descriptor,
            attribute=Clusters.Objects.Descriptor.Attributes.PartsList,
        )

    async def _read_tag_list(self, endpoint: int):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=Clusters.Objects.Descriptor,
            attribute=Clusters.Objects.Descriptor.Attributes.TagList,
        )

    async def _read_identify_time(self, endpoint: int):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=Clusters.Objects.Identify,
            attribute=Clusters.Objects.Identify.Attributes.IdentifyTime,
        )

    async def _read_on_off(self, endpoint: int):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=Clusters.Objects.OnOff,
            attribute=Clusters.Objects.OnOff.Attributes.OnOff,
        )

    async def _read_temperature_measured_value(self, endpoint: int):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=Clusters.Objects.TemperatureMeasurement,
            attribute=Clusters.Objects.TemperatureMeasurement.Attributes.MeasuredValue,
        )

    # -------------------------------------------------------------------------
    # PwRPC Helper Methods
    # -------------------------------------------------------------------------

    def _read_on_off_pwrpc(self, endpoint: int) -> bool:
        result = self._pw_rpc_device.rpcs.chip.rpc.Attributes.Read(
            endpoint=endpoint,
            cluster=Clusters.Objects.OnOff.id,
            attribute_id=Clusters.Objects.OnOff.Attributes.OnOff.attribute_id,
            type=attributes_service_pb2.AttributeType.ZCL_BOOLEAN_ATTRIBUTE_TYPE,
        )
        asserts.assert_true(result.status.ok(), msg="PwRPC OnOff Read status not ok.")
        return result.response.data_bool

    def _write_on_off_pwrpc(self, endpoint: int, on_off: bool):
        result = self._pw_rpc_device.rpcs.chip.rpc.Attributes.Write(
            data=attributes_service_pb2.AttributeData(data_bool=on_off),
            metadata=attributes_service_pb2.AttributeMetadata(
                endpoint=endpoint,
                cluster=Clusters.Objects.OnOff.id,
                attribute_id=Clusters.Objects.OnOff.Attributes.OnOff.attribute_id,
                type=attributes_service_pb2.AttributeType.ZCL_BOOLEAN_ATTRIBUTE_TYPE,
            ),
        )
        asserts.assert_true(result.status.ok(), msg="PwRPC OnOff Write status not ok.")

    def _read_temperature_measured_value_pwrpc(self, endpoint: int) -> int:
        result = self._pw_rpc_device.rpcs.chip.rpc.Attributes.Read(
            endpoint=endpoint,
            cluster=Clusters.Objects.TemperatureMeasurement.id,
            attribute_id=Clusters.Objects.TemperatureMeasurement.Attributes.MeasuredValue.attribute_id,
            type=attributes_service_pb2.AttributeType.ZCL_INT16S_ATTRIBUTE_TYPE,
        )
        asserts.assert_true(result.status.ok(), msg="PwRPC TemperatureMeasurement Read status not ok.")
        return result.response.data_int16

    def _write_temperature_measured_value_pwrpc(self, endpoint: int, value: int):
        result = self._pw_rpc_device.rpcs.chip.rpc.Attributes.Write(
            data=attributes_service_pb2.AttributeData(data_int16=value),
            metadata=attributes_service_pb2.AttributeMetadata(
                endpoint=endpoint,
                cluster=Clusters.Objects.TemperatureMeasurement.id,
                attribute_id=Clusters.Objects.TemperatureMeasurement.Attributes.MeasuredValue.attribute_id,
                type=attributes_service_pb2.AttributeType.ZCL_INT16S_ATTRIBUTE_TYPE,
            ),
        )
        asserts.assert_true(result.status.ok(), msg="PwRPC TemperatureMeasurement Write status not ok.")

    def _read_oven_supported_modes_pwrpc(self):
        result = self._pw_rpc_device.rpcs.chip.rpc.Attributes.Read(
            endpoint=self._CABINET_ENDPOINT,
            cluster=Clusters.Objects.OvenMode.id,
            attribute_id=Clusters.Objects.OvenMode.Attributes.SupportedModes.attribute_id,
            type=attributes_service_pb2.AttributeType.ZCL_ARRAY_ATTRIBUTE_TYPE,
        )
        asserts.assert_true(result.status.ok(), msg="PwRPC OvenMode SupportedModes Read status not ok.")
        asserts.assert_greater(
            len(result.response.tlv_data),
            256,
            "Expected OvenMode.SupportedModes TLV payload to exceed 256 bytes.",
        )
        # PwRPC Attributes.Read encodes an outer struct {1: [AttributeReportIB({1: AttributeDataIB({2: Data})})]}.
        raw_modes_tlv = TLVReader(result.response.tlv_data).get()["Any"][1][0][1][2]
        return Clusters.Objects.OvenMode.Attributes.SupportedModes.FromTagDictOrRawValue(raw_modes_tlv)

    # -------------------------------------------------------------------------
    # Sub-test Methods
    # -------------------------------------------------------------------------

    async def _test_endpoint_composition_and_tags(self):
        # Oven (EP 1) uses tree composition with direct children Cabinet (EP 2) and Cooktop (EP 3).
        oven_parts = await self._read_parts_list(self._OVEN_ENDPOINT)
        asserts.assert_equal(
            sorted(oven_parts),
            [self._CABINET_ENDPOINT, self._COOKTOP_ENDPOINT],
            "Oven (EP 1) PartsList should contain direct child endpoints 2 and 3.",
        )

        # Cooktop (EP 3) has CookSurface (EP 4) as its child endpoint.
        cooktop_parts = await self._read_parts_list(self._COOKTOP_ENDPOINT)
        asserts.assert_equal(
            cooktop_parts,
            [self._COOK_SURFACE_ENDPOINT],
            "Cooktop (EP 3) PartsList should contain endpoint 4.",
        )

        # Verify semantic tags on Cabinet (EP 2 -> "top") and CookSurface (EP 4 -> "left").
        cabinet_tags = await self._read_tag_list(self._CABINET_ENDPOINT)
        asserts.assert_equal(
            cabinet_tags,
            self._CABINET_TAG_LIST,
            "Cabinet (EP 2) TagList should match top position tag.",
        )

        cook_surface_tags = await self._read_tag_list(self._COOK_SURFACE_ENDPOINT)
        asserts.assert_equal(
            cook_surface_tags,
            self._COOK_SURFACE_TAG_LIST,
            "CookSurface (EP 4) TagList should match left position tag.",
        )

    async def _test_identify_on_endpoint(self, endpoint: int):
        asserts.assert_equal(await self._read_identify_time(endpoint), 0)
        await self.send_single_cmd(
            cmd=Clusters.Objects.Identify.Commands.Identify(identifyTime=5),
            endpoint=endpoint,
        )
        identify_time = await self._read_identify_time(endpoint)
        asserts.assert_greater(identify_time, 0)
        asserts.assert_less_equal(identify_time, 5)

    async def _test_oven_mode(self):
        cluster = Clusters.Objects.OvenMode
        attributes = cluster.Attributes

        supported_modes = await self.read_single_attribute_check_success(
            endpoint=self._CABINET_ENDPOINT,
            cluster=cluster,
            attribute=attributes.SupportedModes,
        )
        asserts.assert_equal(supported_modes, self._EXPECTED_OVEN_SUPPORTED_MODES, "OvenMode SupportedModes mismatch.")

        # Verify OvenMode.SupportedModes read and decoded via PwRPC (>256 bytes TLV payload).
        pwrpc_supported_modes = self._read_oven_supported_modes_pwrpc()
        asserts.assert_equal(
            pwrpc_supported_modes,
            self._EXPECTED_OVEN_SUPPORTED_MODES,
            "PwRPC OvenMode SupportedModes mismatch.",
        )

        # Test switching across valid modes.
        for target_mode in [self._MODE_CONVECTION, self._MODE_GRILL, self._MODE_BAKE]:
            response = await self.send_single_cmd(
                cmd=cluster.Commands.ChangeToMode(newMode=target_mode),
                endpoint=self._CABINET_ENDPOINT,
            )
            asserts.assert_equal(response.status, Status.Success, "ChangeToMode should succeed for valid mode.")

            current_mode = await self.read_single_attribute_check_success(
                endpoint=self._CABINET_ENDPOINT,
                cluster=cluster,
                attribute=attributes.CurrentMode,
            )
            asserts.assert_equal(current_mode, target_mode, "OvenMode CurrentMode should match target mode.")

        # Test switching to an invalid mode returns non-Success status and preserves CurrentMode.
        response = await self.send_single_cmd(
            cmd=cluster.Commands.ChangeToMode(newMode=self._INVALID_OVEN_MODE),
            endpoint=self._CABINET_ENDPOINT,
        )
        asserts.assert_not_equal(response.status, Status.Success, "ChangeToMode should fail for unsupported mode.")

        current_mode = await self.read_single_attribute_check_success(
            endpoint=self._CABINET_ENDPOINT,
            cluster=cluster,
            attribute=attributes.CurrentMode,
        )
        asserts.assert_equal(current_mode, self._MODE_BAKE, "CurrentMode should remain unchanged after invalid ChangeToMode.")

    async def _test_oven_cavity_operational_state(self):
        cluster = Clusters.Objects.OvenCavityOperationalState
        attributes = cluster.Attributes
        op_state_enum = cluster.Enums.OperationalStateEnum
        error_enum = cluster.Enums.ErrorStateEnum

        # Verify PhaseList and OperationalStateList.
        phase_list = await self.read_single_attribute_check_success(
            endpoint=self._CABINET_ENDPOINT,
            cluster=cluster,
            attribute=attributes.PhaseList,
        )
        asserts.assert_equal(phase_list, self._EXPECTED_PHASES, "PhaseList should match expected phases.")

        op_state_list = await self.read_single_attribute_check_success(
            endpoint=self._CABINET_ENDPOINT,
            cluster=cluster,
            attribute=attributes.OperationalStateList,
        )
        expected_state_ids = [op_state_enum.kStopped, op_state_enum.kRunning, op_state_enum.kError]
        asserts.assert_equal(
            [s.operationalStateID for s in op_state_list],
            expected_state_ids,
            "OperationalStateList should contain Stopped, Running, and Error.",
        )

        # Ensure cycle is initially stopped.
        op_state = await self.read_single_attribute_check_success(
            endpoint=self._CABINET_ENDPOINT,
            cluster=cluster,
            attribute=attributes.OperationalState,
        )
        if op_state != op_state_enum.kStopped:
            await self.send_single_cmd(cmd=cluster.Commands.Stop(), endpoint=self._CABINET_ENDPOINT)

        asserts.assert_equal(
            await self.read_single_attribute_check_success(
                endpoint=self._CABINET_ENDPOINT, cluster=cluster, attribute=attributes.OperationalState
            ),
            op_state_enum.kStopped,
        )
        asserts.assert_equal(
            await self.read_single_attribute_check_success(
                endpoint=self._CABINET_ENDPOINT, cluster=cluster, attribute=attributes.CurrentPhase
            ),
            NullValue,
        )

        # Start heating cycle.
        start_resp = await self.send_single_cmd(
            cmd=cluster.Commands.Start(),
            endpoint=self._CABINET_ENDPOINT,
        )
        asserts.assert_equal(start_resp.commandResponseState.errorStateID, error_enum.kNoError)

        asserts.assert_equal(
            await self.read_single_attribute_check_success(
                endpoint=self._CABINET_ENDPOINT, cluster=cluster, attribute=attributes.OperationalState
            ),
            op_state_enum.kRunning,
            "OperationalState should be Running after Start.",
        )
        asserts.assert_equal(
            await self.read_single_attribute_check_success(
                endpoint=self._CABINET_ENDPOINT, cluster=cluster, attribute=attributes.CurrentPhase
            ),
            self._PHASE_PRE_HEATING,
            "CurrentPhase should start at pre-heating (0).",
        )

        # Stop heating cycle and verify cleanup.
        stop_resp = await self.send_single_cmd(
            cmd=cluster.Commands.Stop(),
            endpoint=self._CABINET_ENDPOINT,
        )
        asserts.assert_equal(stop_resp.commandResponseState.errorStateID, error_enum.kNoError)

        asserts.assert_equal(
            await self.read_single_attribute_check_success(
                endpoint=self._CABINET_ENDPOINT, cluster=cluster, attribute=attributes.OperationalState
            ),
            op_state_enum.kStopped,
            "OperationalState should be Stopped after Stop.",
        )
        asserts.assert_equal(
            await self.read_single_attribute_check_success(
                endpoint=self._CABINET_ENDPOINT, cluster=cluster, attribute=attributes.CurrentPhase
            ),
            NullValue,
            "CurrentPhase should reset to NullValue after Stop.",
        )

    async def _test_cabinet_temperature_control(self):
        cluster = Clusters.Objects.TemperatureControl
        attributes = cluster.Attributes

        feature_map = await self.read_single_attribute_check_success(
            endpoint=self._CABINET_ENDPOINT, cluster=cluster, attribute=attributes.FeatureMap
        )
        asserts.assert_equal(feature_map, self._CABINET_TEMP_CONTROL_FEATURE_MAP)

        min_temp = await self.read_single_attribute_check_success(
            endpoint=self._CABINET_ENDPOINT, cluster=cluster, attribute=attributes.MinTemperature
        )
        max_temp = await self.read_single_attribute_check_success(
            endpoint=self._CABINET_ENDPOINT, cluster=cluster, attribute=attributes.MaxTemperature
        )
        asserts.assert_equal(min_temp, self._CABINET_MIN_TEMP)
        asserts.assert_equal(max_temp, self._CABINET_MAX_TEMP)

        initial_setpoint = await self.read_single_attribute_check_success(
            endpoint=self._CABINET_ENDPOINT, cluster=cluster, attribute=attributes.TemperatureSetpoint
        )
        asserts.assert_equal(initial_setpoint, self._CABINET_DEFAULT_SETPOINT)

        # Set a valid temperature setpoint.
        await self.send_single_cmd(
            cmd=cluster.Commands.SetTemperature(targetTemperature=self._CABINET_TARGET_SETPOINT),
            endpoint=self._CABINET_ENDPOINT,
        )
        current_setpoint = await self.read_single_attribute_check_success(
            endpoint=self._CABINET_ENDPOINT, cluster=cluster, attribute=attributes.TemperatureSetpoint
        )
        asserts.assert_equal(current_setpoint, self._CABINET_TARGET_SETPOINT)

        # Out-of-range temperature setpoint should fail with ConstraintError.
        try:
            await self.send_single_cmd(
                cmd=cluster.Commands.SetTemperature(targetTemperature=self._CABINET_INVALID_SETPOINT),
                endpoint=self._CABINET_ENDPOINT,
            )
            asserts.fail("Expected ConstraintError for out-of-range targetTemperature.")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError)

        # Verify setpoint remained unchanged.
        current_setpoint = await self.read_single_attribute_check_success(
            endpoint=self._CABINET_ENDPOINT, cluster=cluster, attribute=attributes.TemperatureSetpoint
        )
        asserts.assert_equal(current_setpoint, self._CABINET_TARGET_SETPOINT)

    async def _test_cook_surface_temperature_control(self):
        cluster = Clusters.Objects.TemperatureControl
        attributes = cluster.Attributes

        feature_map = await self.read_single_attribute_check_success(
            endpoint=self._COOK_SURFACE_ENDPOINT, cluster=cluster, attribute=attributes.FeatureMap
        )
        asserts.assert_equal(feature_map, self._COOK_SURFACE_TEMP_CONTROL_FEATURE_MAP)

        supported_levels = await self.read_single_attribute_check_success(
            endpoint=self._COOK_SURFACE_ENDPOINT, cluster=cluster, attribute=attributes.SupportedTemperatureLevels
        )
        asserts.assert_equal(supported_levels, self._COOK_SURFACE_SUPPORTED_TEMP_LEVELS)

        # Cycle through valid temperature levels.
        for level_idx in [1, 2, 0]:
            await self.send_single_cmd(
                cmd=cluster.Commands.SetTemperature(targetTemperatureLevel=level_idx),
                endpoint=self._COOK_SURFACE_ENDPOINT,
            )
            selected_level = await self.read_single_attribute_check_success(
                endpoint=self._COOK_SURFACE_ENDPOINT, cluster=cluster, attribute=attributes.SelectedTemperatureLevel
            )
            asserts.assert_equal(selected_level, level_idx)

        # Out-of-range temperature level should fail with ConstraintError.
        invalid_level = len(supported_levels) + 1
        try:
            await self.send_single_cmd(
                cmd=cluster.Commands.SetTemperature(targetTemperatureLevel=invalid_level),
                endpoint=self._COOK_SURFACE_ENDPOINT,
            )
            asserts.fail("Expected ConstraintError for out-of-range targetTemperatureLevel.")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError)

    async def _test_off_only_on_off(self, endpoint: int):
        cluster = Clusters.Objects.OnOff
        attributes = cluster.Attributes

        feature_map = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attributes.FeatureMap
        )
        asserts.assert_equal(feature_map, self._ON_OFF_OFF_ONLY_FEATURE_MAP)

        # Turn on the endpoint via PwRPC (since OffOnly endpoints only accept the Off command over Matter).
        self._write_on_off_pwrpc(endpoint, True)
        asserts.assert_true(self._read_on_off_pwrpc(endpoint), msg="PwRPC OnOff should be True after write.")
        asserts.assert_true(await self._read_on_off(endpoint), msg="Matter OnOff should be True after PwRPC write.")

        # Send Off command over Matter IM and verify it turns off.
        await self.send_single_cmd(
            cmd=cluster.Commands.Off(),
            endpoint=endpoint,
        )
        asserts.assert_false(await self._read_on_off(endpoint), msg="Matter OnOff should be False after Off command.")
        asserts.assert_false(self._read_on_off_pwrpc(endpoint), msg="PwRPC OnOff should be False after Off command.")

    async def _test_temperature_measurement_on_endpoint(self, endpoint: int, test_values: list[int]):
        for value in test_values:
            self._write_temperature_measured_value_pwrpc(endpoint, value)
            pwrpc_val = self._read_temperature_measured_value_pwrpc(endpoint)
            matter_val = await self._read_temperature_measured_value(endpoint)
            asserts.assert_equal(pwrpc_val, value)
            asserts.assert_equal(matter_val, value)

    async def _test_temperature_measurement(self):
        min_measured = await self.read_single_attribute_check_success(
            endpoint=self._CABINET_ENDPOINT,
            cluster=Clusters.Objects.TemperatureMeasurement,
            attribute=Clusters.Objects.TemperatureMeasurement.Attributes.MinMeasuredValue,
        )
        max_measured = await self.read_single_attribute_check_success(
            endpoint=self._CABINET_ENDPOINT,
            cluster=Clusters.Objects.TemperatureMeasurement,
            attribute=Clusters.Objects.TemperatureMeasurement.Attributes.MaxMeasuredValue,
        )
        asserts.assert_equal(min_measured, self._CABINET_MIN_MEASURED_TEMP)
        asserts.assert_equal(max_measured, self._CABINET_MAX_MEASURED_TEMP)

        await self._test_temperature_measurement_on_endpoint(self._CABINET_ENDPOINT, [9000, 18000, 22000])
        await self._test_temperature_measurement_on_endpoint(self._COOK_SURFACE_ENDPOINT, [5000, 12000])

    @async_test_body
    async def test_TC_OVEN(self):
        """Run all steps."""
        self._init_pw_rpc()

        self.step(1)
        # Commissioning already done.

        self.step(2)
        await self._test_endpoint_composition_and_tags()

        self.step(3)
        await self._test_identify_on_endpoint(self._OVEN_ENDPOINT)
        await self._test_identify_on_endpoint(self._COOKTOP_ENDPOINT)

        self.step(4)
        await self._test_oven_mode()

        self.step(5)
        await self._test_oven_cavity_operational_state()

        self.step(6)
        await self._test_cabinet_temperature_control()
        await self._test_cook_surface_temperature_control()

        self.step(7)
        await self._test_temperature_measurement()

        self.step(8)
        await self._test_off_only_on_off(self._COOKTOP_ENDPOINT)
        await self._test_off_only_on_off(self._COOK_SURFACE_ENDPOINT)


if __name__ == "__main__":
    default_matter_test_main()
