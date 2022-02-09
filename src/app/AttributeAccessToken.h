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

#include <access/Privilege.h>
#include <app/ConcreteAttributePath.h>

namespace chip {
namespace app {

/**
 * AttributeAccessToken records the privilege granted for accessing the specified attribute. This struct is used in chunked write
 * to avoid losing privilege when updating ACL items.
 */
struct AttributeAccessToken
{
    ConcreteAttributePath mPath;
    Access::Privilege mPrivilege;

    bool operator==(const AttributeAccessToken & other) const { return mPath == other.mPath && mPrivilege == other.mPrivilege; }

    bool Matches(const ConcreteAttributePath & aPath, const Access::Privilege & aPrivilege) const
    {
        return mPath == aPath && mPrivilege == aPrivilege;
    }
};

} // namespace app
} // namespace chip
