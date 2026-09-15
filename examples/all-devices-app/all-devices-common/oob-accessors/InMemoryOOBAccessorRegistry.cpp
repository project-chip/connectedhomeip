/*
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

#include <oob-accessors/InMemoryOOBAccessorRegistry.h>

#include <access/SubjectDescriptor.h>
#include <app/AttributeValueDecoder.h>
#include <app/InteractionModelEngine.h>
#include <app/data-model-provider/MetadataLookup.h>
#include <app/data-model-provider/Provider.h>
#include <lib/support/CodeUtils.h>
#include <oob-accessors/OOBDataSerializer.h>

namespace chip::app {

CHIP_ERROR InMemoryOOBAccessorRegistry::Register(std::unique_ptr<OOBAccessor> accessor)
{
    VerifyOrReturnError(accessor != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    mAccessors.push_back(std::move(accessor));
    return CHIP_NO_ERROR;
}

CHIP_ERROR InMemoryOOBAccessorRegistry::HandleAction(CharSpan action, ByteSpan tlvData)
{
    for (const auto & accessor : mAccessors)
    {
        auto result = accessor->HandleAction(action, tlvData);
        if (result.has_value())
        {
            return *result;
        }
    }

    if (action.data_equal("SetAttribute"_span))
    {
        return WriteAttributeToDataModel(tlvData);
    }

    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR InMemoryOOBAccessorRegistry::WriteAttributeToDataModel(ByteSpan tlvData)
{
    auto parseResult = OOBDataSerializer::ParseAttributeRequest(tlvData);
    if (std::holds_alternative<CHIP_ERROR>(parseResult))
    {
        return std::get<CHIP_ERROR>(parseResult);
    }

    auto & request = std::get<OOBDataSerializer::AttributeRequest>(parseResult);

    DataModel::Provider * provider = InteractionModelEngine::GetInstance()->GetDataModelProvider();
    VerifyOrReturnError(provider != nullptr, CHIP_ERROR_NOT_FOUND);

    DataModel::ServerClusterFinder serverClusterFinder(provider);
    auto info = serverClusterFinder.Find(request.path);
    VerifyOrReturnError(info.has_value(), CHIP_ERROR_NOT_FOUND);

    Access::SubjectDescriptor subjectDescriptor{ .authMode = Access::AuthMode::kInternalDeviceAccess };
    DataModel::WriteAttributeRequest writeRequest(request.path, subjectDescriptor);
    AttributeValueDecoder decoder(request.value, subjectDescriptor);

    return provider->WriteAttribute(writeRequest, decoder).GetUnderlyingError();
}

} // namespace chip::app
