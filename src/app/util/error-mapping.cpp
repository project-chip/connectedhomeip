/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "error-mapping.h"

namespace chip {
namespace app {

Protocols::InteractionModel::Status ToInteractionModelStatus(EmberAfStatus code)
{
    using imcode = Protocols::InteractionModel::Status;
    if (code == EMBER_ZCL_STATUS_DUPLICATE_EXISTS /* 0x8A */)
    {
        // For now, this is still used, and should be mapped to success.  Once
        // we update bindings to no longer use it, this case can go away.
        return imcode::Success;
    }

    return static_cast<imcode>(code);
}

} // namespace app
} // namespace chip
