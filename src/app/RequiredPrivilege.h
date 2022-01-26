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

#include <access/Privilege.h>

#include <lib/core/CHIPCore.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {

// This functionality is intended to come from Ember, but until Ember supports it,
// this class will provide a workable alternative.
class RequiredPrivilege
{
    using Privilege = Access::Privilege;

public:
    static Privilege ForReadAttribute(const ConcreteAttributePath & path);
    static Privilege ForWriteAttribute(const ConcreteAttributePath & path);
    static Privilege ForInvokeCommand(const ConcreteCommandPath & path);
    static Privilege ForReadEvent(const ConcreteEventPath & path);
};

} // namespace app
} // namespace chip
