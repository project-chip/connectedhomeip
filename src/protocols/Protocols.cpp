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

#include <protocols/bdx/BdxMessages.h>
#include <protocols/echo/Echo.h>
#include <protocols/interaction_model/Constants.h>
#include <protocols/secure_channel/Constants.h>
#include <protocols/user_directed_commissioning/UserDirectedCommissioning.h>

namespace chip {
namespace Protocols {

static const char sUnknownTypeName[] = "----";

static const char * LookupMessageTypeName(const MessageTypeNameLookup * lookupTable, size_t tableSize, uint8_t msgType)
{
    for (auto ptr = lookupTable; ptr != (lookupTable + tableSize); ptr++)
    {
        if (ptr->mId == msgType)
        {
            return ptr->mName;
        }
    }

    return sUnknownTypeName;
}

const char * GetProtocolName(Id protocolId)
{
    if (protocolId.GetVendorId() != VendorId::Common)
    {
        return sUnknownTypeName;
    }

    switch (protocolId.GetProtocolId())
    {
    case InteractionModel::Id.GetProtocolId():
        return InteractionModel::kProtocolName;
        break;

    case SecureChannel::Id.GetProtocolId():
        return SecureChannel::kProtocolName;
        break;

    case BDX::Id.GetProtocolId():
        return bdx::kProtocolName;
        break;

    case Echo::Id.GetProtocolId():
        return Echo::kProtocolName;
        break;

    case UserDirectedCommissioning::Id.GetProtocolId():
        return UserDirectedCommissioning::kProtocolName;
        break;

    default:
        return sUnknownTypeName;
    }
}

const char * GetMessageTypeName(Id protocolId, uint8_t msgType)
{
    if (protocolId.GetVendorId() != VendorId::Common)
    {
        return sUnknownTypeName;
    }

    const MessageTypeNameLookup * lookupTable = nullptr;
    size_t lookupTableSize                    = 0;

    switch (protocolId.GetProtocolId())
    {
    case InteractionModel::Id.GetProtocolId():
        lookupTable     = MessageTypeTraits<InteractionModel::MsgType>::GetTypeToNameTable()->begin();
        lookupTableSize = MessageTypeTraits<InteractionModel::MsgType>::GetTypeToNameTable()->size();
        break;

    case SecureChannel::Id.GetProtocolId():
        lookupTable     = MessageTypeTraits<SecureChannel::MsgType>::GetTypeToNameTable()->begin();
        lookupTableSize = MessageTypeTraits<SecureChannel::MsgType>::GetTypeToNameTable()->size();
        break;

    case BDX::Id.GetProtocolId():
        lookupTable     = MessageTypeTraits<bdx::MessageType>::GetTypeToNameTable()->begin();
        lookupTableSize = MessageTypeTraits<bdx::MessageType>::GetTypeToNameTable()->size();
        break;

    case Echo::Id.GetProtocolId():
        lookupTable     = MessageTypeTraits<Echo::MsgType>::GetTypeToNameTable()->begin();
        lookupTableSize = MessageTypeTraits<Echo::MsgType>::GetTypeToNameTable()->size();
        break;

    case UserDirectedCommissioning::Id.GetProtocolId():
        lookupTable     = MessageTypeTraits<UserDirectedCommissioning::MsgType>::GetTypeToNameTable()->begin();
        lookupTableSize = MessageTypeTraits<UserDirectedCommissioning::MsgType>::GetTypeToNameTable()->size();
        break;

    default:
        //
        // TODO: Add support at some point to let applications to route to custom protocols defined outside of the standard
        // namespace in the SDK.
        //
        return sUnknownTypeName;
    }

    return LookupMessageTypeName(lookupTable, lookupTableSize, msgType);
}

} // namespace Protocols
} // namespace chip
