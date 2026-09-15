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

#include <posix/named_pipe/translators/AmbientContextTranslator.h>

#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>

namespace chip::app::NamedPipe {

namespace {

CHIP_ERROR EncodeSemanticTagList(TLV::TLVWriter & writer, TLV::Tag tag, const Json::Value & actArray)
{
    TLV::TLVType arrayType;
    ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Array, arrayType));
    for (Json::ArrayIndex i = 0; i < actArray.size(); i++)
    {
        const Json::Value & item = actArray[i];
        VerifyOrReturnError(item.isObject(), CHIP_ERROR_INVALID_ARGUMENT);

        auto typeIdOpt = CommandTranslator::ExtractUInt<uint8_t>(item, "TypeId");
        auto tagIdOpt  = CommandTranslator::ExtractUInt<uint8_t>(item, "TagId");
        VerifyOrReturnError(typeIdOpt.has_value() && tagIdOpt.has_value(), CHIP_ERROR_INVALID_ARGUMENT);

        TLV::TLVType structType;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, structType));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(1), *typeIdOpt));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(2), *tagIdOpt));
        ReturnErrorOnFailure(writer.EndContainer(structType));
    }
    return writer.EndContainer(arrayType);
}

} // namespace

CHIP_ERROR AmbientContextTranslator::TranslateAndExecute(EndpointId endpointId, const Json::Value & json,
                                                         OOBAccessorRegistry & registry) const
{
    std::string actionName = json["Name"].asString();
    if (actionName == "SetAmbientContextSupport")
    {
        return TranslateSetAmbientContextSupport(endpointId, json, registry);
    }
    if (actionName == "AddAmbientContextDetect")
    {
        return TranslateAddAmbientContextDetect(endpointId, json, registry);
    }
    if (actionName == "SetPredictedActivity")
    {
        return TranslateSetPredictedActivity(endpointId, json, registry);
    }
    if (actionName == "SetSensorFusionSupported")
    {
        return TranslateSetSensorFusionSupported(endpointId, json, registry);
    }
    if (actionName == "SetObjectCount")
    {
        return TranslateSetObjectCount(endpointId, json, registry);
    }
    if (actionName == "AddAmbientSensingContributor")
    {
        return TranslateAddAmbientSensingContributor(endpointId, json, registry);
    }
    if (actionName == "RemoveAmbientSensingContributor")
    {
        return TranslateRemoveAmbientSensingContributor(endpointId, json, registry);
    }
    if (actionName == "AddAmbientSensingNonMatterContributor")
    {
        return TranslateAddAmbientSensingNonMatterContributor(endpointId, json, registry);
    }
    if (actionName == "RemoveAmbientSensingNonMatterContributor")
    {
        return TranslateRemoveAmbientSensingNonMatterContributor(endpointId, json, registry);
    }
    if (actionName == "UpdateAmbientSensingContributorStatus")
    {
        return TranslateUpdateAmbientSensingContributorStatus(endpointId, json, registry);
    }
    if (actionName == "SetAmbientSensingUnionName")
    {
        return TranslateSetAmbientSensingUnionName(endpointId, json, registry);
    }

    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR AmbientContextTranslator::TranslateSetAmbientContextSupport(EndpointId endpointId, const Json::Value & json,
                                                                       OOBAccessorRegistry & registry) const
{
    VerifyOrReturnError(json.isMember("AmbientContextType") && json["AmbientContextType"].isArray(), CHIP_ERROR_INVALID_ARGUMENT);

    // Sizing: Each semantic tag structure is 8 bytes in TLV (2B struct wrapper + 3B TypeId + 3B TagId).
    // Sizing with 16 bytes per item provides 2x margin over the 8 bytes requirement.
    TlvMessageBuffer message(static_cast<size_t>(json["AmbientContextType"].size()) * 16);

    TLV::TLVType outerType;
    ReturnErrorOnFailure(message.Writer().StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));
    ReturnErrorOnFailure(message.Writer().Put(TLV::ContextTag(1), endpointId));
    ReturnErrorOnFailure(EncodeSemanticTagList(message.Writer(), TLV::ContextTag(2), json["AmbientContextType"]));
    ReturnErrorOnFailure(message.Writer().EndContainer(outerType));

    ByteSpan payload;
    ReturnErrorOnFailure(message.Finalize(payload));
    return registry.HandleAction("SetAmbientContextSupport"_span, payload);
}

