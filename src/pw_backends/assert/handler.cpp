/*
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
#include <pw_assert_matter/handler.h>

#include <lib/support/CodeUtils.h>
#include <pw_preprocessor/compiler.h>
#include <pw_string/string_builder.h>

void pw_assert_matter_HandleFailure(const char * file_name, int line_number, const char * function_name, const char * format, ...)
{

    pw::StringBuffer<64> builder;

    builder.Format("Assertion failed: %s:%d (in %s):", file_name, line_number, function_name);

    va_list args;
    va_start(args, format);
    builder.FormatVaList(format, args);
    va_end(args);

    ChipLogError(Support, "%s", builder.c_str());
    chipDie();
}

extern "C" void pw_assert_HandleFailure(void)
{
    pw_assert_matter_HandleFailure("<NOFILE>", -1, "<UNKNOWN_FUNCTION>", "Crash: PW_ASSERT() or PW_DASSERT() failure");
}
