/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

/**
 * @brief
 *    File contains definitions on how a connection to a peer can be defined.
 *
 */

#include "PeerAddress.h"

namespace chip {
namespace Transport {

bool PeerAddress::operator==(const PeerAddress & other) const
{
    // Compare common fields
    if (mTransportType != other.mTransportType)
    {
        return false;
    }

    // Compare transport-type specific fields
    switch (mTransportType)
    {
    case Type::kNfc:
        return (mId.mNFCShortId == other.mId.mNFCShortId);

    case Type::kUdp:
    case Type::kTcp:
        return (mIPAddress == other.mIPAddress && mPort == other.mPort && mInterface == other.mInterface);

    case Type::kWiFiPAF:
        return (mId.mRemoteId == other.mId.mRemoteId);

    case Type::kBle:
    default:
        // For transport types with no additional fields to compare
        return true;
    }
}

} // namespace Transport
} // namespace chip
