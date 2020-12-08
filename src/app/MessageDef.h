/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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
/**
 *    @file
 *      This file defines CHIP interaction model message parsers and encoders
 *      Interaction model profile.
 *
 */

#pragma once

#ifndef _CHIP_INTERACTION_MODEL_MESSAGE_DEF_H
#define _CHIP_INTERACTION_MODEL_MESSAGE_DEF_H

#include <core/CHIPCore.h>
#include <core/CHIPTLV.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>
#include <util/basic-types.h>

namespace chip {
namespace app {
/**
 *  @brief
 *    The CHIP interaction model message types.
 *
 *  These values are called out in CHIP Interaction Model: Encoding Specification
 *
 */
enum
{
    kMsgType_SubscribeRequest      = 0x01,
    kMsgType_ReadRequest           = 0x02,
    kMsgType_ReportData            = 0x03,
    kMsgType_WriteRequest          = 0x04,
    kMsgType_WriteResponse         = 0x05,
    kMsgType_InvokeCommandRequest  = 0x06,
    kMsgType_InvokeCommandResponse = 0x07,
};

class Parser
{
public:
    /**
     *  @brief Initialize the Builder object with TLVReader and ContainerType
     *
     *  @param [in] aReader TLVReader
     *  @param [in] aOuterContainerType outer container type
     *
     */
    void Init(const chip::TLV::TLVReader & aReader, chip::TLV::TLVType aOuterContainerType);

    /**
     *  @brief Initialize a TLVReader to point to the beginning of any tagged element in this request
     *
     *  @param [in]  aTagToFind Tag to find in the request
     *  @param [out] apReader   A pointer to TLVReader, which will be initialized at the specified TLV element
     *                          on success
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR GetReaderOnTag(const uint64_t aTagToFind, chip::TLV::TLVReader * const apReader) const;

    /**
     *  @brief Get the TLV Reader
     *
     *  @param [in] aReader A pointer to a TLVReader
     *
     */
    void GetReader(chip::TLV::TLVReader * const apReader);

protected:
    chip::TLV::TLVReader mReader;
    chip::TLV::TLVType mOuterContainerType;
    Parser();

    template <typename T>
    CHIP_ERROR GetUnsignedInteger(const uint8_t aContextTag, T * const apLValue) const;

    template <typename T>
    CHIP_ERROR GetSimpleValue(const uint8_t aContextTag, const chip::TLV::TLVType aTLVType, T * const apLValue) const;
};

class ListParser : public chip::app::Parser
{
protected:
    ListParser();

public:
    /**
     *  @brief Initialize the parser object with TLVReader
     *
     *  @param [in] aReader A pointer to a TLVReader, which should be on the element of the array element
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR Init(const chip::TLV::TLVReader & aReader);

    /**
     *  @brief Initialize the parser object with TLVReader if context tag exists
     *
     *  @param [in] aReader A pointer to a TLVReader, which should be on the element of the array element
     *  @param [in] aContextTagToFind A context tag it tries to find
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR InitIfPresent(const chip::TLV::TLVReader & aReader, const uint8_t aContextTagToFind);

    /**
     *  @brief Iterate to next element
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR Next();
};

class Builder
{
public:
    /**
     *  @brief Initialize the Builder object with TLVWriter and ContainerType
     *
     *  @param [in] apWriter A pointer to a TLVWriter
     *  @param [in] aOuterContainerType outer container type
     *
     */
    void Init(chip::TLV::TLVWriter * const apWriter, chip::TLV::TLVType aOuterContainerType);

    /**
     *  @brief Reset the Error
     *
     */
    void ResetError();

    /**
     *  @brief Reset the Error with particular aErr.
     *  @param [in] aErr the Error it would be reset with
     *
     */
    void ResetError(CHIP_ERROR aErr);

    /**
     *  @brief Get current error
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR GetError() const { return mError; };

    /**
     *  @brief Get TLV Writer
     *
     *  @return #Pointer to the TLVWriter
     */
    chip::TLV::TLVWriter * GetWriter() { return mpWriter; };

protected:
    CHIP_ERROR mError;
    chip::TLV::TLVWriter * mpWriter;
    chip::TLV::TLVType mOuterContainerType;

    Builder();
    void EndOfContainer();

    CHIP_ERROR InitAnonymousStructure(chip::TLV::TLVWriter * const apWriter);
};

class ListBuilder : public chip::app::Builder
{
protected:
    ListBuilder();

public:
    /**
     * Init the TLV array container with an particular context tag.
     * Required to implement arrays of arrays, and to test ListBuilder.
     *
     * @param[in]   apWriter    Pointer to the TLVWriter that is encoding the message.
     * @param[in]   aContextTagToUse    A contextTag to use.
     *
     * @return                  CHIP_ERROR codes returned by Chip::TLV objects.
     */

    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter, const uint8_t aContextTagToUse);
    /**
     * Init the TLV array container with an anonymous tag.
     * Required to implement arrays of arrays, and to test ListBuilder.
     *
     * @param[in]   apWriter    Pointer to the TLVWriter that is encoding the message.
     *
     * @return                  CHIP_ERROR codes returned by Chip::TLV objects.
     */
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter);
};

