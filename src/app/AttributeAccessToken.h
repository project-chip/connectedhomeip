/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
