/*
 *    Copyright (c) 2021-2024 Project CHIP Authors
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
#include "app/ConcreteCommandPath.h"
#include "lib/core/CHIPError.h"
#include "lib/core/DataModelTypes.h"
#include "lib/support/CodeUtils.h"
#include <app/util/ember-global-attribute-access-interface.h>

#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/GlobalAttributes.h>
#include <app/InteractionModelEngine.h>

namespace chip {
namespace app {
namespace Compatibility {

CHIP_ERROR GlobalAttributeReader::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    using namespace Clusters::Globals::Attributes;
    switch (aPath.mAttributeId)
    {
    case AttributeList::Id:
        return aEncoder.EncodeList([this](const auto & encoder) {
            const size_t count     = mCluster->attributeCount;
            bool addedExtraGlobals = false;
            for (size_t i = 0; i < count; ++i)
            {
                AttributeId id              = mCluster->attributes[i].attributeId;
                constexpr auto lastGlobalId = GlobalAttributesNotInMetadata[ArraySize(GlobalAttributesNotInMetadata) - 1];
                // If EventList is not supported. The GlobalAttributesNotInMetadata is missing one id here.
                static_assert(lastGlobalId - GlobalAttributesNotInMetadata[0] == ArraySize(GlobalAttributesNotInMetadata),
                              "Ids in GlobalAttributesNotInMetadata not consecutive (except EventList)");
                if (!addedExtraGlobals && id > lastGlobalId)
                {
                    for (const auto & globalId : GlobalAttributesNotInMetadata)
                    {
                        ReturnErrorOnFailure(encoder.Encode(globalId));
                    }
                    addedExtraGlobals = true;
                }
                ReturnErrorOnFailure(encoder.Encode(id));
            }
            if (!addedExtraGlobals)
            {
                for (const auto & globalId : GlobalAttributesNotInMetadata)
                {
                    ReturnErrorOnFailure(encoder.Encode(globalId));
                }
            }
            return CHIP_NO_ERROR;
        });
    case AcceptedCommandList::Id: {
        auto * commandHandler = CommandHandlerInterfaceRegistry::Instance().GetCommandHandler(aPath.mEndpointId, aPath.mClusterId);
        return aEncoder.EncodeList([&](const auto & encoder) {
            if (mCluster->acceptedCommandList != nullptr)
            {
                for (unsigned i = 0; mCluster->acceptedCommandList[i] != kInvalidCommandId; i++)
                {
                    const ConcreteCommandPath commandPath(aPath.mEndpointId, aPath.mClusterId, mCluster->acceptedCommandList[i]);
                    if ((commandHandler == nullptr) || commandHandler->AcceptsCommandId(commandPath))
                    {
                        ReturnErrorOnFailure(encoder.Encode(commandPath.mCommandId));
                    }
                }
            }
            return CHIP_NO_ERROR;
        });
    }

    case GeneratedCommandList::Id: {
        auto * commandHandler = CommandHandlerInterfaceRegistry::Instance().GetCommandHandler(aPath.mEndpointId, aPath.mClusterId);
        return aEncoder.EncodeList([&](const auto & encoder) {
            if (mCluster->generatedCommandList != nullptr)
            {
                for (unsigned i = 0; mCluster->generatedCommandList[i] != kInvalidCommandId; i++)
                {
                    const ConcreteCommandPath commandPath(aPath.mEndpointId, aPath.mClusterId, mCluster->generatedCommandList[i]);
                    if ((commandHandler == nullptr) || commandHandler->GeneratesCommandId(commandPath))
                    {
                        ReturnErrorOnFailure(encoder.Encode(commandPath.mCommandId));
                    }
                }
            }
            return CHIP_NO_ERROR;
        });
    }
    default:
        // This function is only called if attributeCluster is non-null in
        // ReadSingleClusterData, which only happens for attributes listed in
        // GlobalAttributesNotInMetadata.  If we reach this code, someone added
        // a global attribute to that list but not the above switch.
        VerifyOrDieWithMsg(false, DataManagement, "Unexpected global attribute: " ChipLogFormatMEI,
                           ChipLogValueMEI(aPath.mAttributeId));
        return CHIP_NO_ERROR;
    }
}

} // namespace Compatibility
} // namespace app
} // namespace chip