namespace AttributePath {
enum
{
    kCsTag_NodeId              = 0,
    kCsTag_EndpointId          = 1,
    kCsTag_NamespacedClusterId = 2,
    kCsTag_FieldId             = 3,
    kCsTag_ListIndex           = 4,
};

class Parser : public chip::app::Parser
{
public:
    /**
     *  @brief Initialize the parser object with TLVReader
     *
     *  @param [in] aReader A pointer to a TLVReader, which should point to the beginning of this AttributePath
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR Init(const chip::TLV::TLVReader & aReader);

    /**
     *  @brief Roughly verify the message is correctly formed
     *   1) all mandatory tags are present
     *   2) all elements have expected data type
     *   3) any tag can only appear once
     *   4) At the top level of the structure, unknown tags are ignored for forward compatibility
     *  @note The main use of this function is to print out what we're
     *    receiving during protocol development and debugging.
     *    The encoding rule has changed in IM encoding spec so this
     *    check is only "roughly" conformant now.
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR CheckSchemaValidity() const;

    /**
     *  @brief Get a TLVReader for the NodeId. Next() must be called before accessing them.
     *
     *  @param [in] apNodeId    A pointer to apNodeId
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetNodeId(chip::NodeId * const apNodeId) const;

    /**
     *  @brief Get a TLVReader for the EndpointId. Next() must be called before accessing them.
     *
     *  @param [in] apEndpointId    A pointer to apEndpointId
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetEndpointId(chip::EndpointId * const apEndpointId) const;

    /**
     *  @brief Get a TLVReader for the ClusterId. Next() must be called before accessing them.
     *
     *  @param [in] apClusterId    A pointer to apClusterId
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetNamespacedClusterId(chip::ClusterId * const apClusterId) const;

    /**
     *  @brief Get a TLVReader for the FieldId. Next() must be called before accessing them.
     *
     *  @param [in] apFieldId    A pointer to apFieldId
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetFieldId(uint8_t * const apFieldId) const;

    /**
     *  @brief Get a TLVReader for the ListIndex. Next() must be called before accessing them.
     *
     *  @param [in] apListIndex    A pointer to apListIndex
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetListIndex(uint16_t * const apListIndex) const;
};

class Builder : public chip::app::Builder
{
public:
    /**
     *  @brief Initialize a AttributePath::Builder for writing into a TLV stream
     *
     *  @param [in] apWriter    A pointer to TLVWriter
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter);

    /**
     * Init the AttributePath container with an particular context tag.
     * Required to implement arrays of arrays, and to test ListBuilder.
     *
     * @param[in]   apWriter    Pointer to the TLVWriter that is encoding the message.
     * @param[in]   aContextTagToUse    A contextTag to use.
     *
     * @return                  CHIP_ERROR codes returned by Chip::TLV objects.
     */
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter, const uint8_t aContextTagToUse);

    /**
     *  @brief Inject NodeId into the TLV stream.
     *
     *  @param [in] aNodeId NodeId for this attribute path
     *
     *  @return A reference to *this
     */
    AttributePath::Builder & NodeId(const chip::NodeId aNodeId);

    /**
     *  @brief Inject EndpointId into the TLV stream.
     *
     *  @param [in] aEndpointId NodeId for this attribute path
     *
     *  @return A reference to *this
     */
    AttributePath::Builder & EndpointId(const chip::EndpointId aEndpointId);

    /**
     *  @brief Inject NamespacedClusterId into the TLV stream.
     *
     *  @param [in] aNamespacedClusterId NamespacedClusterId for this attribute path
     *
     *  @return A reference to *this
     */
    AttributePath::Builder & NamespacedClusterId(const chip::ClusterId aNamespacedClusterId);

    /**
     *  @brief Inject FieldId into the TLV stream.
     *
     *  @param [in] aFieldId FieldId for this attribute path
     *
     *  @return A reference to *this
     */
    AttributePath::Builder & FieldId(const uint8_t aFieldId);

    /**
     *  @brief Inject NodeId into the TLV stream.
     *
     *  @param [in] aListIndex NodeId for this attribute path
     *
     *  @return A reference to *this
     */
    AttributePath::Builder & ListIndex(const uint16_t aListIndex);

    /**
     *  @brief Mark the end of this AttributePath
     *
     *  @return A reference to *this
     */
    AttributePath::Builder & EndOfAttributePath();

private:
    CHIP_ERROR _Init(chip::TLV::TLVWriter * const apWriter, const uint64_t aTag);
};

}; // namespace AttributePath

namespace AttributePathList {
class Parser : public ListParser
{
public:
    /**
     *  @brief Roughly verify the message is correctly formed
     *   1) all mandatory tags are present
     *   2) all elements have expected data type
     *   3) any tag can only appear once
     *   4) At the top level of the structure, unknown tags are ignored for forward compatibility
     *  @note The main use of this function is to print out what we're
     *    receiving during protocol development and debugging.
     *    The encoding rule has changed in IM encoding spec so this
     *    check is only "roughly" conformant now.
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR CheckSchemaValidity() const;
};

class Builder : public ListBuilder
{
public:
    /**
     *  @brief Initialize a AttributePath::Builder for writing into the TLV stream
     *
     *  @return A reference to AttributePath::Builder
     */
    AttributePath::Builder & CreateAttributePathBuilder();

    /**
     *  @brief Mark the end of this AttributePath
     *
     *  @return A reference to *this
     */
    AttributePathList::Builder & EndOfAttributePathList();

private:
    AttributePath::Builder mAttributePathBuilder;
};
}; // namespace AttributePathList

namespace EventPath {
enum
{
    kCsTag_NodeId              = 0,
    kCsTag_EndpointId          = 1,
    kCsTag_NamespacedClusterId = 2,
    kCsTag_EventId             = 3,
};

class Parser : public chip::app::Parser
{
public:
    /**
     *  @brief Initialize the parser object with TLVReader
     *
     *  @param [in] aReader A pointer to a TLVReader, which should point to the beginning of this EventPath
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR Init(const chip::TLV::TLVReader & aReader);

    /**
     *  @brief Roughly verify the message is correctly formed
     *   1) all mandatory tags are present
     *   2) all elements have expected data type
     *   3) any tag can only appear once
     *   4) At the top level of the structure, unknown tags are ignored for forward compatibility
     *  @note The main use of this function is to print out what we're
     *    receiving during protocol development and debugging.
     *    The encoding rule has changed in IM encoding spec so this
     *    check is only "roughly" conformant now.
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR CheckSchemaValidity() const;

    /**
     *  @brief Get a TLVReader for the NodeId. Next() must be called before accessing them.
     *
     *  @param [in] apNodeId    A pointer to apNodeId
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetNodeId(chip::NodeId * const apNodeId) const;

    /**
     *  @brief Get a TLVReader for the EndpointId. Next() must be called before accessing them.
     *
     *  @param [in] apEndpointId    A pointer to apEndpointId
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetEndpointId(chip::EndpointId * const apEndpointId) const;

    /**
     *  @brief Get a TLVReader for the NamespacedClusterId. Next() must be called before accessing them.
     *
     *  @param [in] apNamespacedClusterId    A pointer to apNamespacedClusterId
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetNamespacedClusterId(chip::ClusterId * const apNamespacedClusterId) const;

    /**
     *  @brief Get a TLVReader for the EventId. Next() must be called before accessing them.
     *
     *  @param [in] apEventId    A pointer to apEventId
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetEventId(chip::EventId * const apEventId) const;
};

class Builder : public chip::app::Builder
{
public:
    /**
     *  @brief Initialize a EventPath::Builder for writing into a TLV stream
     *
     *  @param [in] apWriter    A pointer to TLVWriter
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter);

    /**
     * Init the EventPath container with an particular context tag.
     * Required to implement arrays of arrays, and to test ListBuilder.
     *
     * @param[in]   apWriter    Pointer to the TLVWriter that is encoding the message.
     * @param[in]   aContextTagToUse    A contextTag to use.
     *
     * @return                  CHIP_ERROR codes returned by Chip::TLV objects.
     */
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter, const uint8_t aContextTagToUse);

