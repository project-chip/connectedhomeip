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

#include <oob-accessors/clusters/AmbientContextOOBAccessor.h>

#include <vector>

#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>

namespace chip::app {

namespace {

CHIP_ERROR DecodeSemanticTagList(TLV::TLVReader & reader, std::vector<Clusters::Globals::Structs::SemanticTagStruct::Type> & tags)
{
    TLV::TLVType containerType;
    ReturnErrorOnFailure(reader.EnterContainer(containerType));
    CHIP_ERROR err = CHIP_NO_ERROR;
    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        TLV::TLVType tagStructType;
        ReturnErrorOnFailure(reader.EnterContainer(tagStructType));
        Clusters::Globals::Structs::SemanticTagStruct::Type tagStruct{};
        bool hasNamespace   = false;
        bool hasTag         = false;
        CHIP_ERROR innerErr = CHIP_NO_ERROR;
        while ((innerErr = reader.Next()) == CHIP_NO_ERROR)
        {
            TLV::Tag tag = reader.GetTag();
            if (!TLV::IsContextTag(tag))
            {
                continue;
            }
            switch (TLV::TagNumFromTag(tag))
            {
            case 1:
                ReturnErrorOnFailure(reader.Get(tagStruct.namespaceID));
                hasNamespace = true;
                break;
            case 2:
                ReturnErrorOnFailure(reader.Get(tagStruct.tag));
                hasTag = true;
                break;
            default:
                break;
            }
        }
        VerifyOrReturnError(innerErr == CHIP_END_OF_TLV, innerErr);
        ReturnErrorOnFailure(reader.ExitContainer(tagStructType));
        VerifyOrReturnError(hasNamespace && hasTag, CHIP_ERROR_INVALID_ARGUMENT);
        tags.push_back(tagStruct);
    }
    VerifyOrReturnError(err == CHIP_END_OF_TLV, err);
    return reader.ExitContainer(containerType);
}

} // namespace

std::optional<CHIP_ERROR> AmbientContextOOBAccessor::HandleAction(CharSpan action, ByteSpan tlvData)
{
    if (action.data_equal("SetAmbientContextSupport"_span))
    {
        return HandleSetAmbientContextSupport(tlvData);
    }
    if (action.data_equal("AddAmbientContextDetect"_span))
    {
        return HandleAddAmbientContextDetect(tlvData);
    }
    if (action.data_equal("SetPredictedActivity"_span))
    {
        return HandleSetPredictedActivity(tlvData);
    }
    if (action.data_equal("SetSensorFusionSupported"_span))
    {
        return HandleSetSensorFusionSupported(tlvData);
    }
    if (action.data_equal("SetObjectCount"_span))
    {
        return HandleSetObjectCount(tlvData);
    }
    return std::nullopt;
}

std::optional<CHIP_ERROR> AmbientContextOOBAccessor::HandleSetAmbientContextSupport(ByteSpan tlvData) const
{
    TLV::TLVReader reader;
    reader.Init(tlvData);
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

    TLV::TLVType outerType;
    ReturnErrorOnFailure(reader.EnterContainer(outerType));

    EndpointId endpointId = kInvalidEndpointId;
    bool hasEndpointId    = false;
    std::vector<Clusters::Globals::Structs::SemanticTagStruct::Type> tags;

    CHIP_ERROR err = CHIP_NO_ERROR;
    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        TLV::Tag tag = reader.GetTag();
        if (!TLV::IsContextTag(tag))
        {
            continue;
        }
        switch (TLV::TagNumFromTag(tag))
        {
        case 1:
            ReturnErrorOnFailure(reader.Get(endpointId));
            hasEndpointId = true;
            break;
        case 2:
            ReturnErrorOnFailure(DecodeSemanticTagList(reader, tags));
            break;
        default:
            break;
        }
    }
    VerifyOrReturnError(err == CHIP_END_OF_TLV, err);
    ReturnErrorOnFailure(reader.ExitContainer(outerType));

    VerifyOrReturnError(hasEndpointId, CHIP_ERROR_INVALID_ARGUMENT);

    if (endpointId != mEndpointId)
    {
        return std::nullopt;
    }

    Span<Clusters::AmbientContextSensing::SemanticTagType> tagSpan(tags.data(), tags.size());
    return mCluster.SetAmbientContextTypeSupported(tagSpan);
}

