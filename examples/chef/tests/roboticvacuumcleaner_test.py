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

from mobly import asserts

import matter.clusters as Clusters
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest, TestStep
from matter.testing.runner import default_matter_test_main

logger = logging.getLogger(__name__)


class TC_ROBOTICVACUUMCLEANER(MatterBaseTest):
    """Tests for chef robotic vacuum cleaner device."""

    ENDPOINT = 1

    _MODE_IDLE = 0
    _MODE_CLEANING = 1

    _OP_STATE_STOPPED = Clusters.Objects.RvcOperationalState.Enums.OperationalStateEnum.kStopped
    _OP_STATE_RUNNING = Clusters.Objects.RvcOperationalState.Enums.OperationalStateEnum.kRunning
    _OP_STATE_PAUSED = Clusters.Objects.RvcOperationalState.Enums.OperationalStateEnum.kPaused
    _OP_STATE_CHARGING = Clusters.Objects.RvcOperationalState.Enums.OperationalStateEnum.kCharging
    _OP_STATE_RUNNING_WHILE_BEEPING = 0x81
    _RUNNING_STATES = [_OP_STATE_RUNNING, _OP_STATE_RUNNING_WHILE_BEEPING]

    def desc_TC_ROBOTICVACUUMCLEANER(self) -> str:
        return "[TC_ROBOTICVACUUMCLEANER] chef robotic vacuum cleaner functionality test."

    def steps_TC_ROBOTICVACUUMCLEANER(self):
        return [
            TestStep(1, "Commissioning already done.", is_commissioning=True),
            TestStep(2, "Test RVC start and stop cycle."),
            TestStep(3, "Test RVC pause command during a running cycle."),
            TestStep(4, "Test RVC resume command from a paused cycle."),
            TestStep(5, "Test RVC GoHome command during a running cycle."),
        ]

    async def _read_operational_state(self):
        return await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT,
            cluster=Clusters.Objects.RvcOperationalState,
            attribute=Clusters.Objects.RvcOperationalState.Attributes.OperationalState,
        )

    async def _read_current_run_mode(self):
        return await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT,
            cluster=Clusters.Objects.RvcRunMode,
            attribute=Clusters.Objects.RvcRunMode.Attributes.CurrentMode,
        )

    async def _change_run_mode(self, new_mode: int):
        return await self.send_single_cmd(
            cmd=Clusters.Objects.RvcRunMode.Commands.ChangeToMode(newMode=new_mode),
            endpoint=self.ENDPOINT,
        )

    async def _send_pause(self):
        return await self.send_single_cmd(
            cmd=Clusters.Objects.RvcOperationalState.Commands.Pause(),
            endpoint=self.ENDPOINT,
        )

    async def _send_resume(self):
        return await self.send_single_cmd(
            cmd=Clusters.Objects.RvcOperationalState.Commands.Resume(),
            endpoint=self.ENDPOINT,
        )

    async def _send_go_home(self):
        return await self.send_single_cmd(
            cmd=Clusters.Objects.RvcOperationalState.Commands.GoHome(),
            endpoint=self.ENDPOINT,
        )

    async def _test_rvc_cycle(self):
        await self._change_run_mode(self._MODE_CLEANING)

        operational_state = await self._read_operational_state()
        asserts.assert_in(
            operational_state,
            self._RUNNING_STATES,
            "Expected RVC operational state to be running (1 or 129).",
        )

        await self._change_run_mode(self._MODE_IDLE)

        operational_state = await self._read_operational_state()
        asserts.assert_equal(
            operational_state,
            self._OP_STATE_STOPPED,
            "Expected RVC operational state to be stopped (0).",
        )

    async def _test_rvc_pause(self):
        await self._change_run_mode(self._MODE_IDLE)
        await self._change_run_mode(self._MODE_CLEANING)

        operational_state = await self._read_operational_state()
        asserts.assert_in(
            operational_state,
            self._RUNNING_STATES,
            "Expected RVC operational state to be running (1 or 129).",
        )

        await self._send_pause()

        operational_state = await self._read_operational_state()
        asserts.assert_equal(
            operational_state,
            self._OP_STATE_PAUSED,
            "Expected RVC operational state to be paused (2).",
        )

        await self._change_run_mode(self._MODE_IDLE)

        operational_state = await self._read_operational_state()
        asserts.assert_equal(
            operational_state,
            self._OP_STATE_STOPPED,
            "Expected RVC operational state to be stopped (0).",
        )

    async def _test_rvc_resume(self):
        await self._change_run_mode(self._MODE_IDLE)
        await self._change_run_mode(self._MODE_CLEANING)
        await self._send_pause()

        operational_state = await self._read_operational_state()
        asserts.assert_equal(
            operational_state,
            self._OP_STATE_PAUSED,
            "Expected RVC operational state to be paused (2).",
        )

        await self._send_resume()

        operational_state = await self._read_operational_state()
        asserts.assert_in(
            operational_state,
            self._RUNNING_STATES,
            "Expected RVC operational state to be running (1 or 129) after resume.",
        )

        await self._change_run_mode(self._MODE_IDLE)

        operational_state = await self._read_operational_state()
        asserts.assert_equal(
            operational_state,
            self._OP_STATE_STOPPED,
            "Expected RVC operational state to be stopped (0).",
        )

    async def _test_rvc_go_home(self):
        await self._change_run_mode(self._MODE_IDLE)
        await self._change_run_mode(self._MODE_CLEANING)

        operational_state = await self._read_operational_state()
        asserts.assert_in(
            operational_state,
            self._RUNNING_STATES,
            "Expected RVC operational state to be running (1 or 129).",
        )

        await self._send_go_home()

        current_mode = await self._read_current_run_mode()
        asserts.assert_equal(
            current_mode,
            self._MODE_IDLE,
            "Expected RVC run mode to be idle (0) after GoHome.",
        )

        operational_state = await self._read_operational_state()
        asserts.assert_equal(
            operational_state,
            self._OP_STATE_CHARGING,
            "Expected RVC operational state to be charging (65) after GoHome.",
        )

    @async_test_body
    async def test_TC_ROBOTICVACUUMCLEANER(self):
        """Run all steps."""

        self.step(1)
        # Commissioning already done.

        self.step(2)
        await self._test_rvc_cycle()

        self.step(3)
        await self._test_rvc_pause()

        self.step(4)
        await self._test_rvc_resume()

        self.step(5)
        await self._test_rvc_go_home()


if __name__ == "__main__":
    default_matter_test_main()