    /**
     *  @brief Inject NodeId into the TLV stream.
     *
     *  @param [in] aNodeId NodeId for this event path
     *
     *  @return A reference to *this
     */
    EventPath::Builder & NodeId(const chip::NodeId aNodeId);

    /**
     *  @brief Inject EndpointId into the TLV stream.
     *
     *  @param [in] aEndpointId EndpointId for this eevent path
     *
     *  @return A reference to *this
     */
    EventPath::Builder & EndpointId(const chip::EndpointId aEndpointId);

    /**
     *  @brief Inject NamespacedClusterId into the TLV stream.
     *
     *  @param [in] aNamespacedClusterId NamespacedClusterId for this event path
     *
     *  @return A reference to *this
     */
    EventPath::Builder & NamespacedClusterId(const chip::ClusterId aNamespacedClusterId);

    /**
     *  @brief Inject EventId into the TLV stream.
     *
     *  @param [in] aEventId NamespacedClusterId for this event path
     *
     *  @return A reference to *this
     */
    EventPath::Builder & EventId(const chip::EventId aEventId);

    /**
     *  @brief Mark the end of this EventPath
     *
     *  @return A reference to *this
     */
    EventPath::Builder & EndOfEventPath();

private:
    CHIP_ERROR _Init(chip::TLV::TLVWriter * const apWriter, const uint64_t aTag);
};
}; // namespace EventPath

namespace EventPathList {
class Parser : public ListParser
{
public:
    /**
     *  @brief Roughly verify the message is correctly formed
     *   1) all mandatory tags are present
     *   2) all elements have expected data type
     *   3) any tag can only appear once
     *   4) At the top level of the structure, unknown tags are ignored for forward compatibility
     *  @note The main use of this function is to print out what we're
     *    receiving during protocol development and debugging.
     *    The encoding rule has changed in IM encoding spec so this
     *    check is only "roughly" conformant now.
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR CheckSchemaValidity() const;
};

class Builder : public ListBuilder
{
public:
    /**
     *  @brief Initialize a EventPath::Builder for writing into the TLV stream
     *
     *  @return A reference to EventPath::Builder
     */
    EventPath::Builder & CreateEventPathBuilder();

    /**
     *  @brief Mark the end of this EventPathList
     *
     *  @return A reference to *this
     */
    EventPathList::Builder & EndOfEventPathList();

private:
    EventPath::Builder mEventPathBuilder;
};
}; // namespace EventPathList

namespace CommandPath {
enum
{
    kCsTag_EndpointId          = 0,
    kCsTag_GroupId             = 1,
    kCsTag_NamespacedClusterId = 2,
    kCsTag_CommandId           = 3,
};

class Parser : public chip::app::Parser
{
public:
    /**
     *  @brief Initialize the parser object with TLVReader
     *
     *  @param [in] aReader A pointer to a TLVReader, which should point to the beginning of this CommandPath
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR Init(const chip::TLV::TLVReader & aReader);

    /**
     *  @brief Roughly verify the message is correctly formed
     *   1) all mandatory tags are present
     *   2) all elements have expected data type
     *   3) any tag can only appear once
     *   4) At the top level of the structure, unknown tags are ignored for forward compatibility
     *  @note The main use of this function is to print out what we're
     *    receiving during protocol development and debugging.
     *    The encoding rule has changed in IM encoding spec so this
     *    check is only "roughly" conformant now.
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR CheckSchemaValidity() const;

    /**
     *  @brief Get a TLVReader for the EndpointId. Next() must be called before accessing them.
     *
     *  @param [in] apEndpointId    A pointer to apEndpointId
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetEndpointId(chip::EndpointId * const apEndpointId) const;

    /**
     *  @brief Get a TLVReader for the GroupId. Next() must be called before accessing them.
     *
     *  @param [in] apGroupId    A pointer to apGroupId
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetGroupId(chip::GroupId * const apGroupId) const;

    /**
     *  @brief Get a TLVReader for the NamespacedClusterId. Next() must be called before accessing them.
     *
     *  @param [in] apEndpointId    A pointer to NamespacedClusterId
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetNamespacedClusterId(chip::ClusterId * const apNamespacedClusterId) const;

    /**
     *  @brief Get a TLVReader for the CommandId. Next() must be called before accessing them.
     *
     *  @param [in] apEndpointId    A pointer to CommandId
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetCommandId(chip::CommandId * const apCommandId) const;
};

class Builder : public chip::app::Builder
{
public:
    /**
     *  @brief Initialize a CommandPath::Builder for writing into a TLV stream
     *
     *  @param [in] apWriter    A pointer to TLVWriter
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter);

    /**
     * Init the CommandPath container with an particular context tag.
     * Required to implement arrays of arrays, and to test ListBuilder.
     *
     * @param[in]   apWriter    Pointer to the TLVWriter that is encoding the message.
     * @param[in]   aContextTagToUse    A contextTag to use.
     *
     * @return                  CHIP_ERROR codes returned by Chip::TLV objects.
     */
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter, const uint8_t aContextTagToUse);

    /**
     *  @brief Inject EndpointId into the TLV stream to indicate the endpointId referenced by the path.
     *
     *  @param [in] aEndpointId refer to the ID of the endpoint as described in the descriptor cluster.
     *
     *  @return A reference to *this
     */
    CommandPath::Builder & EndpointId(const chip::EndpointId aEndpointId);

    /**
     *  @brief Inject GroupId into the TLV stream to indicate the GroupId referenced by the path.
     *
     *  @param [in] aGroupId  Group Id for this Command path
     *
     *  @return A reference to *this
     */
    CommandPath::Builder & GroupId(const chip::GroupId aGroupId);

