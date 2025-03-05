/*
 *   Copyright (c) 2025 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#pragma once

#include <app/tests/suites/commands/interaction_model/InteractionModel.h>

#include "DataModelLogger.h"
#include "ModelCommand.h"

inline constexpr char kWriteCommandKey[]      = "write";
inline constexpr char kWriteByIdCommandKey[]  = "write-by-id";
inline constexpr char kForceWriteCommandKey[] = "force-write";

enum class WriteCommandType
{
    kWrite,      // regular, writable attributes
    kForceWrite, // forced writes, send a write command on something expected to fail
};

template <class T = std::vector<CustomArgument *>>
class WriteAttribute : public InteractionModelWriter, public ModelCommand, public chip::app::WriteClient::Callback
{
public:
    WriteAttribute(CredentialIssuerCommands * credsIssuerConfig) :
        InteractionModelWriter(this), ModelCommand(kWriteByIdCommandKey, credsIssuerConfig)
    {
        AddArgumentClusterIds();
        AddArgumentAttributeIds();
        AddArgumentAttributeValues();
        AddArguments();
    }

    WriteAttribute(chip::ClusterId clusterId, CredentialIssuerCommands * credsIssuerConfig) :
        InteractionModelWriter(this), ModelCommand(kWriteByIdCommandKey, credsIssuerConfig), mClusterIds(1, clusterId)
    {
        AddArgumentAttributeIds();
        AddArgumentAttributeValues();
        AddArguments();
    }

    template <typename minType, typename maxType>
    WriteAttribute(chip::ClusterId clusterId, const char * attributeName, minType minValue, maxType maxValue,
                   chip::AttributeId attributeId, WriteCommandType commandType, CredentialIssuerCommands * credsIssuerConfig) :
        WriteAttribute(clusterId, attributeId, commandType, credsIssuerConfig)
    {
        AddArgumentAttributeName(attributeName);
        AddArgumentAttributeValues(static_cast<int64_t>(minValue), static_cast<uint64_t>(maxValue));
        AddArguments();
    }

    WriteAttribute(chip::ClusterId clusterId, const char * attributeName, float minValue, float maxValue,
                   chip::AttributeId attributeId, WriteCommandType commandType, CredentialIssuerCommands * credsIssuerConfig) :
        WriteAttribute(clusterId, attributeId, commandType, credsIssuerConfig)
    {
        AddArgumentAttributeName(attributeName);
        AddArgumentAttributeValues(minValue, maxValue);
        AddArguments();
    }

    WriteAttribute(chip::ClusterId clusterId, const char * attributeName, double minValue, double maxValue,
                   chip::AttributeId attributeId, WriteCommandType commandType, CredentialIssuerCommands * credsIssuerConfig) :
        WriteAttribute(clusterId, attributeId, commandType, credsIssuerConfig)
    {
        AddArgumentAttributeName(attributeName);
        AddArgumentAttributeValues(minValue, maxValue);
        AddArguments();
    }

    WriteAttribute(chip::ClusterId clusterId, const char * attributeName, chip::AttributeId attributeId,
                   WriteCommandType commandType, CredentialIssuerCommands * credsIssuerConfig) :
        WriteAttribute(clusterId, attributeId, commandType, credsIssuerConfig)
    {
        AddArgumentAttributeName(attributeName);
        AddArgumentAttributeValues();
        AddArguments();
    }

    WriteAttribute(chip::ClusterId clusterId, const char * attributeName, chip::AttributeId attributeId,
                   TypedComplexArgument<T> & attributeParser, WriteCommandType commandType,
                   CredentialIssuerCommands * credsIssuerConfig) :
        WriteAttribute(clusterId, attributeId, commandType, credsIssuerConfig)
    {
        AddArgumentAttributeName(attributeName);
        AddArgumentAttributeValues(attributeParser);
        AddArguments();
    }

    ~WriteAttribute() {}

    CHIP_ERROR SendCommand(chip::DeviceProxy * device, std::vector<chip::EndpointId> endpointIds) override
    {
        return WriteAttribute::SendCommand(device, endpointIds, mClusterIds, mAttributeIds, mAttributeValues);
    }

    CHIP_ERROR SendGroupCommand(chip::GroupId groupId, chip::FabricIndex fabricIndex) override
    {
        return WriteAttribute::SendGroupCommand(groupId, fabricIndex, mClusterIds, mAttributeIds, mAttributeValues);
    }

    /////////// WriteClient Callback Interface /////////
    void OnResponse(const chip::app::WriteClient * client, const chip::app::ConcreteDataAttributePath & path,
                    chip::app::StatusIB status) override
    {
        CHIP_ERROR error = status.ToChipError();
        if (CHIP_NO_ERROR != error)
        {
            LogErrorOnFailure(RemoteDataModelLogger::LogErrorAsJSON(path, status));

            ChipLogError(NotSpecified, "Response Failure: %s", chip::ErrorStr(error));
            mError = error;
        }
    }

    void OnError(const chip::app::WriteClient * client, CHIP_ERROR error) override
    {
        LogErrorOnFailure(RemoteDataModelLogger::LogErrorAsJSON(error));

        ChipLogProgress(NotSpecified, "Error: %s", chip::ErrorStr(error));
        mError = error;
    }

    void OnDone(chip::app::WriteClient * client) override
    {
        InteractionModelWriter::Shutdown();
        SetCommandExitStatus(mError);
    }

    CHIP_ERROR SendCommand(chip::DeviceProxy * device, std::vector<chip::EndpointId> endpointIds,
                           std::vector<chip::ClusterId> clusterIds, std::vector<chip::AttributeId> attributeIds, const T & values)
    {
        return InteractionModelWriter::WriteAttribute(device, endpointIds, clusterIds, attributeIds, values);
    }

    CHIP_ERROR SendGroupCommand(chip::GroupId groupId, chip::FabricIndex fabricIndex, std::vector<chip::ClusterId> clusterIds,
                                std::vector<chip::AttributeId> attributeIds, const T & value)
    {
        ChipLogDetail(NotSpecified, "Sending Write Attribute to Group %u, on Fabric %x, for cluster %u with attributeId %u",
                      groupId, fabricIndex, clusterIds.at(0), attributeIds.at(0));
        chip::Optional<chip::DataVersion> dataVersion = chip::NullOptional;
        if (mDataVersions.HasValue())
        {
            dataVersion.SetValue(mDataVersions.Value().at(0));
        }

        return InteractionModelWriter::WriteGroupAttribute(groupId, fabricIndex, clusterIds.at(0), attributeIds.at(0), value,
                                                           dataVersion);
    }

    void Shutdown() override
    {
        mError = CHIP_NO_ERROR;
        ModelCommand::Shutdown();
    }

protected:
    WriteAttribute(const char * attributeName, CredentialIssuerCommands * credsIssuerConfig) :
        InteractionModelWriter(this), ModelCommand(kWriteCommandKey, credsIssuerConfig)
    {
        // Subclasses are responsible for calling AddArguments.
    }

    void AddArgumentClusterIds()
    {
        AddArgument("cluster-ids", 0, UINT32_MAX, &mClusterIds,
                    "Comma-separated list of cluster ids to write to (e.g. \"6\" or \"6,0x201\").");
    }

    void AddArgumentAttributeIds()
    {
        AddArgument("attribute-ids", 0, UINT32_MAX, &mAttributeIds,
                    "Comma-separated list of attribute ids to write (e.g. \"16385\" or \"16385,0x4002\").");
    }

    void AddArgumentAttributeName(const char * attributeName)
    {
        AddArgument("attribute-name", attributeName, "The attribute name to write.");
    }

    template <typename U = T, std::enable_if_t<std::is_same<U, std::vector<CustomArgument *>>::value, int> = 0>
    static const char * GetAttributeValuesDescription()
    {
        return "Semicolon-separated list of attribute values to write. Each value is represented as follows, depending on the "
               "type:\n"
               "  * struct: a JSON-encoded object, with field ids as keys.\n"
               "  * list: a JSON-encoded array of values.\n"
               "  * null: A literal null.\n"
               "  * boolean: A literal true or false.\n"
               "  * unsigned integer: One of:\n"
               "      a) The number directly, as decimal.\n"
               "      b) The number directly, as 0x followed by hex digits. (Only for the toplevel value, not inside structs or "
               "lists.)\n"
               "      c) A string starting with \"u:\" followed by decimal digits\n"
               "  * signed integer: One of:\n"
               "      a) The number directly, if it's negative.\n"
               "      c) A string starting with \"s:\" followed by decimal digits\n"
               "  * single-precision float: A string starting with \"f:\" followed by the number.\n"
               "  * double-precision float: One of:\n"
               "      a) The number directly, if it's not an integer.\n"
               "      b) A string starting with \"d:\" followed by the number.\n"
               "  * octet string: A string starting with \"hex:\" followed by the hex encoding of the bytes.\n"
               "  * string: A string with the characters.\n"
               "\n"
               "  Example values: '10;20', '10;\"u:20\"', '\"hex:aabbcc\";\"hello\"'.";
    }

    static const char * GetTypedAttributeValuesDescription() { return "Comma-separated list of attribute values to write."; }

    template <typename U = T, std::enable_if_t<!std::is_same<U, std::vector<CustomArgument *>>::value, int> = 0>
    static const char * GetAttributeValuesDescription()
    {
        return GetTypedAttributeValuesDescription();
    }

    template <typename minType, typename maxType>
    void AddArgumentAttributeValues(minType minValue, maxType maxValue)
    {
        AddArgument("attribute-values", minValue, maxValue, &mAttributeValues, GetTypedAttributeValuesDescription());
    }

    void AddArgumentAttributeValues() { AddArgument("attribute-values", &mAttributeValues, GetAttributeValuesDescription()); }

    void AddArgumentAttributeValues(TypedComplexArgument<T> & attributeParser)
    {
        attributeParser.SetArgument(&mAttributeValues);
        AddArgument("attribute-values", &attributeParser, GetTypedAttributeValuesDescription());
    }

    void AddArguments()
    {
        AddArgument("timedInteractionTimeoutMs", 0, UINT16_MAX, &mTimedInteractionTimeoutMs,
                    "If provided, do a timed write with the given timed interaction timeout. See \"7.6.10. Timed Interaction\" in "
                    "the Matter specification.");
        AddArgument("busyWaitForMs", 0, UINT16_MAX, &mBusyWaitForMs,
                    "If provided, block the main thread processing for the given time right after sending a command.");
        AddArgument("data-version", 0, UINT32_MAX, &mDataVersions,
                    "Comma-separated list of data versions for the clusters being written.");
        AddArgument("suppressResponse", 0, 1, &mSuppressResponse);
        AddArgument("repeat-count", 1, UINT16_MAX, &mRepeatCount);
        AddArgument("repeat-delay-ms", 0, UINT16_MAX, &mRepeatDelayInMs);
        ModelCommand::AddArguments();
    }

private:
    // This constructor is private as it is not intended to be used from outside the class.
    WriteAttribute(chip::ClusterId clusterId, chip::AttributeId attributeId, WriteCommandType commandType,
                   CredentialIssuerCommands * credsIssuerConfig) :
        InteractionModelWriter(this),
        ModelCommand(commandType == WriteCommandType::kWrite ? kWriteCommandKey : kForceWriteCommandKey, credsIssuerConfig),
        mClusterIds(1, clusterId), mAttributeIds(1, attributeId)
    {}

    std::vector<chip::ClusterId> mClusterIds;
    std::vector<chip::AttributeId> mAttributeIds;

    CHIP_ERROR mError = CHIP_NO_ERROR;
    T mAttributeValues;
};

template <class T>
class WriteAttributeAsComplex : public WriteAttribute<T>
{
public:
    WriteAttributeAsComplex(chip::ClusterId clusterId, const char * attributeName, chip::AttributeId attributeId,
                            WriteCommandType commandType, CredentialIssuerCommands * credsIssuerConfig) :
        WriteAttribute<T>(clusterId, attributeName, attributeId, mAttributeParser, commandType, credsIssuerConfig)
    {}

private:
    TypedComplexArgument<T> mAttributeParser;
};
