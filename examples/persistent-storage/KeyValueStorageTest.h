/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

namespace chip {

enum TestConfigurations
{
    RUN_ALL_TESTS,
    SKIP_MULTI_READ_TEST
};

void RunKvsTest(TestConfigurations test_config = RUN_ALL_TESTS);

} // namespace chip