    /**
     *  @brief Inject NamespacedClusterId into the TLV stream.
     *
     *  @param [in] aNamespacedClusterId NamespacedClusterId for this command path
     *
     *  @return A reference to *this
     */
    CommandPath::Builder & NamespacedClusterId(const chip::ClusterId aNamespacedClusterId);

    /**
     *  @brief Inject CommandId into the TLV stream
     *
     *  @param [in] aCommandId Command Id for NamespacedClusterId for this command path
     *
     *  @return A reference to *this
     */
    CommandPath::Builder & CommandId(const chip::CommandId aCommandId);

    /**
     *  @brief Mark the end of this CommandPath
     *
     *  @return A reference to *this
     */
    CommandPath::Builder & EndOfCommandPath();

private:
    CHIP_ERROR _Init(chip::TLV::TLVWriter * const apWriter, const uint64_t aTag);
};
}; // namespace CommandPath

namespace EventDataElement {
enum
{
    kCsTag_EventPath            = 0,
    kCsTag_ImportanceLevel      = 1,
    kCsTag_Number               = 2,
    kCsTag_UTCTimestamp         = 3,
    kCsTag_SystemTimestamp      = 4,
    kCsTag_DeltaUTCTimestamp    = 5,
    kCsTag_DeltaSystemTimestamp = 6,
    kCsTag_Data                 = 7,
};

class Parser : public chip::app::Parser
{
public:
    /**
     *  @brief Initialize the parser object with TLVReader
     *
     *  @param [in] aReader A pointer to a TLVReader, which should point to the beginning of this EventDataElement
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR Init(const chip::TLV::TLVReader & aReader);

    /**
     *  @brief Roughly verify the message is correctly formed
     *   1) all mandatory tags are present
     *   2) all elements have expected data type
     *   3) any tag can only appear once
     *   4) At the top level of the structure, unknown tags are ignored for forward compatibility
     *  @note The main use of this function is to print out what we're
     *    receiving during protocol development and debugging.
     *    The encoding rule has changed in IM encoding spec so this
     *    check is only "roughly" conformant now.
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR CheckSchemaValidity() const;

    /**
     *  @brief Get a TLVReader for the EventPath. Next() must be called before accessing them.
     *
     *  @param [in] apEventPath    A pointer to apEventPath
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not a Path
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetEventPath(EventPath::Parser * const apEventPath);

    /**
     *  @brief Get a TLVReader for the Number. Next() must be called before accessing them.
     *
     *  @param [in] apImportanceLevel    A pointer to apImportanceLevel
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetImportanceLevel(uint8_t * const apImportanceLevel);

    /**
     *  @brief Get a TLVReader for the Number. Next() must be called before accessing them.
     *
     *  @param [in] apNumber    A pointer to apNumber
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetNumber(uint64_t * const apNumber);

    /**
     *  @brief Get a TLVReader for the UTCTimestamp. Next() must be called before accessing them.
     *
     *  @param [in] apUTCTimestamp    A pointer to apUTCTimestamp
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetUTCTimestamp(uint64_t * const apUTCTimestamp);

    /**
     *  @brief Get a TLVReader for the SystemTimestamp. Next() must be called before accessing them.
     *
     *  @param [in] apSystemTimestamp    A pointer to apSystemTimestamp
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetSystemTimestamp(uint64_t * const apSystemTimestamp);

    /**
     *  @brief Get a TLVReader for the DeltaUTCTime. Next() must be called before accessing them.
     *
     *  @param [in] apDeltaUTCTimestamp   A pointer to apDeltaUTCTime
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetDeltaUTCTime(uint64_t * const apDeltaUTCTime);

    /**
     *  @brief Get a TLVReader for the DeltaSystemTime. Next() must be called before accessing them.
     *
     *  @param [in] apDeltaSystemTimestamp   A pointer to apDeltaSystemTime
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetDeltaSystemTime(uint64_t * const apDeltaSystemTime);

    /**
     *  @brief Get a TLVReader for the Data. Next() must be called before accessing them.
     *
     *  @param [in] apReader    A pointer to apReader
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetData(chip::TLV::TLVReader * const apReader) const;

protected:
    // A recursively callable function to parse a data element and pretty-print it.
    CHIP_ERROR ParseData(chip::TLV::TLVReader & aReader, int aDepth) const;
};

class Builder : public chip::app::Builder
{
public:
    /**
     *  @brief Initialize a EventDataElement::Builder for writing into a TLV stream
     *
     *  @param [in] apWriter    A pointer to TLVWriter
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter);

    /**
     *  @brief Initialize a EventPath::Builder for writing into the TLV stream
     *
     *  @return A reference to EventPath::Builder
     */
    EventPath::Builder & CreateEventPathBuilder();

    /**
     *  @brief Inject ImportanceLevel into the TLV stream to indicate the importance level associated with
     *  the cluster that is referenced by the path.
     *
     *  @param [in] aImportanceLevel This is an integer representation of the importance level.
     *
     *  @return A reference to *this
     */
    EventDataElement::Builder ImportanceLevel(const uint8_t aImportanceLevel);

    /**
     *  @brief Inject Number into the TLV stream to indicate the number associated with
     *  the cluster that is referenced by the path. The event number is a monotonically increasing number that
     *  uniquely identifies each emitted event. This number is scoped to the ImportanceLevel.
     *
     *  @param [in] aNumber The uint64_t variable to reflectt the event number
     *
     *  @return A reference to *this
     */
    EventDataElement::Builder Number(const uint64_t aNumber);

    /**
     *  @brief Inject UTCTimestamp into the TLV stream.
     *  This is encoded as a 64-bit millisecond number since UNIX epoch (Jan 1 1970 00:00:00 GMT).
     *
     *  @param [in] aUTCTimestamp The uint64_t variable to reflect the UTC timestamp of the Event.
     *
     *  @return A reference to *this
     */
    EventDataElement::Builder UTCTimestamp(const uint64_t aUTCTimestamp);

    /**
     *  @brief Inject SystemTimestamp into the TLV stream. If UTC time is not available, time since boot
     *  SHALL be encoded in this field as 64-bit, milliseconds.
     *
     *  @param [in] aSystemTimestamp The uint64_t variable to reflect system time
     *
     *  @return A reference to *this
     */
    EventDataElement::Builder SystemTimestamp(const uint64_t aSystemTimestamp);

