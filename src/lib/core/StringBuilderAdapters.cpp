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
#include <lib/core/StringBuilderAdapters.h>

namespace pw {

template <>
StatusWithSize ToString<CHIP_ERROR>(const CHIP_ERROR & err, pw::span<char> buffer)
{
    if (CHIP_ERROR::IsSuccess(err))
    {
        // source location probably does not matter
        return pw::string::Format(buffer, "CHIP_NO_ERROR");
    }
    return pw::string::Format(buffer, "CHIP_ERROR:<%" CHIP_ERROR_FORMAT ">", err.Format());
}

template <>
StatusWithSize ToString<std::chrono::duration<uint64_t, std::milli>>(const std::chrono::duration<uint64_t, std::milli> & time,
                                                                     pw::span<char> buffer)
{
    // Cast to llu because some platforms use lu and others use llu.
    return pw::string::Format(buffer, "%llums", static_cast<long long unsigned int>(time.count()));
}

template <>
StatusWithSize ToString<std::chrono::duration<uint64_t, std::micro>>(const std::chrono::duration<uint64_t, std::micro> & time,
                                                                     pw::span<char> buffer)
{
    // Cast to llu because some platforms use lu and others use llu.
    return pw::string::Format(buffer, "%lluus", static_cast<long long unsigned int>(time.count()));
}

} // namespace pw

#if CHIP_CONFIG_TEST_GOOGLETEST
namespace chip {

void PrintTo(const CHIP_ERROR & err, std::ostream * os)
{
    if (CHIP_ERROR::IsSuccess(err))
    {
        *os << "CHIP_NO_ERROR";
        return;
    }
    *os << "CHIP_ERROR:<" << err.Format() << ">";
}

void PrintTo(const std::chrono::duration<uint64_t, std::milli> & time, std::ostream * os)
{
    *os << time.count() << "ms";
}

void PrintTo(const std::chrono::duration<uint64_t, std::micro> & time, std::ostream * os)
{
    *os << time.count() << "us";
}

} // namespace chip
#endif // CHIP_CONFIG_TEST_GOOGLETEST
