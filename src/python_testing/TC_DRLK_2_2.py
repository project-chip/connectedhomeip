#
# SPDX-FileCopyrightText: 2023 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

from drlk_2_x_common import DRLK_COMMON
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main

# Configurable parameters:
# - CredentialIndex: CredentialIndex to use when creating a Credential on the DUT for testing purposes
#     defaults to 1. Add `--int-arg credential_index:<index>` to command line to override
# - UserCodeTemporaryDisableTime: Value used to configure DUT for testing purposes.
#     defaults to 15 seconds. Add `--int-arg user_code_temporary_disable_time:<secs>` to command line to override
# - WrongCodeEntryLimit: Value used to configure DUT for testing purposes.
#     defaults to 3 attempts. Add `--int-arg wrong_code_entry_limit:<value>` to command line to override
# - AutoRelockTime: Value used to configure DUT for testing purposes.
#     defaults to 60 seconds. Add `--int-arg auto_relock_time:<value>` to command line to override


class TC_DRLK_2_2(MatterBaseTest, DRLK_COMMON):
    def setup_class(self):
        return super().setup_class()

    @async_test_body
    async def teardown_test(self):
        await self.teardown()
        return super().teardown_test()

    @async_test_body
    async def test_TC_DRLK_2_2(self):
        await self.run_drlk_test_2_2()


if __name__ == "__main__":
    default_matter_test_main()