CHIP_ERROR AmbientContextTranslator::TranslateAddAmbientContextDetect(EndpointId endpointId, const Json::Value & json,
                                                                      OOBAccessorRegistry & registry) const
{
    VerifyOrReturnError(json.isMember("AmbientContextType") && json["AmbientContextType"].isArray(), CHIP_ERROR_INVALID_ARGUMENT);

    // Sizing: Each semantic tag structure is 8 bytes in TLV; 16 bytes per item provides 2x margin.
    TlvMessageBuffer message(static_cast<size_t>(json["AmbientContextType"].size()) * 16);

    TLV::TLVType outerType;
    ReturnErrorOnFailure(message.Writer().StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));
    ReturnErrorOnFailure(message.Writer().Put(TLV::ContextTag(1), endpointId));
    ReturnErrorOnFailure(EncodeSemanticTagList(message.Writer(), TLV::ContextTag(2), json["AmbientContextType"]));

    if (json.isMember("DetectionConfidence"))
    {
        auto confOpt = ExtractUInt<uint8_t>(json, "DetectionConfidence");
        VerifyOrReturnError(confOpt.has_value() && *confOpt <= 100, CHIP_ERROR_INVALID_ARGUMENT);
        ReturnErrorOnFailure(message.Writer().Put(TLV::ContextTag(3), *confOpt));
    }

    ReturnErrorOnFailure(message.Writer().EndContainer(outerType));

    ByteSpan payload;
    ReturnErrorOnFailure(message.Finalize(payload));
    return registry.HandleAction("AddAmbientContextDetect"_span, payload);
}

CHIP_ERROR AmbientContextTranslator::TranslateSetPredictedActivity(EndpointId endpointId, const Json::Value & json,
                                                                   OOBAccessorRegistry & registry) const
{
    VerifyOrReturnError(json.isMember("PredAct") && json["PredAct"].isArray(), CHIP_ERROR_INVALID_ARGUMENT);
    const Json::Value & predActArray = json["PredAct"];

    size_t totalTags = 0;
    for (Json::ArrayIndex i = 0; i < predActArray.size(); i++)
    {
        if (predActArray[i].isObject() && predActArray[i].isMember("AmbientContextType") &&
            predActArray[i]["AmbientContextType"].isArray())
        {
            totalTags += predActArray[i]["AmbientContextType"].size();
        }
    }

    // Sizing: Fixed fields per activity item require ~28 bytes (timestamps, flags, confidence, struct envelope).
    // Each semantic tag structure requires 8 bytes (2B struct wrapper + 3B TypeId + 3B TagId).
    // Sizing with 64 bytes per item + 16 bytes per tag provides >2x margin.
    TlvMessageBuffer message((static_cast<size_t>(predActArray.size()) * 64) + (totalTags * 16));

    TLV::TLVType outerType;
    ReturnErrorOnFailure(message.Writer().StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));
    ReturnErrorOnFailure(message.Writer().Put(TLV::ContextTag(1), endpointId));

    TLV::TLVType arrayType;
    ReturnErrorOnFailure(message.Writer().StartContainer(TLV::ContextTag(2), TLV::kTLVType_Array, arrayType));
    for (Json::ArrayIndex i = 0; i < predActArray.size(); i++)
    {
        const Json::Value & item = predActArray[i];
        VerifyOrReturnError(item.isObject() && item.isMember("AmbientContextType") && item["AmbientContextType"].isArray(),
                            CHIP_ERROR_INVALID_ARGUMENT);

        auto startTStampOpt = ExtractUInt<uint32_t>(item, "StartTStamp");
        auto endTStampOpt   = ExtractUInt<uint32_t>(item, "EndTStamp");
        auto confOpt        = ExtractUInt<uint8_t>(item, "Conf");
        VerifyOrReturnError(startTStampOpt.has_value() && endTStampOpt.has_value() && confOpt.has_value() && *confOpt <= 100,
                            CHIP_ERROR_INVALID_ARGUMENT);

        TLV::TLVType itemType;
        ReturnErrorOnFailure(message.Writer().StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, itemType));
        ReturnErrorOnFailure(message.Writer().Put(TLV::ContextTag(1), *startTStampOpt));
        ReturnErrorOnFailure(message.Writer().Put(TLV::ContextTag(2), *endTStampOpt));
        ReturnErrorOnFailure(EncodeSemanticTagList(message.Writer(), TLV::ContextTag(3), item["AmbientContextType"]));

        if (item.isMember("CrowdDetect"))
        {
            auto crowdDetectOpt = ExtractBool(item, "CrowdDetect");
            VerifyOrReturnError(crowdDetectOpt.has_value(), CHIP_ERROR_INVALID_ARGUMENT);
            ReturnErrorOnFailure(message.Writer().Put(TLV::ContextTag(4), *crowdDetectOpt));
        }
        if (item.isMember("CrowdCnt"))
        {
            auto crowdCntOpt = ExtractUInt<uint8_t>(item, "CrowdCnt");
            VerifyOrReturnError(crowdCntOpt.has_value(), CHIP_ERROR_INVALID_ARGUMENT);
            ReturnErrorOnFailure(message.Writer().Put(TLV::ContextTag(5), *crowdCntOpt));
        }
        ReturnErrorOnFailure(message.Writer().Put(TLV::ContextTag(6), *confOpt));
        ReturnErrorOnFailure(message.Writer().EndContainer(itemType));
    }
    ReturnErrorOnFailure(message.Writer().EndContainer(arrayType));
    ReturnErrorOnFailure(message.Writer().EndContainer(outerType));

    ByteSpan payload;
    ReturnErrorOnFailure(message.Finalize(payload));
    return registry.HandleAction("SetPredictedActivity"_span, payload);
}

