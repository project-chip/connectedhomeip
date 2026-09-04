/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <oob-accessors/InMemoryOOBAccessorRegistry.h>
#include <oob-accessors/OOBAccessor.h>
#include <posix/named_pipe/CommandTranslator.h>
#include <posix/named_pipe/Dispatcher.h>
#include <posix/named_pipe/translators/AmbientContextTranslator.h>
#include <posix/named_pipe/translators/BasicInformationTranslator.h>
#include <posix/named_pipe/translators/BooleanStateTranslator.h>
#include <posix/named_pipe/translators/ElectricalEnergyMeasurementTranslator.h>
#include <posix/named_pipe/translators/OccupancyTranslator.h>
#include <posix/named_pipe/translators/OnOffTranslator.h>
#include <posix/named_pipe/translators/RvcTranslator.h>

#include <lib/core/CHIPError.h>
#include <lib/core/TLV.h>
#include <lib/support/Span.h>
#include <pw_unit_test/framework.h>

#include <json/json.h>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

using namespace chip;
using namespace chip::app;
using namespace chip::app::NamedPipe;

namespace {

class MockOOBAccessor : public OOBAccessor
{
public:
    std::optional<CHIP_ERROR> HandleAction(CharSpan action, ByteSpan tlvData) override
    {
        mLastAction = std::string(action.data(), action.size());
        mLastData.assign(tlvData.begin(), tlvData.end());
        return mResultToReturn;
    }

    std::string mLastAction;
    std::vector<uint8_t> mLastData;
    std::optional<CHIP_ERROR> mResultToReturn = CHIP_NO_ERROR;
};

Json::Value ParseJson(const std::string & jsonStr)
{
    Json::Value root;
    Json::CharReaderBuilder builder;
    std::string errs;
    std::istringstream stream(jsonStr);
    EXPECT_TRUE(Json::parseFromStream(builder, stream, &root, &errs));
    return root;
}

} // namespace

class TestNamedPipeTranslators : public ::testing::Test
{
protected:
    void SetUp() override
    {
        mRegistry.Clear();
        auto mock     = std::make_unique<MockOOBAccessor>();
        mMockAccessor = mock.get();
        EXPECT_EQ(mRegistry.Register(std::move(mock)), CHIP_NO_ERROR);
    }

    void TearDown() override
    {
        mRegistry.Clear();
        mMockAccessor = nullptr;
    }

    InMemoryOOBAccessorRegistry mRegistry;
    MockOOBAccessor * mMockAccessor = nullptr;
};

TEST_F(TestNamedPipeTranslators, CommandTranslator_ExtractBool)
{
    Json::Value json;
    json["bool_true"]  = true;
    json["bool_false"] = false;
    json["int_1"]      = 1;
    json["int_0"]      = 0;
    json["int_neg"]    = -1;
    json["int_2"]      = 2;
    json["str_val"]    = "true";

    EXPECT_EQ(CommandTranslator::ExtractBool(json, "bool_true"), std::make_optional(true));
    EXPECT_EQ(CommandTranslator::ExtractBool(json, "bool_false"), std::make_optional(false));
    EXPECT_EQ(CommandTranslator::ExtractBool(json, "int_1"), std::make_optional(true));
    EXPECT_EQ(CommandTranslator::ExtractBool(json, "int_0"), std::make_optional(false));
    EXPECT_EQ(CommandTranslator::ExtractBool(json, "int_neg"), std::make_optional(true));
    EXPECT_EQ(CommandTranslator::ExtractBool(json, "int_2"), std::make_optional(true));
    EXPECT_EQ(CommandTranslator::ExtractBool(json, "str_val"), std::nullopt);
    EXPECT_EQ(CommandTranslator::ExtractBool(json, "missing"), std::nullopt);
}

