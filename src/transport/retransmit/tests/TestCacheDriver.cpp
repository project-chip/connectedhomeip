/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "TestRetransmit.h"
#include <nlunit-test.h>

int main()
{
    nlTestSetOutputStyle(OUTPUT_CSV);
    return (TestCache());
}