CHIP_ERROR AmbientContextTranslator::TranslateSetSensorFusionSupported(EndpointId endpointId, const Json::Value & json,
                                                                       OOBAccessorRegistry & registry) const
{
    VerifyOrReturnError(json.isMember("AmbientContextType") && json["AmbientContextType"].isArray(), CHIP_ERROR_INVALID_ARGUMENT);

    // Sizing: Each semantic tag structure is 8 bytes in TLV; 16 bytes per item provides 2x margin.
    TlvMessageBuffer message(static_cast<size_t>(json["AmbientContextType"].size()) * 16);

    TLV::TLVType outerType;
    ReturnErrorOnFailure(message.Writer().StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));
    ReturnErrorOnFailure(message.Writer().Put(TLV::ContextTag(1), endpointId));
    ReturnErrorOnFailure(EncodeSemanticTagList(message.Writer(), TLV::ContextTag(2), json["AmbientContextType"]));
    ReturnErrorOnFailure(message.Writer().EndContainer(outerType));

    ByteSpan payload;
    ReturnErrorOnFailure(message.Finalize(payload));
    return registry.HandleAction("SetSensorFusionSupported"_span, payload);
}

CHIP_ERROR AmbientContextTranslator::TranslateSetObjectCount(EndpointId endpointId, const Json::Value & json,
                                                             OOBAccessorRegistry & registry) const
{
    auto objectCount = ExtractUInt<uint16_t>(json, "ObjectCount");
    VerifyOrReturnError(objectCount.has_value(), CHIP_ERROR_INVALID_ARGUMENT);

    return DispatchAction(registry, "SetObjectCount"_span, endpointId, *objectCount);
}

namespace {

NodeId ParseNodeIdFromJson(const Json::Value & json)
{
    if (!json.isMember("NodeId"))
    {
        return kUndefinedNodeId;
    }
    if (json["NodeId"].isString())
    {
        std::string s = json["NodeId"].asString();
        return static_cast<NodeId>(strtoull(s.c_str(), nullptr, 0));
    }
    if (json["NodeId"].isIntegral())
    {
        return static_cast<NodeId>(json["NodeId"].asUInt64());
    }
    return kUndefinedNodeId;
}

} // namespace