    /**
     *  @brief Inject DataVersion into the TLV stream to indicate the numerical data version associated with
     *  the cluster that is referenced by the path. When this field is present, the UTC Timestamp field SHALL be omitted.
     *
     *  @param [in] aDataVersion The uint64_t variable to reflect DeltaUTCTime
     *
     *  @return A reference to *this
     */
    EventDataElement::Builder DeltaUTCTime(const uint64_t aDeltaUTCTime);

    /**
     *  @brief Inject DeltaSystemTimestamp into the TLV stream.
     *  This field is present if delta encoding of the System timestamp relative to a prior event is desired for compression
     * reasons. When this field is present, the System Timestamp field SHALL be omitted.
     *
     *  @param [in] DeltaSystemTimestamp The uint64_t variable to reflect DeltaSystemTime
     *
     *  @return A reference to *this
     */
    EventDataElement::Builder DeltaSystemTime(const uint64_t aDeltaSystemTime);

    /**
     *  @brief Mark the end of this EventDataElement
     *
     *  @return A reference to *this
     */
    EventDataElement::Builder & EndOfEventDataElement();

private:
    EventPath::Builder mEventPathBuilder;
};
}; // namespace EventDataElement

namespace EventList {
class Parser : public ListParser
{
public:
    /**
     *  @brief Roughly verify the message is correctly formed
     *   1) all mandatory tags are present
     *   2) all elements have expected data type
     *   3) any tag can only appear once
     *   4) At the top level of the structure, unknown tags are ignored for forward compatibility
     *  @note The main use of this function is to print out what we're
     *    receiving during protocol development and debugging.
     *    The encoding rule has changed in IM encoding spec so this
     *    check is only "roughly" conformant now.
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR CheckSchemaValidity() const;
};

class Builder : public ListBuilder
{
public:
    /**
     *  @brief Initialize a EventDataElement::Builder for writing into the TLV stream
     *
     *  @return A reference to EventDataElement::Builder
     */
    EventDataElement::Builder & CreateEventBuilder();

    /**
     *  @brief Mark the end of this EventList
     *
     *  @return A reference to *this
     */
    EventList::Builder & EndOfEventList();

private:
    EventDataElement::Builder mEventDataElementBuilder;
};
}; // namespace EventList

namespace StatusElement {
enum
{
    kCsTag_GeneralCode         = 1,
    kCsTag_ProtocolId          = 2,
    kCsTag_ProtocolCode        = 3,
    kCsTag_NamespacedClusterId = 4
};

class Parser : public ListParser
{
public:
    /**
     *  @brief Initialize the parser object with TLVReader
     *
     *  @param [in] aReader A pointer to a TLVReader, which should point to the beginning of this StatusElement
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR Init(const chip::TLV::TLVReader & aReader);

    /**
     *  @brief Roughly verify the message is correctly formed
     *   1) all mandatory tags are present
     *   2) all elements have expected data type
     *   3) any tag can only appear once
     *   4) At the top level of the structure, unknown tags are ignored for forward compatibility
     *  @note The main use of this function is to print out what we're
     *    receiving during protocol development and debugging.
     *    The encoding rule has changed in IM encoding spec so this
     *    check is only "roughly" conformant now.
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR CheckSchemaValidity() const;

    /**
    `* Read the GeneralCode, ProtocolId, ProtocolCode, NamespacedClusterId
     *
     * @param[out]   apGeneralCode     Pointer to the storage for the GeneralCode
     * @param[out]   apProtocolId     Pointer to the storage for the ProtocolId
     * @param[out]   apProtocolCode   Pointer to the storage for the ProtocolCode
     * @param[out]   apNamespacedClusterId     Pointer to the storage for the NamespacedClusterId
     *
     * @return       CHIP_ERROR codes returned by Chip::TLV objects. CHIP_END_OF_TLV if either
     *               element is missing. CHIP_ERROR_WRONG_TLV_TYPE if the elements are of the wrong
     *               type.
     */
    CHIP_ERROR DecodeStatusElement(uint16_t * apGeneralCode, uint32_t * apProtocolId, uint16_t * apProtocolCode,
                                   chip::ClusterId * apNamespacedClusterId) const;
};

class Builder : public ListBuilder
{
public:
    /**
     *  @brief Initialize a StatusElement::Builder for writing into a TLV stream
     *
     *  @param [in] apWriter    A pointer to TLVWriter
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter);

    /**
     * Init the StatusElement container with an particular context tag.
     * Required to implement arrays of arrays, and to test ListBuilder.
     *
     * @param[in]   apWriter    Pointer to the TLVWriter that is encoding the message.
     * @param[in]   aContextTagToUse    A contextTag to use.
     *
     * @return                  CHIP_ERROR codes returned by Chip::TLV objects.
     */
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter, const uint8_t aContextTagToUse);

    /**
    `* Read the GeneralCode, ProtocolId, ProtocolCode, NamespacedClusterId
     *
     * @param[in]   aGeneralCode    General status code
     * @param[in]   aProtocolId     A protocol ID (32-bit integer composed of a 16-bit vendor id and 16-bit Scoped id)
     * @param[in]   aProtocolCode   16-bit protocol-specific error code
     * @param[in]   aNamespacedClusterId      Cluster Id for ZCL
     *
     * @return       CHIP_ERROR codes returned by Chip::TLV objects. CHIP_END_OF_TLV if either
     *               element is missing. CHIP_ERROR_WRONG_TLV_TYPE if the elements are of the wrong
     *               type.
     */
    StatusElement::Builder & EncodeStatusElement(const uint16_t aGeneralCode, const uint32_t aProtocolId,
                                                 const uint16_t aProtocolCode, const chip::ClusterId aNamespacedClusterId);

    /**
     *  @brief Mark the end of this StatusElement
     *
     *  @return A reference to *this
     */
    StatusElement::Builder & EndOfStatusElement();
};
}; // namespace StatusElement

namespace AttributeStatusElement {
enum
{
    kCsTag_AttributePath = 0,
    kCsTag_StatusElement = 1,
};

class Builder : public chip::app::Builder
{
public:
    /**
     *  @brief Initialize a AttributeStatusElement::Builder for writing into a TLV stream
     *
     *  @param [in] apWriter    A pointer to TLVWriter
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter);

    /**
     *  @brief Initialize a AttributePath::Builder for writing into the TLV stream
     *
     *  @return A reference to AttributePath::Builder
     */
    AttributePath::Builder & CreateAttributePathBuilder();

