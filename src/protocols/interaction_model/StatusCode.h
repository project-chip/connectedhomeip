/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