CHIP_ERROR AmbientContextTranslator::TranslateAddAmbientSensingContributor(EndpointId endpointId, const Json::Value & json,
                                                                           OOBAccessorRegistry & registry) const
{
    NodeId nodeId = ParseNodeIdFromJson(json);
    VerifyOrReturnError(nodeId != kUndefinedNodeId, CHIP_ERROR_INVALID_ARGUMENT);

    auto contributorEndpointIdOpt = ExtractUInt<EndpointId>(json, "ContributorEndpointId");
    VerifyOrReturnError(contributorEndpointIdOpt.has_value(), CHIP_ERROR_INVALID_ARGUMENT);

    auto statusOpt = ExtractUInt<uint8_t>(json, "Status");
    VerifyOrReturnError(statusOpt.has_value(), CHIP_ERROR_INVALID_ARGUMENT);

    // Estimate buffer: nodeId(8) + contributorEndpointId(4) + status(3) + optional name(128) + optional fabricIndex(3)
    TlvMessageBuffer message(150);

    TLV::TLVType outerType;
    ReturnErrorOnFailure(message.Writer().StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));
    ReturnErrorOnFailure(message.Writer().Put(TLV::ContextTag(1), endpointId));
    ReturnErrorOnFailure(message.Writer().Put(TLV::ContextTag(2), nodeId));
    ReturnErrorOnFailure(message.Writer().Put(TLV::ContextTag(3), *contributorEndpointIdOpt));
    ReturnErrorOnFailure(message.Writer().Put(TLV::ContextTag(4), *statusOpt));

    if (json.isMember("ContributorName") && json["ContributorName"].isString())
    {
        std::string name = json["ContributorName"].asString();
        ReturnErrorOnFailure(message.Writer().PutString(TLV::ContextTag(5), name.c_str()));
    }

    if (json.isMember("FabricIndex") && json["FabricIndex"].isIntegral())
    {
        auto fabricIndexOpt = ExtractUInt<FabricIndex>(json, "FabricIndex");
        VerifyOrReturnError(fabricIndexOpt.has_value(), CHIP_ERROR_INVALID_ARGUMENT);
        ReturnErrorOnFailure(message.Writer().Put(TLV::ContextTag(6), *fabricIndexOpt));
    }

    ReturnErrorOnFailure(message.Writer().EndContainer(outerType));

    ByteSpan payload;
    ReturnErrorOnFailure(message.Finalize(payload));
    return registry.HandleAction("AddAmbientSensingContributor"_span, payload);
}

CHIP_ERROR AmbientContextTranslator::TranslateRemoveAmbientSensingContributor(EndpointId endpointId, const Json::Value & json,
                                                                              OOBAccessorRegistry & registry) const
{
    NodeId nodeId = ParseNodeIdFromJson(json);
    VerifyOrReturnError(nodeId != kUndefinedNodeId, CHIP_ERROR_INVALID_ARGUMENT);

    auto contributorEndpointIdOpt = ExtractUInt<EndpointId>(json, "ContributorEndpointId");
    VerifyOrReturnError(contributorEndpointIdOpt.has_value(), CHIP_ERROR_INVALID_ARGUMENT);

    TlvMessageBuffer message(16);

    TLV::TLVType outerType;
    ReturnErrorOnFailure(message.Writer().StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));
    ReturnErrorOnFailure(message.Writer().Put(TLV::ContextTag(1), endpointId));
    ReturnErrorOnFailure(message.Writer().Put(TLV::ContextTag(2), nodeId));
    ReturnErrorOnFailure(message.Writer().Put(TLV::ContextTag(3), *contributorEndpointIdOpt));
    ReturnErrorOnFailure(message.Writer().EndContainer(outerType));

    ByteSpan payload;
    ReturnErrorOnFailure(message.Finalize(payload));
    return registry.HandleAction("RemoveAmbientSensingContributor"_span, payload);
}

CHIP_ERROR AmbientContextTranslator::TranslateAddAmbientSensingNonMatterContributor(EndpointId endpointId, const Json::Value & json,
                                                                                    OOBAccessorRegistry & registry) const
{
    VerifyOrReturnError(json.isMember("ContributorName") && json["ContributorName"].isString(), CHIP_ERROR_INVALID_ARGUMENT);
    std::string name = json["ContributorName"].asString();
    VerifyOrReturnError(!name.empty(), CHIP_ERROR_INVALID_ARGUMENT);

    auto statusOpt = ExtractUInt<uint8_t>(json, "Status");
    VerifyOrReturnError(statusOpt.has_value(), CHIP_ERROR_INVALID_ARGUMENT);

    TlvMessageBuffer message(name.size() + 16);

    TLV::TLVType outerType;
    ReturnErrorOnFailure(message.Writer().StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));
    ReturnErrorOnFailure(message.Writer().Put(TLV::ContextTag(1), endpointId));
    ReturnErrorOnFailure(message.Writer().PutString(TLV::ContextTag(2), name.c_str()));
    ReturnErrorOnFailure(message.Writer().Put(TLV::ContextTag(3), *statusOpt));

    if (json.isMember("FabricIndex") && json["FabricIndex"].isIntegral())
    {
        auto fabricIndexOpt = ExtractUInt<FabricIndex>(json, "FabricIndex");
        VerifyOrReturnError(fabricIndexOpt.has_value(), CHIP_ERROR_INVALID_ARGUMENT);
        ReturnErrorOnFailure(message.Writer().Put(TLV::ContextTag(4), *fabricIndexOpt));
    }

    ReturnErrorOnFailure(message.Writer().EndContainer(outerType));

    ByteSpan payload;
    ReturnErrorOnFailure(message.Finalize(payload));
    return registry.HandleAction("AddAmbientSensingNonMatterContributor"_span, payload);
}

