/*
 *
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

#include "ConcreteAttributePath.h"
#include "ConcreteCommandPath.h"
#include "ConcreteEventPath.h"

#include <app/util/privilege-storage.h>

#include <access/Privilege.h>

#include <lib/core/CHIPCore.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace RequiredPrivilege {
inline chip::Access::Privilege ForReadAttribute(const ConcreteAttributePath & path)
{
    return MatterGetAccessPrivilegeForReadAttribute(path.mClusterId, path.mAttributeId);
}

inline chip::Access::Privilege ForWriteAttribute(const ConcreteAttributePath & path)
{
    return MatterGetAccessPrivilegeForWriteAttribute(path.mClusterId, path.mAttributeId);
}

inline chip::Access::Privilege ForInvokeCommand(const ConcreteCommandPath & path)
{
    return MatterGetAccessPrivilegeForInvokeCommand(path.mClusterId, path.mCommandId);
}

inline chip::Access::Privilege ForReadEvent(const ConcreteEventPath & path)
{
    return MatterGetAccessPrivilegeForReadEvent(path.mClusterId, path.mEventId);
}
} // namespace RequiredPrivilege
} // namespace app
} // namespace chip