TEST_F(TestNamedPipeTranslators, CommandTranslator_ExtractUInt)
{
    Json::Value json;
    json["uint8_ok"]    = 200;
    json["uint8_max"]   = 255;
    json["uint8_over"]  = 256;
    json["uint8_neg"]   = -5;
    json["uint16_ok"]   = 50000;
    json["uint16_over"] = 70000;
    json["uint32_ok"]   = Json::UInt64(3000000000ULL);
    json["str_val"]     = "123";

    EXPECT_EQ(CommandTranslator::ExtractUInt<uint8_t>(json, "uint8_ok"), std::make_optional<uint8_t>(200));
    EXPECT_EQ(CommandTranslator::ExtractUInt<uint8_t>(json, "uint8_max"), std::make_optional<uint8_t>(255));
    EXPECT_EQ(CommandTranslator::ExtractUInt<uint8_t>(json, "uint8_over"), std::nullopt);
    EXPECT_EQ(CommandTranslator::ExtractUInt<uint8_t>(json, "uint8_neg"), std::nullopt);

    EXPECT_EQ(CommandTranslator::ExtractUInt<uint16_t>(json, "uint16_ok"), std::make_optional<uint16_t>(50000));
    EXPECT_EQ(CommandTranslator::ExtractUInt<uint16_t>(json, "uint16_over"), std::nullopt);

    EXPECT_EQ(CommandTranslator::ExtractUInt<uint32_t>(json, "uint32_ok"), std::make_optional<uint32_t>(3000000000U));
    EXPECT_EQ(CommandTranslator::ExtractUInt<uint32_t>(json, "str_val"), std::nullopt);
    EXPECT_EQ(CommandTranslator::ExtractUInt<uint32_t>(json, "missing"), std::nullopt);
}