std::optional<CHIP_ERROR> AmbientContextOOBAccessor::HandleAddAmbientContextDetect(ByteSpan tlvData) const
{
    TLV::TLVReader reader;
    reader.Init(tlvData);
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

    TLV::TLVType outerType;
    ReturnErrorOnFailure(reader.EnterContainer(outerType));

    EndpointId endpointId = kInvalidEndpointId;
    bool hasEndpointId    = false;
    uint8_t confidence    = 0;
    bool hasConfidence    = false;
    std::vector<Clusters::Globals::Structs::SemanticTagStruct::Type> tags;

    CHIP_ERROR err = CHIP_NO_ERROR;
    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        TLV::Tag tag = reader.GetTag();
        if (!TLV::IsContextTag(tag))
        {
            continue;
        }
        switch (TLV::TagNumFromTag(tag))
        {
        case 1:
            ReturnErrorOnFailure(reader.Get(endpointId));
            hasEndpointId = true;
            break;
        case 2:
            ReturnErrorOnFailure(DecodeSemanticTagList(reader, tags));
            break;
        case 3:
            ReturnErrorOnFailure(reader.Get(confidence));
            hasConfidence = true;
            break;
        default:
            break;
        }
    }
    VerifyOrReturnError(err == CHIP_END_OF_TLV, err);
    ReturnErrorOnFailure(reader.ExitContainer(outerType));

    VerifyOrReturnError(hasEndpointId, CHIP_ERROR_INVALID_ARGUMENT);

    if (endpointId != mEndpointId)
    {
        return std::nullopt;
    }

    auto tagList = DataModel::List<const Clusters::Globals::Structs::SemanticTagStruct::Type>(tags.data(), tags.size());
    Clusters::AmbientContextSensing::Structs::AmbientContextTypeStruct::Type acsType = { .ambientContextSensed = tagList };
    if (hasConfidence)
    {
        VerifyOrReturnError(confidence <= 100, CHIP_ERROR_INVALID_ARGUMENT);
        acsType.detectionConfidence.SetValue(static_cast<Percent>(confidence));
    }

    return mCluster.AddDetection(acsType);
}

std::optional<CHIP_ERROR> AmbientContextOOBAccessor::HandleSetPredictedActivity(ByteSpan tlvData) const
{
    TLV::TLVReader reader;
    reader.Init(tlvData);
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

    TLV::TLVType outerType;
    ReturnErrorOnFailure(reader.EnterContainer(outerType));

    EndpointId endpointId = kInvalidEndpointId;
    bool hasEndpointId    = false;

    std::vector<Clusters::AmbientContextSensing::Structs::PredictedActivityStruct::Type> activities;
    std::vector<std::vector<Clusters::Globals::Structs::SemanticTagStruct::Type>> allTags;

    CHIP_ERROR err = CHIP_NO_ERROR;
    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        TLV::Tag tag = reader.GetTag();
        if (!TLV::IsContextTag(tag))
        {
            continue;
        }
        switch (TLV::TagNumFromTag(tag))
        {
        case 1:
            ReturnErrorOnFailure(reader.Get(endpointId));
            hasEndpointId = true;
            break;
        case 2: {
            TLV::TLVType arrayType;
            ReturnErrorOnFailure(reader.EnterContainer(arrayType));
            CHIP_ERROR arrayErr = CHIP_NO_ERROR;
            while ((arrayErr = reader.Next()) == CHIP_NO_ERROR)
            {
                TLV::TLVType structType;
                ReturnErrorOnFailure(reader.EnterContainer(structType));

                Clusters::AmbientContextSensing::Structs::PredictedActivityStruct::Type item{};
                std::vector<Clusters::Globals::Structs::SemanticTagStruct::Type> itemTags;

                CHIP_ERROR itemErr = CHIP_NO_ERROR;
                while ((itemErr = reader.Next()) == CHIP_NO_ERROR)
                {
                    TLV::Tag itemTag = reader.GetTag();
                    if (!TLV::IsContextTag(itemTag))
                    {
                        continue;
                    }
                    switch (TLV::TagNumFromTag(itemTag))
                    {
                    case 1:
                        ReturnErrorOnFailure(reader.Get(item.startTimestamp));
                        break;
                    case 2:
                        ReturnErrorOnFailure(reader.Get(item.endTimestamp));
                        break;
                    case 3:
                        ReturnErrorOnFailure(DecodeSemanticTagList(reader, itemTags));
                        break;
                    case 4: {
                        bool crowdDetected = false;
                        ReturnErrorOnFailure(reader.Get(crowdDetected));
                        item.crowdDetected.SetValue(crowdDetected);
                        break;
                    }
                    case 5: {
                        uint8_t crowdCount = 0;
                        ReturnErrorOnFailure(reader.Get(crowdCount));
                        item.crowdCount.SetValue(crowdCount);
                        break;
                    }
                    case 6: {
                        uint8_t conf = 0;
                        ReturnErrorOnFailure(reader.Get(conf));
                        VerifyOrReturnError(conf <= 100, CHIP_ERROR_INVALID_ARGUMENT);
                        item.confidence = static_cast<Percent>(conf);
                        break;
                    }
                    default:
                        break;
                    }
                }
                VerifyOrReturnError(itemErr == CHIP_END_OF_TLV, itemErr);
                ReturnErrorOnFailure(reader.ExitContainer(structType));

                allTags.push_back(std::move(itemTags));
                activities.push_back(item);
            }
            VerifyOrReturnError(arrayErr == CHIP_END_OF_TLV, arrayErr);
            ReturnErrorOnFailure(reader.ExitContainer(arrayType));
            break;
        }
        default:
            break;
        }
    }
    VerifyOrReturnError(err == CHIP_END_OF_TLV, err);
    ReturnErrorOnFailure(reader.ExitContainer(outerType));

    VerifyOrReturnError(hasEndpointId, CHIP_ERROR_INVALID_ARGUMENT);

    if (endpointId != mEndpointId)
    {
        return std::nullopt;
    }

    for (size_t i = 0; i < activities.size(); i++)
    {
        activities[i].ambientContextType.SetValue(
            DataModel::List<const Clusters::Globals::Structs::SemanticTagStruct::Type>(allTags[i].data(), allTags[i].size()));
    }

    Span<Clusters::AmbientContextSensing::PredictedActivityType> activitySpan(activities.data(), activities.size());
    return mCluster.SetPredictedActivity(activitySpan);
}