CHIP_ERROR AmbientContextTranslator::TranslateRemoveAmbientSensingNonMatterContributor(EndpointId endpointId,
                                                                                       const Json::Value & json,
                                                                                       OOBAccessorRegistry & registry) const
{
    VerifyOrReturnError(json.isMember("ContributorName") && json["ContributorName"].isString(), CHIP_ERROR_INVALID_ARGUMENT);
    std::string name = json["ContributorName"].asString();
    VerifyOrReturnError(!name.empty(), CHIP_ERROR_INVALID_ARGUMENT);

    return DispatchStringAction(registry, "RemoveAmbientSensingNonMatterContributor"_span, endpointId,
                                CharSpan::fromCharString(name.c_str()));
}

CHIP_ERROR AmbientContextTranslator::TranslateUpdateAmbientSensingContributorStatus(EndpointId endpointId, const Json::Value & json,
                                                                                    OOBAccessorRegistry & registry) const
{
    auto statusOpt = ExtractUInt<uint8_t>(json, "Status");
    VerifyOrReturnError(statusOpt.has_value(), CHIP_ERROR_INVALID_ARGUMENT);

    bool hasMatter    = json.isMember("NodeId");
    bool hasNonMatter = json.isMember("ContributorName") && json["ContributorName"].isString();
    VerifyOrReturnError(hasMatter || hasNonMatter, CHIP_ERROR_INVALID_ARGUMENT);

    // Estimate: nodeId(8) + contributorEndpointId(4) + status(3) + name(128)
    TlvMessageBuffer message(150);

    TLV::TLVType outerType;
    ReturnErrorOnFailure(message.Writer().StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));
    ReturnErrorOnFailure(message.Writer().Put(TLV::ContextTag(1), endpointId));
    ReturnErrorOnFailure(message.Writer().Put(TLV::ContextTag(2), *statusOpt));

    if (hasMatter)
    {
        NodeId nodeId = ParseNodeIdFromJson(json);
        VerifyOrReturnError(nodeId != kUndefinedNodeId, CHIP_ERROR_INVALID_ARGUMENT);

        auto contributorEndpointIdOpt = ExtractUInt<EndpointId>(json, "ContributorEndpointId");
        VerifyOrReturnError(contributorEndpointIdOpt.has_value(), CHIP_ERROR_INVALID_ARGUMENT);

        ReturnErrorOnFailure(message.Writer().Put(TLV::ContextTag(3), nodeId));
        ReturnErrorOnFailure(message.Writer().Put(TLV::ContextTag(4), *contributorEndpointIdOpt));
    }
    else
    {
        std::string name = json["ContributorName"].asString();
        VerifyOrReturnError(!name.empty(), CHIP_ERROR_INVALID_ARGUMENT);
        ReturnErrorOnFailure(message.Writer().PutString(TLV::ContextTag(5), name.c_str()));
    }

    ReturnErrorOnFailure(message.Writer().EndContainer(outerType));

    ByteSpan payload;
    ReturnErrorOnFailure(message.Finalize(payload));
    return registry.HandleAction("UpdateAmbientSensingContributorStatus"_span, payload);
}

CHIP_ERROR AmbientContextTranslator::TranslateSetAmbientSensingUnionName(EndpointId endpointId, const Json::Value & json,
                                                                         OOBAccessorRegistry & registry) const
{
    VerifyOrReturnError(json.isMember("UnionName") && json["UnionName"].isString(), CHIP_ERROR_INVALID_ARGUMENT);
    std::string name = json["UnionName"].asString();
    VerifyOrReturnError(!name.empty(), CHIP_ERROR_INVALID_ARGUMENT);

    return DispatchStringAction(registry, "SetAmbientSensingUnionName"_span, endpointId, CharSpan::fromCharString(name.c_str()));
}

} // namespace chip::app::NamedPipe