    /**
     *  @brief Initialize a StatusElement::Builder for writing into the TLV stream
     *
     *  @return A reference to StatusElement::Builder
     */
    StatusElement::Builder & CreateStatusElementBuilder();

    /**
     *  @brief Mark the end of this AttributeStatusElement
     *
     *  @return A reference to *this
     */
    AttributeStatusElement::Builder & EndOfAttributeStatusElement();

private:
    AttributePath::Builder mAttributePathBuilder;
    StatusElement::Builder mStatusElementBuilder;
};

class Parser : public chip::app::Parser
{
public:
    /**
     *  @brief Initialize the parser object with TLVReader
     *
     *  @param [in] aReader A pointer to a TLVReader, which should point to the beginning of this AttributeStatusElement
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR Init(const chip::TLV::TLVReader & aReader);

    /**
     *  @brief Roughly verify the message is correctly formed
     *   1) all mandatory tags are present
     *   2) all elements have expected data type
     *   3) any tag can only appear once
     *   4) At the top level of the structure, unknown tags are ignored for forward compatibility
     *  @note The main use of this function is to print out what we're
     *    receiving during protocol development and debugging.
     *    The encoding rule has changed in IM encoding spec so this
     *    check is only "roughly" conformant now.
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR CheckSchemaValidity() const;

    /**
     *  @brief Get a TLVReader for the AttributePath. Next() must be called before accessing them.
     *
     *  @param [in] apAttributePath    A pointer to apAttributePath
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not a Path
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetAttributePath(AttributePath::Parser * const apAttributePath) const;

    /**
     *  @brief Get a TLVReader for the StatusElement. Next() must be called before accessing them.
     *
     *  @param [in] apStatusElement    A pointer to apStatusElement
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not a Path
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetStatusElement(StatusElement::Parser * const apStatusElement) const;
};
}; // namespace AttributeStatusElement

namespace AttributeStatusList {
class Builder : public ListBuilder
{
public:
    /**
     *  @brief Initialize a AttributeStatus::Builder for writing into the TLV stream
     *
     *  @return A reference to AttributeStatus::Builder
     */
    AttributeStatusElement::Builder & CreateAttributeStatusBuilder();

    /**
     *  @brief Mark the end of this AttributeStatusList
     *
     *  @return A reference to *this
     */
    AttributeStatusList::Builder & EndOfAttributeStatusList();

private:
    AttributeStatusElement::Builder mAttributeStatusBuilder;
};

class Parser : public ListParser
{
public:
    /**
     *  @brief Roughly verify the message is correctly formed
     *   1) all mandatory tags are present
     *   2) all elements have expected data type
     *   3) any tag can only appear once
     *   4) At the top level of the structure, unknown tags are ignored for forward compatibility
     *  @note The main use of this function is to print out what we're
     *    receiving during protocol development and debugging.
     *    The encoding rule has changed in IM encoding spec so this
     *    check is only "roughly" conformant now.
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR CheckSchemaValidity() const;
};
}; // namespace AttributeStatusList

namespace AttributeDataElement {
enum
{
    kCsTag_AttributePath       = 0,
    kCsTag_DataVersion         = 1,
    kCsTag_Data                = 2,
    kCsTag_MoreClusterDataFlag = 3,
};

class Parser : public chip::app::Parser
{
public:
    /**
     *  @brief Initialize the parser object with TLVReader
     *
     *  @param [in] aReader A pointer to a TLVReader, which should point to the beginning of this AttributeDataElement
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR Init(const chip::TLV::TLVReader & aReader);

    /**
     *  @brief Roughly verify the message is correctly formed
     *   1) all mandatory tags are present
     *   2) all elements have expected data type
     *   3) any tag can only appear once
     *   4) At the top level of the structure, unknown tags are ignored for forward compatibility
     *  @note The main use of this function is to print out what we're
     *    receiving during protocol development and debugging.
     *    The encoding rule has changed in IM encoding spec so this
     *    check is only "roughly" conformant now.
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR CheckSchemaValidity() const;

    /**
     *  @brief Get a TLVReader for the AttributePath. Next() must be called before accessing them.
     *
     *  @param [in] apAttributePath    A pointer to apAttributePath
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not a Path
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetAttributePath(AttributePath::Parser * const apAttributePath) const;

    /**
     *  @brief Get a TLVReader for the DataVersion. Next() must be called before accessing them.
     *
     *  @param [in] apVersion    A pointer to apVersion
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetDataVersion(chip::DataVersion * const apVersion) const;

    /**
     *  @brief Get a TLVReader for the Data. Next() must be called before accessing them.
     *
     *  @param [in] apReader    A pointer to apReader
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetData(chip::TLV::TLVReader * const apReader) const;

    /**
     *  @brief Check whether it need more cluster data Next() must be called before accessing them.
     *
     *  @param [in] apMoreClusterDataFlag    A pointer to apMoreClusterDataFlag
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetMoreClusterDataFlag(bool * const apMoreClusterDataFlag) const;

protected:
    // A recursively callable function to parse a data element and pretty-print it.
    CHIP_ERROR ParseData(chip::TLV::TLVReader & aReader, int aDepth) const;
};

class Builder : public chip::app::Builder
{
public:
    /**
     *  @brief Initialize a AttributeDataElement::Builder for writing into a TLV stream
     *
     *  @param [in] apWriter    A pointer to TLVWriter
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter);

    /**
     *  @brief Initialize a AttributePath::Builder for writing into the TLV stream
     *
     *  @return A reference to AttributePath::Builder
     */
    AttributePath::Builder & CreateAttributePathBuilder();

    /**
     *  @brief Inject DataVersion into the TLV stream to indicate the numerical data version associated with
     *  the cluster that is referenced by the path.
     *
     *  @param [in] aDataVersion The boolean variable to indicate if AttributeDataElement has version
     *
     *  @return A reference to *this
     */
    AttributeDataElement::Builder & DataVersion(const chip::DataVersion aDataVersion);

    /**
     *  @brief Inject aMoreClusterData into the TLV stream to indicate whether there is more cluster data.
     *  This is present when there is more than one AttributeDataElement as part of a logical Changeset,
     *  and the entire set needs to be applied ‘atomically’ on the receiver.
     *
     *  @param [in] aMoreClusterData The boolean variable to indicate if more cluster data is needed.
     *
     *  @return A reference to *this
     */
    AttributeDataElement::Builder & MoreClusterData(const bool aMoreClusterData);

