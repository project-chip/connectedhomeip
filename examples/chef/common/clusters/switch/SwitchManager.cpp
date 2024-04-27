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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/switch-server/switch-server.h>
#include <app/server/Server.h>
#include <app/util/att-storage.h>
#include <app/util/attribute-storage.h>
#include <platform/PlatformManager.h>
#include "SwitchEventHandler.h"

#include "chef-rpc-actions-worker.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Switch;
using namespace chip::DeviceLayer;

using namespace chip::rpc;
using namespace chip::app;

class SwitchActionsDelegate: public chip::app::ActionsDelegate
{
public:
    SwitchActionsDelegate(ClusterId clusterId, SwitchEventHandler *eventHandler): ActionsDelegate(clusterId), mEventHandler(eventHandler){};
    ~SwitchActionsDelegate() override {};

    void AttributeWriteHandler(chip::EndpointId endpointId, chip::AttributeId attributeId, std::vector<uint32_t>args) override;
    void EventHandler(chip::EndpointId endpointId, chip::EventId eventId, std::vector<uint32_t>args) override;


private:
    SwitchEventHandler *mEventHandler;
};

void SwitchActionsDelegate::AttributeWriteHandler(chip::EndpointId endpointId, chip::AttributeId attributeId, std::vector<uint32_t>args)
{
    if (args.empty()) {
        // TODO: error
        return;
    }

    switch (attributeId) {
    case Switch::Attributes::NumberOfPositions::Id:
        {
            uint8_t data = static_cast<uint8_t>(args[0]);
            app::Clusters::Switch::Attributes::NumberOfPositions::Set(endpointId, data);
        }
        break;
    case Switch::Attributes::CurrentPosition::Id:
        {
            uint8_t data = static_cast<uint8_t>(args[0]);
            app::Clusters::Switch::Attributes::CurrentPosition::Set(endpointId, data);
        }
        break;
    case Switch::Attributes::MultiPressMax::Id:
        {
            uint8_t data = static_cast<uint8_t>(args[0]);
            app::Clusters::Switch::Attributes::MultiPressMax::Set(endpointId, data);
        }
        break;
    default:
        break;
    }
}

void SwitchActionsDelegate::EventHandler(chip::EndpointId endpointId, chip::EventId eventId, std::vector<uint32_t>args)
{
    if (args.empty()) {
        // TODO: error
        return;
    }
    switch (eventId) {
    case Events::SwitchLatched::Id:
        {
            uint8_t newPosition = static_cast<uint8_t>(args[0]);
            mEventHandler->OnSwitchLatched(endpointId, newPosition);
        }
        break;
    case Events::InitialPress::Id:
        {
            uint8_t newPosition = static_cast<uint8_t>(args[0]);
            mEventHandler->OnInitialPress(endpointId, newPosition);
        }
        break;
    case Events::LongPress::Id:
        {
            uint8_t newPosition = static_cast<uint8_t>(args[0]);
            mEventHandler->OnLongPress(endpointId, newPosition);
        }
        break;
    case Events::ShortRelease::Id:
        {
            uint8_t previousPosition = static_cast<uint8_t>(args[0]);
            mEventHandler->OnShortRelease(endpointId, previousPosition);
        }
        break;
    case Events::LongRelease::Id:
        {
            uint8_t previousPosition = static_cast<uint8_t>(args[0]);
            mEventHandler->OnLongRelease(endpointId, previousPosition);
        }
        break;
    case Events::MultiPressOngoing::Id:
        {
            if (args.size() < 2) {
                // TODO: error
                return;
            }
            uint8_t newPosition = static_cast<uint8_t>(args[0]);
            uint8_t currentNumberOfPressesCounted = static_cast<uint8_t>(args[1]);
            mEventHandler->OnMultiPressOngoing(endpointId, newPosition, currentNumberOfPressesCounted);
        }
        break;
    case Events::MultiPressComplete::Id:
        {
            if (args.size() < 2) {
                // TODO: error
                return;
            }
            uint8_t previousPosition = static_cast<uint8_t>(args[0]);
            uint8_t totalNumberOfPressesCounted = static_cast<uint8_t>(args[1]);
            mEventHandler->OnMultiPressComplete(endpointId, previousPosition, totalNumberOfPressesCounted);
        }
        break;
    default:
        break;
    }
};

