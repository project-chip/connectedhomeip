/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-objects.h>
#include <lib/support/CodeUtils.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Actions {

static constexpr size_t kActionNameMaxSize       = 128u;
static constexpr size_t kEndpointListNameMaxSize = 128u;
static constexpr size_t kEndpointListMaxSize     = 256u;

struct ActionStructStorage : public Structs::ActionStruct::Type
{
    ActionStructStorage(){};

    ActionStructStorage(uint16_t aAction, const CharSpan & aActionName, ActionTypeEnum aActionType, uint16_t aEpListID,
                        BitMask<CommandBits> aCommands, ActionStateEnum aActionState)
    {
        Set(aAction, aActionName, aActionType, aEpListID, aCommands, aActionState);
    }

    ActionStructStorage(const ActionStructStorage & aAction) { *this = aAction; }

    ActionStructStorage & operator=(const ActionStructStorage & aAction)
    {
        Set(aAction.actionID, aAction.name, aAction.type, aAction.endpointListID, aAction.supportedCommands, aAction.state);
        return *this;
    }

    void Set(uint16_t aAction, const CharSpan & aActionName, ActionTypeEnum aActionType, uint16_t aEpListID,
             BitMask<CommandBits> aCommands, ActionStateEnum aActionState)
    {
        actionID          = aAction;
        type              = aActionType;
        endpointListID    = aEpListID;
        supportedCommands = aCommands;
        state             = aActionState;
        MutableCharSpan actionName(mBuffer);
        CopyCharSpanToMutableCharSpanWithTruncation(aActionName, actionName);
        name = actionName;
    }

private:
    char mBuffer[kActionNameMaxSize];
};

struct EndpointListStorage : public Structs::EndpointListStruct::Type
{
    EndpointListStorage(){};

    EndpointListStorage(uint16_t aEpListId, const CharSpan & aEpListName, EndpointListTypeEnum aEpListType,
                        const DataModel::List<const EndpointId> & aEndpointList)
    {
        Set(aEpListId, aEpListName, aEpListType, aEndpointList);
    }

    EndpointListStorage(const EndpointListStorage & aEpList) { *this = aEpList; }

    EndpointListStorage & operator=(const EndpointListStorage & aEpList)
    {
        Set(aEpList.endpointListID, aEpList.name, aEpList.type, aEpList.endpoints);
        return *this;
    }

    void Set(uint16_t aEpListId, const CharSpan & aEpListName, EndpointListTypeEnum aEpListType,
             const DataModel::List<const EndpointId> & aEndpointList)
    {
        endpointListID    = aEpListId;
        type              = aEpListType;
        size_t epListSize = std::min(aEndpointList.size(), MATTER_ARRAY_SIZE(mEpList));

        for (size_t index = 0; index < epListSize; index++)
        {
            mEpList[index] = aEndpointList[index];
        }
        endpoints = DataModel::List<const EndpointId>(Span(mEpList, epListSize));
        MutableCharSpan epListName(mBuffer);
        CopyCharSpanToMutableCharSpanWithTruncation(aEpListName, epListName);
        name = epListName;
    }

private:
    char mBuffer[kEndpointListNameMaxSize];
    EndpointId mEpList[kEndpointListMaxSize];
};

} // namespace Actions
} // namespace Clusters
} // namespace app
} // namespace chip