    /**
     *  @brief Mark the end of this AttributeDataElement
     *
     *  @return A reference to *this
     */
    AttributeDataElement::Builder & EndOfAttributeDataElement();

private:
    AttributePath::Builder mAttributePathBuilder;
};
}; // namespace AttributeDataElement

namespace AttributeDataList {
class Parser : public ListParser
{
public:
    /**
     *  @brief Roughly verify the message is correctly formed
     *   1) all mandatory tags are present
     *   2) all elements have expected data type
     *   3) any tag can only appear once
     *   4) At the top level of the structure, unknown tags are ignored for forward compatibility
     *  @note The main use of this function is to print out what we're
     *    receiving during protocol development and debugging.
     *    The encoding rule has changed in IM encoding spec so this
     *    check is only "roughly" conformant now.
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR CheckSchemaValidity() const;
};

class Builder : public ListBuilder
{
public:
    /**
     *  @brief Initialize a AttributeDataElement::Builder for writing into the TLV stream
     *
     *  @return A reference to AttributeDataElement::Builder
     */
    AttributeDataElement::Builder & CreateAttributeDataElementBuilder();

    /**
     *  @brief Mark the end of this AttributeDataList
     *
     *  @return A reference to *this
     */
    AttributeDataList::Builder & EndOfAttributeDataList();

private:
    AttributeDataElement::Builder mAttributeDataElementBuilder;
};
}; // namespace AttributeDataList

namespace CommandDataElement {
enum
{
    kCsTag_CommandPath   = 0,
    kCsTag_Data          = 1,
    kCsTag_StatusElement = 2,
};

class Parser : public chip::app::Parser
{
public:
    /**
     *  @brief Initialize the parser object with TLVReader
     *
     *  @param [in] aReader A pointer to a TLVReader, which should point to the beginning of this CommandDataElement
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR Init(const chip::TLV::TLVReader & aReader);

    /**
     *  @brief Roughly verify the message is correctly formed
     *   1) all mandatory tags are present
     *   2) all elements have expected data type
     *   3) any tag can only appear once
     *   4) At the top level of the structure, unknown tags are ignored for forward compatibility
     *  @note The main use of this function is to print out what we're
     *    receiving during protocol development and debugging.
     *    The encoding rule has changed in IM encoding spec so this
     *    check is only "roughly" conformant now.
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR CheckSchemaValidity() const;

    /**
     *  @brief Get a TLVReader for the CommandPath. Next() must be called before accessing them.
     *
     *  @param [in] apCommandPath    A pointer to apCommandPath
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not a Path
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetCommandPath(CommandPath::Parser * const apCommandPath) const;

    /**
     *  @brief Get a TLVReader for the Data. Next() must be called before accessing them.
     *
     *  @param [in] apReader    A pointer to apReader
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetData(chip::TLV::TLVReader * const apReader) const;

    /**
     *  @brief Get a TLVReader for the Data. Next() must be called before accessing them.
     *
     *  @param [in] apStatusElement    A pointer to apStatusElement
     *
     *  @return #CHIP_NO_ERROR on success
     *          # CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not a structure
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetStatusElement(StatusElement::Parser * const apStatusElement) const;

protected:
    // A recursively callable function to parse a data element and pretty-print it.
    CHIP_ERROR ParseData(chip::TLV::TLVReader & aReader, int aDepth) const;
};

class Builder : public chip::app::Builder
{
public:
    /**
     *  @brief Initialize a AttributeDataList::Builder for writing into a TLV stream
     *
     *  @param [in] apWriter    A pointer to TLVWriter
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter);

    /**
     *  @brief Initialize a CommandPath::Builder for writing into the TLV stream
     *
     *  @return A reference to CommandPath::Builder
     */
    CommandPath::Builder & CreateCommandPathBuilder();

    /**
     *  @brief Initialize a StatusElement::Builder for writing into the TLV stream
     *
     *  @return A reference to StatusElement::Builder
     */
    StatusElement::Builder & CreateStatusElementBuilder();

    /**
     *  @brief Mark the end of this CommandDataElement
     *
     *  @return A reference to *this
     */
    CommandDataElement::Builder & EndOfCommandDataElement();

private:
    CommandPath::Builder mCommandPathBuilder;
    StatusElement::Builder mStatusElementBuilder;
};
}; // namespace CommandDataElement

namespace CommandList {
class Parser : public ListParser
{
public:
    /**
     *  @brief Roughly verify the message is correctly formed
     *   1) all mandatory tags are present
     *   2) all elements have expected data type
     *   3) any tag can only appear once
     *   4) At the top level of the structure, unknown tags are ignored for forward compatibility
     *  @note The main use of this function is to print out what we're
     *    receiving during protocol development and debugging.
     *    The encoding rule has changed in IM encoding spec so this
     *    check is only "roughly" conformant now.
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR CheckSchemaValidity() const;
};

class Builder : public ListBuilder
{
public:
    /**
     *  @brief Initialize a CommandDataElement::Builder for writing into the TLV stream
     *
     *  @return A reference to AttributeDataList::Builder
     */
    CommandDataElement::Builder & CreateCommandDataElementBuilder();

    /**
     *  @brief Mark the end of this CommandList
     *
     *  @return A reference to *this
     */
    CommandList::Builder & EndOfCommandList();

private:
    CommandDataElement::Builder mCommandDataElementBuilder;
};
}; // namespace CommandList

