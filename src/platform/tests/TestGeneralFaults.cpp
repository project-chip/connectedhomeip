/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <platform/GeneralFaults.h>
#include <pw_unit_test/framework.h>

using namespace chip;
using namespace chip::DeviceLayer;

TEST(TestGeneralFaults, TestAddAndRemove)
{
    GeneralFaults<4> faults;

    EXPECT_EQ(faults.size(), 0u);

    // Add faults
    EXPECT_EQ(faults.add(1), CHIP_NO_ERROR);
    EXPECT_EQ(faults.add(2), CHIP_NO_ERROR);
    EXPECT_EQ(faults.add(3), CHIP_NO_ERROR);
    EXPECT_EQ(faults.add(1), CHIP_NO_ERROR);

    EXPECT_EQ(faults.size(), 4u);
    EXPECT_EQ(faults[0], 1);
    EXPECT_EQ(faults[1], 2);
    EXPECT_EQ(faults[2], 3);
    EXPECT_EQ(faults[3], 1);

    // Exceed capacity
    EXPECT_EQ(faults.add(4), CHIP_ERROR_NO_MEMORY);

    // Remove non-existent fault
    EXPECT_EQ(faults.remove(4), CHIP_ERROR_KEY_NOT_FOUND);
    EXPECT_EQ(faults.size(), 4u);

    // Remove an existing fault (which appears twice)
    EXPECT_EQ(faults.remove(1), CHIP_NO_ERROR);
    EXPECT_EQ(faults.size(), 2u);
    EXPECT_EQ(faults[0], 2);
    EXPECT_EQ(faults[1], 3);

    // Remove remaining faults
    EXPECT_EQ(faults.remove(2), CHIP_NO_ERROR);
    EXPECT_EQ(faults.size(), 1u);
    EXPECT_EQ(faults[0], 3);

    EXPECT_EQ(faults.remove(3), CHIP_NO_ERROR);
    EXPECT_EQ(faults.size(), 0u);
}

TEST(TestGeneralFaults, TestIterator)
{
    GeneralFaults<3> faults;
    EXPECT_EQ(faults.add(10), CHIP_NO_ERROR);
    EXPECT_EQ(faults.add(20), CHIP_NO_ERROR);

    auto it = faults.begin();
    EXPECT_TRUE(it != faults.end());
    EXPECT_EQ(*it, 10);
    ++it;
    EXPECT_TRUE(it != faults.end());
    EXPECT_EQ(*it, 20);
    ++it;
    EXPECT_FALSE(it != faults.end());
}
