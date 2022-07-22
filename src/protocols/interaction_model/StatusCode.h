/*
 *    Copyright (c) 2022 Project CHIP Authors
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

#pragma once

#include <stdint.h>

#include <lib/core/CHIPConfig.h>
#include <lib/support/TypeTraits.h>

#if CHIP_CONFIG_IM_STATUS_CODE_VERBOSE_FORMAT
#define ChipLogFormatIMStatus "0x%02x (%s)"
#define ChipLogValueIMStatus(status) chip::to_underlying(status), chip::Protocols::InteractionModel::StatusName(status)
#else // CHIP_CONFIG_IM_STATUS_CODE_VERBOSE_FORMAT
#define ChipLogFormatIMStatus "0x%02x"
#define ChipLogValueIMStatus(status) chip::to_underlying(status)
#endif // CHIP_CONFIG_IM_STATUS_CODE_VERBOSE_FORMAT

namespace chip {
namespace Protocols {
namespace InteractionModel {

// This table comes from the IM's "Status Code Table" section from the Interaction Model spec.
enum class Status : uint8_t
{
#define CHIP_IM_STATUS_CODE(name, spec_name, value) name = value,
#include <protocols/interaction_model/StatusCodeList.h>
#undef CHIP_IM_STATUS_CODE

    InvalidValue = ConstraintError, // Deprecated
};

#if CHIP_CONFIG_IM_STATUS_CODE_VERBOSE_FORMAT
const char * StatusName(Status status);
#endif // CHIP_CONFIG_IM_STATUS_CODE_VERBOSE_FORMAT

} // namespace InteractionModel
} // namespace Protocols
} // namespace chip