// Please refer to https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/namespaces
constexpr const uint8_t kNamespaceCommonLevel = 5;
// Common Number Namespace: 5, tag 0 (Low)
constexpr const uint8_t kTagCommonLow = 0;
// Common Number Namespace: 5, tag 1 (Medium)
constexpr const uint8_t kTagCommonMedium = 1;
// Common Number Namespace: 5, tag 2 (High)
constexpr const uint8_t kTagCommonHigh = 2;

constexpr const uint8_t kNamespaceCommonNumber = 7;
// Common Number Namespace: 7, tag 0 (Zero)
constexpr const uint8_t kTagCommonZero = 0;
// Common Number Namespace: 7, tag 1 (One)
constexpr const uint8_t kTagCommonOne = 1;
// Common Number Namespace: 7, tag 2 (Two)
constexpr const uint8_t kTagCommonTwo = 2;

constexpr const uint8_t kNamespacePosition = 8;
// Common Position Namespace: 8, tag: 0 (Left)
constexpr const uint8_t kTagPositionLeft = 0;
// Common Position Namespace: 8, tag: 1 (Right)
constexpr const uint8_t kTagPositionRight = 1;
// Common Position Namespace: 8, tag: 2 (Top)
constexpr const uint8_t kTagPositionTop = 2;
// Common Position Namespace: 8, tag: 3 (Bottom)
constexpr const uint8_t kTagPositionBottom                                 = 3;
// Common Position Namespace: 8, tag: 4 (Middle)
constexpr const uint8_t kTagPositionMiddle                                 = 4;
// Common Position Namespace: 8, tag: 5 (Row)
constexpr const uint8_t kTagPositionRow                                 = 5;
// Common Position Namespace: 8, tag: 6 (Column)
constexpr const uint8_t kTagPositionColumn                              = 6;

const Clusters::Descriptor::Structs::SemanticTagStruct::Type gLatchingSwitch[] = {
    { .namespaceID = kNamespaceCommonLevel, 
        .tag = kTagCommonLow, 
        .label = chip::Optional<chip::app::DataModel::Nullable<chip::CharSpan>>(
                    { chip::app::DataModel::MakeNullable(chip::CharSpan("Low", 3)) })},
    { .namespaceID = kNamespaceCommonLevel, 
        .tag = kTagCommonMedium, 
        .label = chip::Optional<chip::app::DataModel::Nullable<chip::CharSpan>>(
                    { chip::app::DataModel::MakeNullable(chip::CharSpan("Medium", 6)) })},
    { .namespaceID = kNamespaceCommonLevel, 
        .tag = kTagCommonHigh,
        .label = chip::Optional<chip::app::DataModel::Nullable<chip::CharSpan>>(
                    { chip::app::DataModel::MakeNullable(chip::CharSpan("High", 4)) })}
};

#if 0
const Clusters::Descriptor::Structs::SemanticTagStruct::Type gEp0TagList[] = {
    { .namespaceID = kNamespaceCommonNumber, .tag = kTagCommonZero }, { .namespaceID = kNamespacePosition, .tag = kTagPositionBottom }
};
const Clusters::Descriptor::Structs::SemanticTagStruct::Type gEp1TagList[] = {
    { .namespaceID = kNamespaceCommon, .tag = kTagCommonOne }, { .namespaceID = kNamespacePosition, .tag = kTagPositionLeft }
};
const Clusters::Descriptor::Structs::SemanticTagStruct::Type gEp2TagList[] = {
    { .namespaceID = kNamespaceCommon, .tag = kTagCommonTwo }, { .namespaceID = kNamespacePosition, .tag = kTagPositionRight }
};
#endif


void emberAfSwitchClusterInitCallback(EndpointId endpointId)
{
    ChipLogProgress(Zcl, "Chef: emberAfSwitchClusterInitCallback");
printf("\033[44m %s, %d, Switch::ID=%u \033[0m \n", __func__, __LINE__, Switch::Id);
    ChefRpcActionsWorker::Instance().RegisterRpcActionsDelegate(Clusters::Switch::Id, new SwitchActionsDelegate(Clusters::Switch::Id, new SwitchEventHandler()));
    SetTagList(/* endpoint= */ 1, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(gLatchingSwitch));
}


