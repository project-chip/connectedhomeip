/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