std::optional<CHIP_ERROR> AmbientContextOOBAccessor::HandleSetSensorFusionSupported(ByteSpan tlvData) const
{
    TLV::TLVReader reader;
    reader.Init(tlvData);
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

    TLV::TLVType outerType;
    ReturnErrorOnFailure(reader.EnterContainer(outerType));

    EndpointId endpointId = kInvalidEndpointId;
    bool hasEndpointId    = false;
    std::vector<Clusters::Globals::Structs::SemanticTagStruct::Type> tags;

    CHIP_ERROR err = CHIP_NO_ERROR;
    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        TLV::Tag tag = reader.GetTag();
        if (!TLV::IsContextTag(tag))
        {
            continue;
        }
        switch (TLV::TagNumFromTag(tag))
        {
        case 1:
            ReturnErrorOnFailure(reader.Get(endpointId));
            hasEndpointId = true;
            break;
        case 2:
            ReturnErrorOnFailure(DecodeSemanticTagList(reader, tags));
            break;
        default:
            break;
        }
    }
    VerifyOrReturnError(err == CHIP_END_OF_TLV, err);
    ReturnErrorOnFailure(reader.ExitContainer(outerType));

    VerifyOrReturnError(hasEndpointId, CHIP_ERROR_INVALID_ARGUMENT);

    if (endpointId != mEndpointId)
    {
        return std::nullopt;
    }

    Span<Clusters::AmbientContextSensing::SemanticTagType> tagSpan(tags.data(), tags.size());
    return mCluster.SetSensorFusionSupported(tagSpan);
}

std::optional<CHIP_ERROR> AmbientContextOOBAccessor::HandleSetObjectCount(ByteSpan tlvData) const
{
    TLV::TLVReader reader;
    reader.Init(tlvData);
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

    TLV::TLVType outerType;
    ReturnErrorOnFailure(reader.EnterContainer(outerType));

    EndpointId endpointId = kInvalidEndpointId;
    uint16_t objectCount  = 0;
    bool hasEndpointId    = false;
    bool hasObjectCount   = false;

    CHIP_ERROR err = CHIP_NO_ERROR;
    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        TLV::Tag tag = reader.GetTag();
        if (!TLV::IsContextTag(tag))
        {
            continue;
        }
        switch (TLV::TagNumFromTag(tag))
        {
        case 1:
            ReturnErrorOnFailure(reader.Get(endpointId));
            hasEndpointId = true;
            break;
        case 2:
            ReturnErrorOnFailure(reader.Get(objectCount));
            hasObjectCount = true;
            break;
        default:
            break;
        }
    }
    VerifyOrReturnError(err == CHIP_END_OF_TLV, err);
    ReturnErrorOnFailure(reader.ExitContainer(outerType));

    VerifyOrReturnError(hasEndpointId && hasObjectCount, CHIP_ERROR_INVALID_ARGUMENT);

    if (endpointId != mEndpointId)
    {
        return std::nullopt;
    }

    return mCluster.SetObjectCount(objectCount);
}

} // namespace chip::app
