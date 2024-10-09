#
#    Copyright (c) 2023 Project CHIP Authors
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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${CHIP_LOCK_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

from drlk_2_x_common import DRLK_COMMON
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main

# Configurable parameters:
# - userIndex: userIndex to use when creating a user on the DUT for testing purposes
#     defaults to 1. Add `--int-arg user_index:<index>` to command line to override
# - CredentialIndex: CredentialIndex to use when creating a Credential on the DUT for testing purposes
#     defaults to 1. Add `--int-arg credential_index:<index>` to command line to override
# - UserCodeTemporaryDisableTime: Value used to configure DUT for testing purposes.
#     defaults to 15 seconds. Add `--int-arg user_code_temporary_disable_time:<secs>` to command line to override
# - WrongCodeEntryLimit: Value used to configure DUT for testing purposes.
#     defaults to 3 attempts. Add `--int-arg wrong_code_entry_limit:<value>` to command line to override
# - AutoRelockTime: Value used to configure DUT for testing purposes.
#     defaults to 60 seconds. Add `--int-arg auto_relock_time:<value>` to command line to override


class TC_DRLK_2_3(MatterBaseTest, DRLK_COMMON):

    @async_test_body
    async def teardown_test(self):
        await self.teardown()
        return super().teardown_test()

    def setup_class(self):
        return super().setup_class()

    def pics_TC_DRLK_2_3(self) -> list[str]:
        return ["DRLK.S"]

    @async_test_body
    async def test_TC_DRLK_2_3(self):
        await self.run_drlk_test_2_3()


if __name__ == "__main__":
    default_matter_test_main()