TEST_F(TestNamedPipeTranslators, OnOffTranslator)
{
    OnOffTranslator translator;
    auto names = translator.GetActionNames();
    EXPECT_EQ(names.size(), 1U);
    EXPECT_TRUE(names[0].data_equal("SetOnOff"_span));

    // Valid boolean OnOff
    Json::Value validBool = ParseJson(R"({"Name": "SetOnOff", "OnOff": true})");
    EXPECT_EQ(translator.TranslateAndExecute(1, validBool, mRegistry), CHIP_NO_ERROR);
    EXPECT_EQ(mMockAccessor->mLastAction, "SetOnOff");

    // Valid integer OnOff
    Json::Value validInt = ParseJson(R"({"Name": "SetOnOff", "OnOff": 0})");
    EXPECT_EQ(translator.TranslateAndExecute(1, validInt, mRegistry), CHIP_NO_ERROR);
    EXPECT_EQ(mMockAccessor->mLastAction, "SetOnOff");

    // Missing OnOff field
    Json::Value missing = ParseJson(R"({"Name": "SetOnOff"})");
    EXPECT_EQ(translator.TranslateAndExecute(1, missing, mRegistry), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(TestNamedPipeTranslators, OccupancyTranslator)
{
    OccupancyTranslator translator;
    auto names = translator.GetActionNames();
    EXPECT_EQ(names.size(), 2U);

    // SetOccupancy
    Json::Value occJson = ParseJson(R"({"Name": "SetOccupancy", "Occupancy": true})");
    EXPECT_EQ(translator.TranslateAndExecute(1, occJson, mRegistry), CHIP_NO_ERROR);
    EXPECT_EQ(mMockAccessor->mLastAction, "SetOccupancy");

    // SetHoldTime
    Json::Value holdJson = ParseJson(R"({"Name": "SetHoldTime", "HoldTime": 120})");
    EXPECT_EQ(translator.TranslateAndExecute(1, holdJson, mRegistry), CHIP_NO_ERROR);
    EXPECT_EQ(mMockAccessor->mLastAction, "SetHoldTime");

    // Invalid HoldTime
    Json::Value invalidHold = ParseJson(R"({"Name": "SetHoldTime", "HoldTime": -1})");
    EXPECT_EQ(translator.TranslateAndExecute(1, invalidHold, mRegistry), CHIP_ERROR_INVALID_ARGUMENT);

    // Unknown action
    Json::Value unknown = ParseJson(R"({"Name": "UnknownAction"})");
    EXPECT_EQ(translator.TranslateAndExecute(1, unknown, mRegistry), CHIP_ERROR_NOT_FOUND);
}

TEST_F(TestNamedPipeTranslators, BooleanStateTranslator)
{
    BooleanStateTranslator translator;

    // SetBooleanState with StateValue
    Json::Value json1 = ParseJson(R"({"Name": "SetBooleanState", "StateValue": true})");
    EXPECT_EQ(translator.TranslateAndExecute(1, json1, mRegistry), CHIP_NO_ERROR);
    EXPECT_EQ(mMockAccessor->mLastAction, "SetBooleanState");

    // SetBooleanState with NewState
    Json::Value json2 = ParseJson(R"({"Name": "SetBooleanState", "NewState": false})");
    EXPECT_EQ(translator.TranslateAndExecute(1, json2, mRegistry), CHIP_NO_ERROR);

    // SetStateValue with BooleanState
    Json::Value json3 = ParseJson(R"({"Name": "SetStateValue", "BooleanState": 1})");
    EXPECT_EQ(translator.TranslateAndExecute(1, json3, mRegistry), CHIP_NO_ERROR);

    // Missing state field
    Json::Value invalid = ParseJson(R"({"Name": "SetBooleanState"})");
    EXPECT_EQ(translator.TranslateAndExecute(1, invalid, mRegistry), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(TestNamedPipeTranslators, BasicInformationTranslator)
{
    BasicInformationTranslator translator;

    // IncreaseConfigurationVersion
    Json::Value json1 = ParseJson(R"({"Name": "IncreaseConfigurationVersion"})");
    EXPECT_EQ(translator.TranslateAndExecute(0, json1, mRegistry), CHIP_NO_ERROR);
    EXPECT_EQ(mMockAccessor->mLastAction, "IncreaseConfigurationVersion");

    // SimulateConfigurationVersionChange alias
    Json::Value json2 = ParseJson(R"({"Name": "SimulateConfigurationVersionChange"})");
    EXPECT_EQ(translator.TranslateAndExecute(0, json2, mRegistry), CHIP_NO_ERROR);
    EXPECT_EQ(mMockAccessor->mLastAction, "IncreaseConfigurationVersion");

    // Unknown
    Json::Value unknown = ParseJson(R"({"Name": "Unknown"})");
    EXPECT_EQ(translator.TranslateAndExecute(0, unknown, mRegistry), CHIP_ERROR_NOT_FOUND);
}

TEST_F(TestNamedPipeTranslators, ElectricalEnergyMeasurementTranslator)
{
    ElectricalEnergyMeasurementTranslator translator;

    // GenerateElectricalEnergyMeasurementSnapshots
    Json::Value json = ParseJson(R"({"Name": "GenerateElectricalEnergyMeasurementSnapshots"})");
    EXPECT_EQ(translator.TranslateAndExecute(1, json, mRegistry), CHIP_NO_ERROR);
    EXPECT_EQ(mMockAccessor->mLastAction, "GenerateElectricalEnergyMeasurementSnapshots");
}

TEST_F(TestNamedPipeTranslators, AmbientContextTranslator)
{
    AmbientContextTranslator translator;

    // SetAmbientContextSupport
    Json::Value suppJson = ParseJson(R"({"Name": "SetAmbientContextSupport", "AmbientContextType": [{"TypeId": 75, "TagId": 1}]})");
    EXPECT_EQ(translator.TranslateAndExecute(1, suppJson, mRegistry), CHIP_NO_ERROR);
    EXPECT_EQ(mMockAccessor->mLastAction, "SetAmbientContextSupport");

    // AddAmbientContextDetect
    Json::Value detJson = ParseJson(
        R"({"Name": "AddAmbientContextDetect", "AmbientContextType": [{"TypeId": 75, "TagId": 1}], "DetectionConfidence": 85})");
    EXPECT_EQ(translator.TranslateAndExecute(1, detJson, mRegistry), CHIP_NO_ERROR);
    EXPECT_EQ(mMockAccessor->mLastAction, "AddAmbientContextDetect");

    // SetSensorFusionSupported
    Json::Value fusionJson =
        ParseJson(R"({"Name": "SetSensorFusionSupported", "AmbientContextType": [{"TypeId": 75, "TagId": 1}]})");
    EXPECT_EQ(translator.TranslateAndExecute(1, fusionJson, mRegistry), CHIP_NO_ERROR);
    EXPECT_EQ(mMockAccessor->mLastAction, "SetSensorFusionSupported");

    // SetObjectCount
    Json::Value countJson = ParseJson(R"({"Name": "SetObjectCount", "ObjectCount": 12})");
    EXPECT_EQ(translator.TranslateAndExecute(1, countJson, mRegistry), CHIP_NO_ERROR);
    EXPECT_EQ(mMockAccessor->mLastAction, "SetObjectCount");

    // SetPredictedActivity
    Json::Value predJson = ParseJson(R"({
        "Name": "SetPredictedActivity",
        "PredAct": [
            {
                "AmbientContextType": [{"TypeId": 75, "TagId": 1}],
                "StartTStamp": 100,
                "EndTStamp": 200,
                "Conf": 90,
                "CrowdDetect": true,
                "CrowdCnt": 5
            }
        ]
    })");
    // SetPredictedActivity with multiple nested semantic tags
    Json::Value predMultiJson = ParseJson(R"({
        "Name": "SetPredictedActivity",
        "PredAct": [
            {
                "AmbientContextType": [
                    {"TypeId": 75, "TagId": 1},
                    {"TypeId": 75, "TagId": 2},
                    {"TypeId": 75, "TagId": 3},
                    {"TypeId": 75, "TagId": 4},
                    {"TypeId": 75, "TagId": 5},
                    {"TypeId": 75, "TagId": 6},
                    {"TypeId": 75, "TagId": 7},
                    {"TypeId": 75, "TagId": 8}
                ],
                "StartTStamp": 100,
                "EndTStamp": 200,
                "Conf": 90,
                "CrowdDetect": true,
                "CrowdCnt": 5
            }
        ]
    })");
    EXPECT_EQ(translator.TranslateAndExecute(1, predMultiJson, mRegistry), CHIP_NO_ERROR);
    EXPECT_EQ(mMockAccessor->mLastAction, "SetPredictedActivity");

    // Unknown action
    Json::Value unknown = ParseJson(R"({"Name": "Unknown"})");
    EXPECT_EQ(translator.TranslateAndExecute(1, unknown, mRegistry), CHIP_ERROR_NOT_FOUND);
}

TEST_F(TestNamedPipeTranslators, RvcTranslator)
{
    RvcTranslator translator;

    // Parameterless actions
    Json::Value resetJson = ParseJson(R"({"Name": "Reset"})");
    EXPECT_EQ(translator.TranslateAndExecute(1, resetJson, mRegistry), CHIP_NO_ERROR);
    EXPECT_EQ(mMockAccessor->mLastAction, "Reset");

    Json::Value dockedJson = ParseJson(R"({"Name": "Docked"})");
    EXPECT_EQ(translator.TranslateAndExecute(1, dockedJson, mRegistry), CHIP_NO_ERROR);
    EXPECT_EQ(mMockAccessor->mLastAction, "Docked");

    // ErrorEvent
    Json::Value errJson = ParseJson(R"({"Name": "ErrorEvent", "Error": "DustBinFull"})");
    EXPECT_EQ(translator.TranslateAndExecute(1, errJson, mRegistry), CHIP_NO_ERROR);
    EXPECT_EQ(mMockAccessor->mLastAction, "ErrorEvent");

    // AddMap
    Json::Value addMapJson = ParseJson(R"({"Name": "AddMap", "MapId": 1, "MapName": "LivingRoom"})");
    EXPECT_EQ(translator.TranslateAndExecute(1, addMapJson, mRegistry), CHIP_NO_ERROR);
    EXPECT_EQ(mMockAccessor->mLastAction, "AddMap");

    // RemoveMap
    Json::Value remMapJson = ParseJson(R"({"Name": "RemoveMap", "MapId": 1})");
    EXPECT_EQ(translator.TranslateAndExecute(1, remMapJson, mRegistry), CHIP_NO_ERROR);
    EXPECT_EQ(mMockAccessor->mLastAction, "RemoveMap");

    // AddArea
    Json::Value addAreaJson = ParseJson(R"({"Name": "AddArea", "AreaId": 10, "MapId": 1, "LocationName": "Couch"})");
    EXPECT_EQ(translator.TranslateAndExecute(1, addAreaJson, mRegistry), CHIP_NO_ERROR);
    EXPECT_EQ(mMockAccessor->mLastAction, "AddArea");

    // RemoveArea
    Json::Value remAreaJson = ParseJson(R"({"Name": "RemoveArea", "AreaId": 10})");
    EXPECT_EQ(translator.TranslateAndExecute(1, remAreaJson, mRegistry), CHIP_NO_ERROR);
    EXPECT_EQ(mMockAccessor->mLastAction, "RemoveArea");

    // Unknown action
    Json::Value unknown = ParseJson(R"({"Name": "UnknownAction"})");
    EXPECT_EQ(translator.TranslateAndExecute(1, unknown, mRegistry), CHIP_ERROR_NOT_FOUND);
}

TEST_F(TestNamedPipeTranslators, Dispatcher_DispatchJson)
{
    Dispatcher dispatcher(mRegistry);
    EXPECT_EQ(dispatcher.EnsureTranslatorRegistered<OnOffTranslator>(), CHIP_NO_ERROR);

    // Valid action on explicit endpoint
    Json::Value valid = ParseJson(R"({"Name": "SetOnOff", "EndpointId": 1, "OnOff": true})");
    EXPECT_EQ(dispatcher.DispatchJson(valid), CHIP_NO_ERROR);
    EXPECT_EQ(mMockAccessor->mLastAction, "SetOnOff");

    // Valid action with default endpoint (0)
    Json::Value defEp = ParseJson(R"({"Name": "SetOnOff", "OnOff": false})");
    EXPECT_EQ(dispatcher.DispatchJson(defEp), CHIP_NO_ERROR);

    // Invalid JSON structure (not object)
    Json::Value arrayVal(Json::arrayValue);
    EXPECT_EQ(dispatcher.DispatchJson(arrayVal), CHIP_ERROR_INVALID_ARGUMENT);

    // Missing Name
    Json::Value missingName = ParseJson(R"({"OnOff": true})");
    EXPECT_EQ(dispatcher.DispatchJson(missingName), CHIP_ERROR_INVALID_ARGUMENT);

    // Non-string Name
    Json::Value intName = ParseJson(R"({"Name": 123})");
    EXPECT_EQ(dispatcher.DispatchJson(intName), CHIP_ERROR_INVALID_ARGUMENT);

    // Unknown action
    Json::Value unkAction = ParseJson(R"({"Name": "UnknownAction"})");
    EXPECT_EQ(dispatcher.DispatchJson(unkAction), CHIP_ERROR_NOT_FOUND);
}

TEST_F(TestNamedPipeTranslators, TlvMessageBuffer_LifecycleAndSizing)
{
    // Verify default sizing adds envelope overhead
    TlvMessageBuffer buffer0;
    EXPECT_GE(TlvMessageBuffer::kTlvEnvelopeOverhead, 64U);

    // Verify writing and finalize
    TlvMessageBuffer buffer(128);
    TLV::TLVType outerType;
    EXPECT_EQ(buffer.Writer().StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType), CHIP_NO_ERROR);
    EXPECT_EQ(buffer.Writer().Put(TLV::ContextTag(1), static_cast<uint16_t>(5)), CHIP_NO_ERROR);
    EXPECT_EQ(buffer.Writer().EndContainer(outerType), CHIP_NO_ERROR);

    ByteSpan payload;
    EXPECT_EQ(buffer.Finalize(payload), CHIP_NO_ERROR);
    EXPECT_GT(payload.size(), 0U);
}

TEST_F(TestNamedPipeTranslators, CommandTranslator_DispatchStringAction_LargePayload)
{
    // Large string exceeding previous fixed stack buffers (1024 bytes)
    std::string largeError(1024, 'E');
    EXPECT_EQ(
        CommandTranslator::DispatchStringAction(mRegistry, "ErrorEvent"_span, 1, CharSpan(largeError.data(), largeError.size())),
        CHIP_NO_ERROR);
    EXPECT_EQ(mMockAccessor->mLastAction, "ErrorEvent");
    EXPECT_GT(mMockAccessor->mLastData.size(), 1024U);
}
