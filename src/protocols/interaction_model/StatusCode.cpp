/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <lib/core/CHIPConfig.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace Protocols {
namespace InteractionModel {

#if CHIP_CONFIG_IM_STATUS_CODE_VERBOSE_FORMAT
const char * StatusName(Status status)
{
    switch (status)
    {
#define CHIP_IM_STATUS_CODE(name, spec_name, value)                                                                                \
    case Status(value):                                                                                                            \
        return #spec_name;
#include <protocols/interaction_model/StatusCodeList.h>
#undef CHIP_IM_STATUS_CODE
    }

    return "Unallocated";
}
#endif // CHIP_CONFIG_IM_STATUS_CODE_VERBOSE_FORMAT

} // namespace InteractionModel
} // namespace Protocols
} // namespace chip