namespace ReportData {
enum
{
    kCsTag_RequestResponse     = 0,
    kCsTag_SubscriptionId      = 1,
    kCsTag_AttributeStatusList = 2,
    kCsTag_AttributeDataList   = 3,
    kCsTag_EventDataList       = 4,
    kCsTag_IsLastReport        = 5,
};

class Parser : public chip::app::Parser
{
public:
    /**
     *  @brief Initialize the parser object with TLVReader
     *
     *  @param [in] aReader A pointer to a TLVReader, which should point to the beginning of this ReportData
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR Init(const chip::TLV::TLVReader & aReader);

    /**
     *  @brief Roughly verify the message is correctly formed
     *   1) all mandatory tags are present
     *   2) all elements have expected data type
     *   3) any tag can only appear once
     *   4) At the top level of the structure, unknown tags are ignored for forward compatibility
     *  @note The main use of this function is to print out what we're
     *    receiving during protocol development and debugging.
     *    The encoding rule has changed in IM encoding spec so this
     *    check is only "roughly" conformant now.
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR CheckSchemaValidity() const;

    /**
     *  @brief Check whether this message needs request response. Next() must be called before accessing them.
     *
     *  @param [in] apRequestResponse    A pointer to apRequestResponse
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetRequestResponse(bool * const apRequestResponse) const;

    /**
     *  @brief Get Subscription Id. Next() must be called before accessing them.
     *
     *  @param [in] apSubscriptionId    A pointer to apIsLastReport
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetSubscriptionId(uint64_t * const apSubscriptionId) const;

    /**
     *  @brief Get a TLVReader for the AttributesStatusList. Next() must be called before accessing them.
     *
     *  @param [in] apAttributeStatusList    A pointer to apAttributeStatusList
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not a Array
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetAttributeStatusList(AttributeStatusList::Parser * const apAttributeStatusList) const;

    /**
     *  @brief Get a TLVReader for the AttributesDataList. Next() must be called before accessing them.
     *
     *  @param [in] apAttributeDataList    A pointer to apAttributeDataList
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not a Array
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetAttributeDataList(AttributeDataList::Parser * const apAttributeDataList) const;

    /**
     *  @brief Get a TLVReader for the EventDataList. Next() must be called before accessing them.
     *
     *  @param [in] apEventDataList    A pointer to apEventDataList
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not a Array
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetEventDataList(EventList::Parser * const apEventDataList) const;

    /**
     *  @brief Check whether this message is last report. Next() must be called before accessing them.
     *
     *  @param [in] apIsLastReport    A pointer to apIsLastReport
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetIsLastReport(bool * const apIsLastReport) const;
};

class Builder : public chip::app::Builder
{
public:
    /**
     *  @brief Initialize a ReportData::Builder for writing into a TLV stream
     *
     *  @param [in] apWriter    A pointer to TLVWriter
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter);

    /**
     *  @brief Inject RequestResponse into the TLV stream to indicate whether a response (a StatusReponse specifically)
     *  is to be sent back to the request.
     *
     *  @param [in] aRequestResponse The boolean variable to indicate if request response is needed.
     *
     *  @return A reference to *this
     */
    ReportData::Builder & RequestResponse(const bool aRequestResponse);

    /**
     *  @brief Inject subscription id into the TLV stream, This field contains the Subscription ID
     *  to which the data is being sent against. This is not present when the ReportDataRequest is
     *  sent in response to a ReadRequest, but is present when sent in response to a SubscribeRequest.
     *  Attempts should be made to ensure the SubscriptionId does not collide with IDs from previous
     *  subscriptions to ensure disambiguation.
     *
     *  @param [in] aSubscriptionId  Subscription Id for this report data
     *
     *  @return A reference to *this
     */
    ReportData::Builder & SubscriptionId(const uint64_t aSubscriptionId);

    /**
     *  @brief Initialize a AttributeStatusList::Builder for writing into the TLV stream
     *
     *  @return A reference to AttributeStatusList::Builder
     */
    AttributeStatusList::Builder & CreateAttributeStatusListBuilder();

    /**
     *  @brief Initialize a AttributeDataList::Builder for writing into the TLV stream
     *
     *  @return A reference to AttributeDataList::Builder
     */
    AttributeDataList::Builder & CreateAttributeDataListBuilder();

    /**
     *  @brief Initialize a EventList::Builder for writing into the TLV stream
     *
     *  @return A reference to EventList::Builder
     */
    EventList::Builder & CreateEventDataListBuilder();

    /**
     *  @brief This flag is set to ‘true’ when this is the last ReportDataRequest message
     *  in a transaction and there are no more Changes to be conveyed.
     *  @param [in] aIsLastReport The boolean variable to indicate if it is LastReport
     *  @return A reference to *this
     */
    ReportData::Builder & IsLastReport(const bool aIsLastReport);

    /**
     *  @brief Mark the end of this ReportData
     *
     *  @return A reference to *this
     */
    ReportData::Builder & EndOfReportData();

private:
    AttributeStatusList::Builder mAttributeStatusListBuilder;
    AttributeDataList::Builder mAttributeDataListBuilder;
    EventList::Builder mEventDataListBuilder;
};
}; // namespace ReportData

namespace InvokeCommand {
enum
{
    kCsTag_CommandList = 0,
};

class Parser : public chip::app::Parser
{
public:
    /**
     *  @brief Initialize the parser object with TLVReader
     *
     *  @param [in] aReader A pointer to a TLVReader, which should point to the beginning of this request
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR Init(const chip::TLV::TLVReader & aReader);

    /**
     *  @brief Roughly verify the message is correctly formed
     *   1) all mandatory tags are present
     *   2) all elements have expected data type
     *   3) any tag can only appear once
     *   4) At the top level of the structure, unknown tags are ignored for forward compatibility
     *  @note The main use of this function is to print out what we're
     *    receiving during protocol development and debugging.
     *    The encoding rule has changed in IM encoding spec so this
     *    check is only "roughly" conformant now.
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR CheckSchemaValidity() const;

    /**
     *  @brief Get a TLVReader for the CommandList. Next() must be called before accessing them.
     *
     *  @param [in] apWriter    A pointer to TLVWriter
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetCommandList(CommandList::Parser * const apCommandList) const;
};

class Builder : public chip::app::Builder
{
public:
    /**
     *  @brief Initialize a InvokeCommand::Builder for writing into a TLV stream
     *
     *  @param [in] apWriter    A pointer to TLVWriter
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter);

    /**
     *  @brief Initialize a CommandList::Builder for writing into the TLV stream
     *
     *  @return A reference to CommandList::Builder
     */
    CommandList::Builder & CreateCommandListBuilder();

    /**
     *  @brief Get reference to CommandList::Builder
     *
     *  @return A reference to CommandList::Builder
     */
    CommandList::Builder & GetCommandListBuilder();

    /**
     *  @brief Mark the end of this InvokeCommand
     *
     *  @return A reference to *this
     */
    InvokeCommand::Builder & EndOfInvokeCommand();

private:
    CommandList::Builder mCommandListBuilder;
};
}; // namespace InvokeCommand

}; // namespace app
}; // namespace chip

#endif // _CHIP_INTERACTION_MODEL_MESSAGE_DEF_H
