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
#include <app/data-model-provider/StringBuilderAdapters.h>

#include <lib/core/StringBuilderAdapters.h>

namespace pw {

template <>
StatusWithSize ToString<chip::app::DataModel::ActionReturnStatus>(const chip::app::DataModel::ActionReturnStatus & status,
                                                                  pw::span<char> buffer)
{
    chip::app::DataModel::ActionReturnStatus::StringStorage storage;
    return pw::string::Format(buffer, "ActionReturnStatus<%s>", status.c_str(storage));
}

} // namespace pw
//
#if CHIP_CONFIG_TEST_GOOGLETEST
namespace chip {

void PrintTo(const chip::app::DataModel::ActionReturnStatus & status, std::ostream * os)
{
    chip::app::DataModel::ActionReturnStatus::StringStorage storage;
    *os << "ActionReturnStatus<" << status.c_str(storage) << ">";
}

} // namespace chip
#endif // CHIP_CONFIG_TEST_GOOGLETEST
