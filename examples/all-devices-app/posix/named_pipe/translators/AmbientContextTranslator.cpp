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

namespace chip::app {

namespace {

CHIP_ERROR EncodeSemanticTagList(TLV::TLVWriter & writer, TLV::Tag tag, const Json::Value & actArray)
{
    TLV::TLVType arrayType;
    ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Array, arrayType));
    for (Json::ArrayIndex i = 0; i < actArray.size(); i++)
    {
        const Json::Value & item = actArray[i];
        VerifyOrReturnError(item.isObject(), CHIP_ERROR_INVALID_ARGUMENT);

        auto typeIdOpt = NamedPipeCommandTranslator::ExtractUInt<uint8_t>(item, "TypeId");
        auto tagIdOpt  = NamedPipeCommandTranslator::ExtractUInt<uint8_t>(item, "TagId");
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
                                                         OOBAccessorRegistry & registry)
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

    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR AmbientContextTranslator::TranslateSetAmbientContextSupport(EndpointId endpointId, const Json::Value & json,
                                                                      OOBAccessorRegistry & registry) const
{
    VerifyOrReturnError(json.isMember("AmbientContextType") && json["AmbientContextType"].isArray(), CHIP_ERROR_INVALID_ARGUMENT);

    uint8_t buffer[1024];
    TLV::TLVWriter writer;
    writer.Init(buffer, sizeof(buffer));

    TLV::TLVType outerType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(1), endpointId));
    ReturnErrorOnFailure(EncodeSemanticTagList(writer, TLV::ContextTag(2), json["AmbientContextType"]));
    ReturnErrorOnFailure(writer.EndContainer(outerType));
    ReturnErrorOnFailure(writer.Finalize());

    return registry.HandleAction("SetAmbientContextSupport"_span, ByteSpan(buffer, writer.GetLengthWritten()));
}

CHIP_ERROR AmbientContextTranslator::TranslateAddAmbientContextDetect(EndpointId endpointId, const Json::Value & json,
                                                                     OOBAccessorRegistry & registry) const
{
    VerifyOrReturnError(json.isMember("AmbientContextType") && json["AmbientContextType"].isArray(), CHIP_ERROR_INVALID_ARGUMENT);

    uint8_t buffer[1024];
    TLV::TLVWriter writer;
    writer.Init(buffer, sizeof(buffer));

    TLV::TLVType outerType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(1), endpointId));
    ReturnErrorOnFailure(EncodeSemanticTagList(writer, TLV::ContextTag(2), json["AmbientContextType"]));

    if (json.isMember("DetectionConfidence"))
    {
        auto confOpt = ExtractUInt<uint8_t>(json, "DetectionConfidence");
        VerifyOrReturnError(confOpt.has_value(), CHIP_ERROR_INVALID_ARGUMENT);
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(3), *confOpt));
    }

    ReturnErrorOnFailure(writer.EndContainer(outerType));
    ReturnErrorOnFailure(writer.Finalize());

    return registry.HandleAction("AddAmbientContextDetect"_span, ByteSpan(buffer, writer.GetLengthWritten()));
}

CHIP_ERROR AmbientContextTranslator::TranslateSetPredictedActivity(EndpointId endpointId, const Json::Value & json,
                                                                   OOBAccessorRegistry & registry) const
{
    VerifyOrReturnError(json.isMember("PredAct") && json["PredAct"].isArray(), CHIP_ERROR_INVALID_ARGUMENT);
    const Json::Value & predActArray = json["PredAct"];

    uint8_t buffer[2048];
    TLV::TLVWriter writer;
    writer.Init(buffer, sizeof(buffer));

    TLV::TLVType outerType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(1), endpointId));

    TLV::TLVType arrayType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::ContextTag(2), TLV::kTLVType_Array, arrayType));
    for (Json::ArrayIndex i = 0; i < predActArray.size(); i++)
    {
        const Json::Value & item = predActArray[i];
        VerifyOrReturnError(item.isObject() && item.isMember("AmbientContextType") && item["AmbientContextType"].isArray(),
                            CHIP_ERROR_INVALID_ARGUMENT);

        auto startTStampOpt = ExtractUInt<uint32_t>(item, "StartTStamp");
        auto endTStampOpt   = ExtractUInt<uint32_t>(item, "EndTStamp");
        auto confOpt        = ExtractUInt<uint8_t>(item, "Conf");
        VerifyOrReturnError(startTStampOpt.has_value() && endTStampOpt.has_value() && confOpt.has_value(),
                            CHIP_ERROR_INVALID_ARGUMENT);

        TLV::TLVType itemType;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, itemType));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(1), *startTStampOpt));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(2), *endTStampOpt));
        ReturnErrorOnFailure(EncodeSemanticTagList(writer, TLV::ContextTag(3), item["AmbientContextType"]));

        if (item.isMember("CrowdDetect"))
        {
            auto crowdDetectOpt = ExtractBool(item, "CrowdDetect");
            VerifyOrReturnError(crowdDetectOpt.has_value(), CHIP_ERROR_INVALID_ARGUMENT);
            ReturnErrorOnFailure(writer.Put(TLV::ContextTag(4), *crowdDetectOpt));
        }
        if (item.isMember("CrowdCnt"))
        {
            auto crowdCntOpt = ExtractUInt<uint8_t>(item, "CrowdCnt");
            VerifyOrReturnError(crowdCntOpt.has_value(), CHIP_ERROR_INVALID_ARGUMENT);
            ReturnErrorOnFailure(writer.Put(TLV::ContextTag(5), *crowdCntOpt));
        }
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(6), *confOpt));
        ReturnErrorOnFailure(writer.EndContainer(itemType));
    }
    ReturnErrorOnFailure(writer.EndContainer(arrayType));
    ReturnErrorOnFailure(writer.EndContainer(outerType));
    ReturnErrorOnFailure(writer.Finalize());

    return registry.HandleAction("SetPredictedActivity"_span, ByteSpan(buffer, writer.GetLengthWritten()));
}

CHIP_ERROR AmbientContextTranslator::TranslateSetSensorFusionSupported(EndpointId endpointId, const Json::Value & json,
                                                                     OOBAccessorRegistry & registry) const
{
    VerifyOrReturnError(json.isMember("AmbientContextType") && json["AmbientContextType"].isArray(), CHIP_ERROR_INVALID_ARGUMENT);

    uint8_t buffer[1024];
    TLV::TLVWriter writer;
    writer.Init(buffer, sizeof(buffer));

    TLV::TLVType outerType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(1), endpointId));
    ReturnErrorOnFailure(EncodeSemanticTagList(writer, TLV::ContextTag(2), json["AmbientContextType"]));
    ReturnErrorOnFailure(writer.EndContainer(outerType));
    ReturnErrorOnFailure(writer.Finalize());

    return registry.HandleAction("SetSensorFusionSupported"_span, ByteSpan(buffer, writer.GetLengthWritten()));
}

CHIP_ERROR AmbientContextTranslator::TranslateSetObjectCount(EndpointId endpointId, const Json::Value & json,
                                                           OOBAccessorRegistry & registry) const
{
    auto objectCount = ExtractUInt<uint16_t>(json, "ObjectCount");
    VerifyOrReturnError(objectCount.has_value(), CHIP_ERROR_INVALID_ARGUMENT);

    return DispatchAction(registry, "SetObjectCount"_span, endpointId, *objectCount);
}

} // namespace chip::app
