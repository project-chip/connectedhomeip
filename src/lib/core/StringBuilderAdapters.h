/*
 *    Copyright (c) 2024 Project CHIP Authors
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
#pragma once

/// This header includes pigweed stringbuilder adaptations for various chip types.
/// You can see https://pigweed.dev/pw_string/guide.html as a reference.
///
/// In particular, pigweed code generally looks like:
///
///    pw::StringBuffer<42> sb;
///    sb << "Here is a value: ";
///    sb << value;
///
/// Where specific formatters exist for "value". In particular these are used when
/// reporting unit test assertions such as ASSERT_EQ/EXPECT_EQ so if you write code
/// like:
///
///    ASSERT_EQ(SomeCall(), CHIP_NO_ERROR);
///
/// On failure without adapters, the objects are reported as "24-byte object at 0x....."
/// which is not as helpful as a full CHIP_ERROR formatted output.
///
/// Example output WITHOUT the adapters
///    Expected: .... == CHIP_ERROR(0, "src/setup_payload/tests/TestAdditionalDataPayload.cpp", 234)
///    Actual: <24-byte object at 0x7ffe39510b80> == <24-byte object at 0x7ffe39510ba0>
///
/// Example output WITH the adapters:
///    Expected: .... == CHIP_ERROR(0, "src/setup_payload/tests/TestAdditionalDataPayload.cpp", 234)
///    Actual: CHIP_ERROR:<src/lib/core/TLVReader.cpp:889: Error 0x00000022> == CHIP_NO_ERROR

#include <pw_string/string_builder.h>

#include <lib/core/CHIPError.h>

namespace pw {

template <>
StatusWithSize ToString<CHIP_ERROR>(const CHIP_ERROR & err, pw::span<char> buffer);

} // namespace pw
