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
    case AcceptedCommandList::Id:
        return EncodeCommandList(aPath, aEncoder, &CommandHandlerInterface::EnumerateAcceptedCommands,
                                 mCluster->acceptedCommandList);
    case GeneratedCommandList::Id:
        return EncodeCommandList(aPath, aEncoder, &CommandHandlerInterface::EnumerateGeneratedCommands,
                                 mCluster->generatedCommandList);
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

CHIP_ERROR GlobalAttributeReader::EncodeCommandList(const ConcreteClusterPath & aClusterPath, AttributeValueEncoder & aEncoder,
                                                    GlobalAttributeReader::CommandListEnumerator aEnumerator,
                                                    const CommandId * aClusterCommandList)
{
    return aEncoder.EncodeList([&](const auto & encoder) {
        auto * commandHandler =
            CommandHandlerInterfaceRegistry::Instance().GetCommandHandler(aClusterPath.mEndpointId, aClusterPath.mClusterId);
        if (commandHandler)
        {
            struct Context
            {
                decltype(encoder) & commandIdEncoder;
                CHIP_ERROR err;
            } context{ encoder, CHIP_NO_ERROR };
            CHIP_ERROR err = (commandHandler->*aEnumerator)(
                aClusterPath,
                [](CommandId command, void * closure) -> Loop {
                    auto * ctx = static_cast<Context *>(closure);
                    ctx->err   = ctx->commandIdEncoder.Encode(command);
                    if (ctx->err != CHIP_NO_ERROR)
                    {
                        return Loop::Break;
                    }
                    return Loop::Continue;
                },
                &context);
            if (err != CHIP_ERROR_NOT_IMPLEMENTED)
            {
                return context.err;
            }
            // Else fall through to the list in aClusterCommandList.
        }

        for (const CommandId * cmd = aClusterCommandList; cmd != nullptr && *cmd != kInvalidCommandId; cmd++)
        {
            ReturnErrorOnFailure(encoder.Encode(*cmd));
        }
        return CHIP_NO_ERROR;
    });
}

} // namespace Compatibility
} // namespace app
} // namespace chip
