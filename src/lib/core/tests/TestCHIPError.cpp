/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
 *    All rights reserved.
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

#include <string>

#include <pw_unit_test/framework.h>

#include <lib/core/CHIPError.h>

namespace chip {
namespace {

TEST(ChipErrorTest, RangeConstructor)
{
    ChipError error(ChipError::Range::kSDK, /*value=*/1 CHIP_ERROR_SOURCE_LOCATION);
#if CHIP_CONFIG_ERROR_SOURCE
    EXPECT_EQ(error.GetFile(), __FILE__);
    EXPECT_EQ(error.GetLine(), 30u);
#if CHIP_CONFIG_ERROR_STD_SOURCE_LOCATION
    std::source_location location = error.GetSourceLocation();
    EXPECT_EQ(location.line(), 30u);
    EXPECT_EQ(location.file_name(), __FILE__);
#endif // CHIP_CONFIG_ERROR_STD_SOURCE_LOCATION
#endif // CHIP_CONFIG_ERROR_SOURCE
}

TEST(ChipErrorTest, SdkPartConstructor)
{
    ChipError error(ChipError::SdkPart::kCore, /*code=*/1 CHIP_ERROR_SOURCE_LOCATION);
#if CHIP_CONFIG_ERROR_SOURCE
    EXPECT_EQ(error.GetFile(), __FILE__);
    EXPECT_EQ(error.GetLine(), 44u);
#if CHIP_CONFIG_ERROR_STD_SOURCE_LOCATION
    std::source_location location = error.GetSourceLocation();
    EXPECT_EQ(location.line(), 44u);
    EXPECT_EQ(location.file_name(), __FILE__);
#endif // CHIP_CONFIG_ERROR_STD_SOURCE_LOCATION
#endif // CHIP_CONFIG_ERROR_SOURCE
}

TEST(ChipErrorTest, StorageTypeConstructor)
{
    ChipError error(/*error=*/1 CHIP_ERROR_SOURCE_LOCATION);
    EXPECT_EQ(error.AsInteger(), 1u);
#if CHIP_CONFIG_ERROR_SOURCE
    EXPECT_EQ(error.GetFile(), __FILE__);
    EXPECT_EQ(error.GetLine(), 58u);
#if CHIP_CONFIG_ERROR_STD_SOURCE_LOCATION
    std::source_location location = error.GetSourceLocation();
    EXPECT_EQ(location.line(), 58u);
    EXPECT_EQ(location.file_name(), __FILE__);
#endif // CHIP_CONFIG_ERROR_STD_SOURCE_LOCATION
#endif // CHIP_CONFIG_ERROR_SOURCE
}

} // namespace
} // namespace chip
