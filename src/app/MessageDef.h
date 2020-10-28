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

#ifndef _CHIP_INTERACTION_MODEL_MESSAGE_DEF_H
#define _CHIP_INTERACTION_MODEL_MESSAGE_DEF_H

#include <core/CHIPCore.h>
#include <core/CHIPTLV.h>
#include <support/CodeUtils.h>
#include <support/DLLUtil.h>
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

static inline bool IsVersionNewer(const CHIPDataVersion & aVersion, const CHIPDataVersion & aReference)
{
    return (aVersion != aReference);
}

static inline bool IsVersionNewerOrEqual(const CHIPDataVersion & aVersion, const CHIPDataVersion & aReference)
{
    return true;
}

/**
 *  @class ParserBase
 *
 *  @brief
 *    Base class for WDM message parsers
 */
class ParserBase
{
public:
    /**
     *  @brief Initialize a TLVReader to point to the beginning of any tagged element in this request
     *
     *  @param [in]  aTagToFind Tag to find in the request
     *  @param [out] apReader   A pointer to TLVReader, which will be initialized at the specified TLV element
     *                          on success
     *
     *  @retval #CHIP_NO_ERROR on success
     */
    CHIP_ERROR GetReaderOnTag(const uint64_t aTagToFind, chip::TLV::TLVReader * const apReader) const;

protected:
    chip::TLV::TLVReader mReader;

    ParserBase();

    template <typename T>
    CHIP_ERROR GetUnsignedInteger(const uint8_t aContextTag, T * const apLValue) const;

    template <typename T>
    CHIP_ERROR GetSimpleValue(const uint8_t aContextTag, const chip::TLV::TLVType aTLVType, T * const apLValue) const;
};

/**
 *  @class ListParserBase
 *
 *  @brief
 *    Base class for WDM message parsers, specialized in TLV array elements like Data Lists and Version Lists
 */
class ListParserBase : public ParserBase
{
protected:
    ListParserBase();

public:
    // aReader has to be on the element of the array element
    CHIP_ERROR Init(const chip::TLV::TLVReader & aReader);

    // aReader has to be at the beginning of some container
    CHIP_ERROR InitIfPresent(const chip::TLV::TLVReader & aReader, const uint8_t aContextTagToFind);

    CHIP_ERROR Next();
    void GetReader(chip::TLV::TLVReader * const apReader);
};

/**
 *  @class BuilderBase
 *
 *  @brief
 *    Base class for CHIP interaction model message encoders
 */
class BuilderBase
{
public:
    void ResetError();
    void ResetError(CHIP_ERROR aErr);
    CHIP_ERROR GetError() const { return mError; };
    chip::TLV::TLVWriter * GetWriter() { return mpWriter; };

protected:
    CHIP_ERROR mError;
    chip::TLV::TLVWriter * mpWriter;
    chip::TLV::TLVType mOuterContainerType;

    BuilderBase();
    void EndOfContainer();

    CHIP_ERROR InitAnonymousStructure(chip::TLV::TLVWriter * const apWriter);
};

/**
 *  @class ListBuilderBase
 *
 *  @brief
 *    Base class for CHIP interaction message encoders, specialized in TLV array elements like Data Lists and Event Lists
 */
class ListBuilderBase : public BuilderBase
{
protected:
    ListBuilderBase();

public:
    /**
     * Init the TLV array container with an anonymous tag.
     * Required to implement arrays of arrays, and to test ListBuilderBase.
     * There is no chip interaction model message that has an array as the outermost container.
     *
     * @param[in]   apWriter    Pointer to the TLVWriter that is encoding the message.
     * @param[in]   aContextTagToUse    A contextTag to use.
     *
     * @return                  CHIP_ERROR codes returned by Chip::TLV objects.
     */

    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter, const uint8_t aContextTagToUse);
    /**
     * Init the TLV array container with an anonymous tag.
     * Required to implement arrays of arrays, and to test ListBuilderBase.
     * There is no chip interaction model message that has an array as the outermost container.
     *
     * @param[in]   apWriter    Pointer to the TLVWriter that is encoding the message.
     *
     * @return                  CHIP_ERROR codes returned by Chip::TLV objects.
     */
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter);
};

/**
 *  @brief
 *    CHIP interaction model Attribute Path definition
 *
 */
namespace AttributePath {
enum
{
    kCsTag_NodeId     = 0,
    kCsTag_EndpointId = 1,
    kCsTag_ClusterId  = 2,
    kCsTag_FieldTag   = 3,
    kCsTag_ListIndex  = 4,
};

/**
 *  @class AttributePath::Parser
 *
 *  @brief
 *    Parses a Attribute Path container
 */
class Parser : public ParserBase
{
public:
    // aReader has to be on the element of EventPath container
    CHIP_ERROR Init(const chip::TLV::TLVReader & aReader);

    // Roughly verify the schema is right, including
    // 1) all mandatory tags are present
    // 2) no unknown tags
    // 3) all elements have expected data type
    // 4) any tag can only appear once
    CHIP_ERROR CheckSchemaValidity() const;

    // CHIP_END_OF_TLV if there is no such element
    // CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
    CHIP_ERROR GetNodeId(CHIPNodeId * const apNodeId) const;

    // CHIP_END_OF_TLV if there is no such element
    // CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
    CHIP_ERROR GetEndpointId(CHIPEndpointId * const apEndpointId) const;

    // CHIP_END_OF_TLV if there is no such element
    // CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
    CHIP_ERROR GetClusterId(CHIPClusterId * const apClusterId) const;

    // CHIP_END_OF_TLV if there is no such element
    // CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
    CHIP_ERROR GetFieldTag(uint8_t * const apFieldTag) const;

    // CHIP_END_OF_TLV if there is no such element
    // CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
    CHIP_ERROR GetListIndex(uint16_t * const apListIndex) const;

    // Get a TLVReader at the additional tags section. Next() must be called before accessing it.
    CHIP_ERROR GetTags(chip::TLV::TLVReader * const apReader) const;
};

/**
 *  @class AttributePath::Builder
 *
 *  @brief
 *    Attribute Path Builder
 */
class Builder : public BuilderBase
{
public:
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter);
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter, const uint8_t aContextTagToUse);

    AttributePath::Builder & NodeId(const CHIPNodeId aNodeId);
    AttributePath::Builder & EndpointId(const CHIPEndpointId aEndpointId);
    AttributePath::Builder & ClusterId(const uint32_t aClusterId);
    AttributePath::Builder & FieldTag(const uint8_t aFieldTag);
    AttributePath::Builder & ListIndex(const uint16_t aListIndex);

    AttributePath::Builder & EndOfAttributePath();

private:
    CHIP_ERROR _Init(chip::TLV::TLVWriter * const apWriter, const uint64_t aTagInApiForm);
};

}; // namespace AttributePath

/**
 *  @brief
 *    Chip Attribute Path List definition
 */
namespace AttributePathList {
class Parser : public ListParserBase
{
public:
    // Roughly verify the schema is right, including
    // 1) at least one element is there
    // 2) all elements are anonymous and of Path type
    // 3) every path is also valid in schema
    CHIP_ERROR CheckSchemaValidity() const;
};

class Builder : public ListBuilderBase
{
public:
    // Re-initialize the shared EventPathBuilder with anonymous tag
    AttributePath::Builder & CreateAttributePathBuilder();

    // Mark the end of this array and recover the type for outer container
    AttributePathList::Builder & EndOfAttributePathList();

private:
    AttributePath::Builder mAttributePathBuilder;
};
}; // namespace AttributePathList

/**
 *  @brief
 *    CHIP interaction model Event Path definition
 *
 */
namespace EventPath {
enum
{
    kCsTag_NodeId     = 0,
    kCsTag_EndpointId = 1,
    kCsTag_ClusterId  = 2,
    kCsTag_EventId    = 3,
};

/**
 *  @class Parser
 *
 *  @brief
 *    Parses a Event Path container
 */
class Parser : public ParserBase
{
public:
    // aReader has to be on the element of EventPath container
    CHIP_ERROR Init(const chip::TLV::TLVReader & aReader);

    // Roughly verify the schema is right, including
    // 1) all mandatory tags are present
    // 2) no unknown tags
    // 3) all elements have expected data type
    // 4) any tag can only appear once
    CHIP_ERROR CheckSchemaValidity() const;

    // CHIP_END_OF_TLV if there is no such element
    // CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
    CHIP_ERROR GetNodeId(CHIPNodeId * const apNodeId) const;

    // CHIP_END_OF_TLV if there is no such element
    // CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
    CHIP_ERROR GetEndpointId(CHIPEndpointId * const apEndpointId) const;

    // CHIP_END_OF_TLV if there is no such element
    // CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
    CHIP_ERROR GetClusterId(CHIPClusterId * const apClusterId) const;

    // CHIP_END_OF_TLV if there is no such element
    // CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
    CHIP_ERROR GetEventId(CHIPEventId * const apEventId) const;
};

class Builder : public BuilderBase
{
public:
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter);
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter, const uint8_t aContextTagToUse);

    EventPath::Builder & NodeId(const uint64_t aNodeId);
    EventPath::Builder & EndpointId(const CHIPEndpointId aEndpointId);
    EventPath::Builder & ClusterId(const uint32_t aClusterId);
    EventPath::Builder & EventId(const uint16_t aEventId);

    EventPath::Builder & EndOfEventPath();

private:
    CHIP_ERROR _Init(chip::TLV::TLVWriter * const apWriter, const uint64_t aTagInApiForm);
};
}; // namespace EventPath

/**
 *  @brief
 *    Chip Event Path List definition
 */
namespace EventPathList {
class Parser : public ListParserBase
{
public:
    // Roughly verify the schema is right, including
    // 1) at least one element is there
    // 2) all elements are anonymous and of Path type
    // 3) every path is also valid in schema
    CHIP_ERROR CheckSchemaValidity() const;
};

class Builder : public ListBuilderBase
{
public:
    // Re-initialize the shared EventPathBuilder with anonymous tag
    EventPath::Builder & CreateEventPathBuilder();

    // Mark the end of this array and recover the type for outer container
    EventPathList::Builder & EndOfEventPathList();

private:
    EventPath::Builder mEventPathBuilder;
};
}; // namespace EventPathList

/**
 *  @brief
 *    CHIP interaction model Command Path definition
 *
 */
namespace CommandPath {
    enum
    {
        kCsTag_EndpointId = 0,
        kCsTag_GroupId    = 1,
        kCsTag_ClusterId  = 2,
        kCsTag_CommandId  = 3,
    };

/**
 *  @class Parser
 *
 *  @brief
 *    Parses a Command Path container
 */
class Parser : public ParserBase
{
public:
    // aReader has to be on the element of EventPath container
    CHIP_ERROR Init(const chip::TLV::TLVReader & aReader);

    // Roughly verify the schema is right, including
    // 1) all mandatory tags are present
    // 2) no unknown tags
    // 3) all elements have expected data type
    // 4) any tag can only appear once
    CHIP_ERROR CheckSchemaValidity() const;

    // CHIP_END_OF_TLV if there is no such element
    // CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
    CHIP_ERROR GetEndpointId(CHIPEndpointId * const apEndpointId) const;

    // CHIP_END_OF_TLV if there is no such element
    // CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
    CHIP_ERROR GetGroupId(CHIPGroupId * const apGroupId) const;

    // CHIP_END_OF_TLV if there is no such element
    // CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
    CHIP_ERROR GetClusterId(CHIPClusterId * const apClusterId) const;

    // CHIP_END_OF_TLV if there is no such element
    // CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
    CHIP_ERROR GetCommandId(CHIPEventId * const apEventId) const;
};

class Builder : public BuilderBase
{
public:
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter);
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter, const uint8_t aContextTagToUse);

    CommandPath::Builder & EndpointId(const CHIPEndpointId aEndpointId);
    CommandPath::Builder & GroupId(const CHIPGroupId aGroupId);
    CommandPath::Builder & ClusterId(const CHIPClusterId aClusterId);
    CommandPath::Builder & CommandId(const uint16_t aCommandId);

    CommandPath::Builder & EndOfCommandPath();

private:
    CHIP_ERROR _Init(chip::TLV::TLVWriter * const apWriter, const uint64_t aTagInApiForm);
};
}; // namespace CommandPath

namespace EventDataElement {
enum
{
    kCsTag_EventPath       = 0,
    kCsTag_ImportanceLevel = 1,
    kCsTag_Number          = 2,
    kCsTag_UTCTimestamp    = 3,
    kCsTag_SystemTimestamp = 4,
    kCsTag_DeltaUTCTime    = 5,
    kCsTag_DeltaSystemTime = 6,
    kCsTag_Data            = 7,
};

class Parser : public ParserBase
{
public:
    CHIP_ERROR Init(const chip::TLV::TLVReader & aReader);

    // Roughly verify the schema is right
    CHIP_ERROR CheckSchemaValidity() const;

    CHIP_ERROR GetEventPath(EventPath::Parser * const apEventPath);
    CHIP_ERROR GetImportanceLevel(uint8_t * const apImportanceLevel);
    CHIP_ERROR GetNumber(uint64_t * const apNumber);
    CHIP_ERROR GetUTCTimestamp(uint64_t * const apUTCTimestamp);
    CHIP_ERROR GetSystemTimestamp(uint64_t * const apSystemTimestamp);
    CHIP_ERROR GetDeltaUTCTime(uint64_t * const apDeltaUTCTime);
    CHIP_ERROR GetDeltaSystemTime(uint64_t * const apDeltaSystemTime);
    CHIP_ERROR GetReaderOnEvent(chip::TLV::TLVReader * const apReader) const;
};

class Builder : public BuilderBase
{
public:
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter);

    EventPath::Builder & CreateEventPathBuilder();
    EventDataElement::Builder ImportanceLevel(const uint8_t aImportanceLevel);
    EventDataElement::Builder Number(const uint64_t aNumber);
    EventDataElement::Builder UTCTimestamp(const uint64_t aUTCTimestamp);
    EventDataElement::Builder SystemTimestamp(const uint64_t aSystemTimestamp);
    EventDataElement::Builder DeltaUTCTime(const uint64_t aDeltaUTCTime);
    EventDataElement::Builder DeltaSystemTime(const uint64_t aDeltaSystemTime);

    // Mark the end of this array and recover the type for outer container
    EventDataElement::Builder & EndOfEventDataElement();

private:
    EventPath::Builder mEventPathBuilder;
};
}; // namespace EventDataElement

namespace EventList {
class Parser : public ListParserBase
{
public:
    // Roughly verify the schema is right, including
    // 1) at least one element is there
    // 2) all elements are anonymous and of Structure type
    // 3) every Event is also valid in schema
    CHIP_ERROR CheckSchemaValidity() const;
};

class Builder : public ListBuilderBase
{
public:
    // Re-initialize the shared EventBuilder with anonymous tag
    EventDataElement::Builder & CreateEventBuilder();

    // Mark the end of this array and recover the type for outer container
    EventList::Builder & EndOfEventList();

private:
    EventDataElement::Builder mEventDataElementBuilder;
};
}; // namespace EventList

/**
 *  @brief
 *    Chip Status Code definition
 *
 */
namespace StatusCode {
enum
{
    kCsTag_ProtocolId   = 0,
    kCsTag_ClusterId    = 1,
    kCsTag_DetailedCode = 2,
    kCsTag_GeneralCode  = 3,
};

class Parser;
class Builder;

/**
 *  @brief
 *    Chip Attribute Status Element parser definition
 */
class Parser : public ParserBase
{
public:
    // aReader has to be on the element of DataElement
    CHIP_ERROR Init(const chip::TLV::TLVReader & aReader);

    // Roughly verify the schema is right, including
    // 1) all mandatory tags are present
    // 2) all elements have expected data type
    // 3) any tag can only appear once
    // At the top level of the structure, unknown tags are ignored for foward compatibility
    CHIP_ERROR CheckSchemaValidity() const;

    /**
    ` * Read the ProfileID and the StatusCode from the StatusCode.
     *
     * @param[out]   apProtocolId     Pointer to the storage for the ProtocolId
     *
     * @return       CHIP_ERROR codes returned by Chip::TLV objects. CHIP_END_OF_TLV if either
     *               element is missing. CHIP_ERROR_WRONG_TLV_TYPE if the elements are of the wrong
     *               type.
     */
    CHIP_ERROR GetProtocolId(uint32_t * apProtocolId) const;

    /**
    ` * Read the ClusterId from the StatusCode.
     *
     * @param[out]   apStatusCode     Pointer to the storage for the ClusterId
     *
     * @return       CHIP_ERROR codes returned by Chip::TLV objects. CHIP_END_OF_TLV if either
     *               element is missing. CHIP_ERROR_WRONG_TLV_TYPE if the elements are of the wrong
     *               type.
     */
    CHIP_ERROR GetClusterId(CHIPClusterId * apClusterId) const;

    /**
    ` * Read the detailed code from the StatusCode.
     *
     * @param[out]   apDetailedCode     Pointer to the storage for the Detailed code
     *
     * @return       CHIP_ERROR codes returned by Chip::TLV objects. CHIP_END_OF_TLV if either
     *               element is missing. CHIP_ERROR_WRONG_TLV_TYPE if the elements are of the wrong
     *               type.
     */
    CHIP_ERROR GetDetailedCode(uint16_t * apDetailedCode) const;

    /**
    ` * Read the general code from the StatusCode.
     *
     * @param[out]   apGeneralCode     Pointer to the storage for the general code
     *
     * @return       CHIP_ERROR codes returned by Chip::TLV objects. CHIP_END_OF_TLV if either
     *               element is missing. CHIP_ERROR_WRONG_TLV_TYPE if the elements are of the wrong
     *               type.
     */
    CHIP_ERROR GetGeneralCode(uint16_t * apGeneralCode) const;
};

/**
 *  @brief
 *    CHIP StatusCode encoder definition
 */
class Builder : public BuilderBase
{
public:
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter);
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter, const uint8_t aContextTagToUse);

    StatusCode::Builder & ProtocolId(const uint32_t aProtocolId);
    StatusCode::Builder & ClusterId(const uint32_t aClusterId);
    StatusCode::Builder & DetailedCode(const uint16_t aDetailedCode);
    StatusCode::Builder & GeneralCode(const uint16_t aGeneralCode);

    StatusCode::Builder & EndOfStatusCode();

private:
    CHIP_ERROR _Init(chip::TLV::TLVWriter * const apWriter, const uint64_t aTagInApiForm);
};
}; // namespace StatusCode

/**
 *  @brief
 *    Chip Attribute Status Element definition
 *
 */
namespace AttributeStatusElement {
enum
{
    kCsTag_AttributePath = 9,
    kCsTag_StatusCode    = 1,
};

/**
 *  @brief
 *    Chip Attribute Status Element builder definition
 */
class Builder : public BuilderBase
{
public:
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter);

    /**
     *  @brief Initialize a AttributePath::Builder for writing into the TLV stream
     *
     *  @return A reference to AttributePath::Builder
     */
    AttributePath::Builder & CreateAttributePathBuilder();

    StatusCode::Builder & CreateStatusCodeBuilder();

    AttributeStatusElement::Builder & EndOfAttributeStatusElement();

private:
    AttributePath::Builder mAttributePathBuilder;
    StatusCode::Builder mStatusCodeBuilder;
};

/**
 *  @brief
 *    Chip Attribute Status Element parser definition
 */
class Parser : public ParserBase
{
public:
    // aReader has to be on the element of DataElement
    CHIP_ERROR Init(const chip::TLV::TLVReader & aReader);

    // Roughly verify the schema is right, including
    // 1) all mandatory tags are present
    // 2) all elements have expected data type
    // 3) any tag can only appear once
    // At the top level of the structure, unknown tags are ignored for foward compatibility
    CHIP_ERROR CheckSchemaValidity() const;

    // CHIP_END_OF_TLV if there is no such element
    CHIP_ERROR GetAttributePath(AttributePath::Parser * const apAttributePath) const;

    // CHIP_END_OF_TLV if there is no such element
    CHIP_ERROR GetStatusCode(StatusCode::Parser * const apStatusCode) const;
};
}; // namespace AttributeStatusElement

namespace AttributeStatusList {
/**
 * StatusList builder.
 * Supports both the current and the deprecated StatusList format.
 */
class Builder : public ListBuilderBase
{
public:
    /**
     *  @brief Initialize a AttributeStatusEElement::Builder for writing into the TLV stream
     *
     *  @return A reference to AttributeStatusElement::Builder
     */
    AttributeStatusElement::Builder & CreateAttributeStatusBuilder();
    AttributeStatusList::Builder & EndOfStatusList();

private:
    AttributeStatusElement::Builder mAttributeStatusBuilder;
};

class Parser : public ListParserBase
{
public:
    CHIP_ERROR CheckSchemaValidity() const;
};
}; // namespace AttributeStatusList

/**
 *  @brief
 *    CHIP Attribute Data Element definition
 *
 */
namespace AttributeDataElement {
enum
{
    kCsTag_AttributePath  = 0,
    kCsTag_DataVersion    = 1,
    kCsTag_Data           = 2,
    kCsTag_MoreToComeFlag = 3,
};

/**
 *  @brief
 *    Chip Interaction Model Data Element parser definition
 */
class Parser : public ParserBase
{
public:
    // aReader has to be on the element of DataElement
    CHIP_ERROR Init(const chip::TLV::TLVReader & aReader);

    // Roughly verify the schema is right, including
    // 1) all mandatory tags are present
    // 2) all elements have expected data type
    // 3) any tag can only appear once
    // At the top level of the structure, unknown tags are ignored for foward compatibility
    CHIP_ERROR CheckSchemaValidity() const;

    // CHIP_END_OF_TLV if there is no such element
    // CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not a Path
    CHIP_ERROR GetAttributePath(AttributePath::Parser * const apPath) const;

    // CHIP_END_OF_TLV if there is no such element
    // CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
    CHIP_ERROR GetDataVersion(CHIPDataVersion * const apVersion) const;

    // Data could be of any type, so we can only position the reader so the caller has
    // full information of tag, element type, length, and value
    CHIP_ERROR GetData(chip::TLV::TLVReader * const apReader) const;

    // Default is false if there is no such element
    // CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not a boolean
    CHIP_ERROR GetMoreToComeFlag(bool * const apMoreToComeFlag) const;

protected:
    // A recursively callable function to parse a data element and pretty-print it.
    CHIP_ERROR ParseData(chip::TLV::TLVReader & aReader, int aDepth) const;
};

/**
 *  @brief
 *    WDM Data Element encoder definition
 */
class Builder : public BuilderBase
{
public:
    // DataElement is only used in a Data List, which requires every path to be anonymous
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter);

    AttributePath::Builder & CreateAttributePathBuilder();

    AttributeDataElement::Builder & DataVersion(const CHIPDataVersion aDataVersion);

    // Nothing would be written if MoreToCome == false, as that's the default value
    AttributeDataElement::Builder & MoreToCome(const bool aMoreToCome);

    AttributeDataElement::Builder & EndOfAttributeDataElement();

private:
    AttributePath::Builder mAttributePathBuilder;
};
}; // namespace AttributeDataElement

namespace AttributeDataList {
class Parser : public ListParserBase
{
public:
    // Roughly verify the schema is right, including
    // 1) at least one element is there
    // 2) all elements are anonymous and of Structure type
    // 3) every AttributeData Element is also valid in schema
    CHIP_ERROR CheckSchemaValidity() const;
};

class Builder : public ListBuilderBase
{
public:
    // Re-initialize the shared PathBuilder with anonymous tag
    AttributeDataElement::Builder & CreateAttributeDataElementBuilder();

    // Mark the end of this array and recover the type for outer container
    AttributeDataList::Builder & EndOfAttributeDataList();

private:
    AttributeDataElement::Builder mAttributeDataElementBuilder;
};
}; // namespace AttributeDataList

/**
 *  @brief
 *    CHIP Command Data Element definition
 *
 */
namespace CommandDataElement {
    enum
    {
        kCsTag_CommandPath  = 0,
        kCsTag_Data         = 1,
        kCsTag_StatusCode   = 2,
    };

/**
 *  @brief
 *    Chip Interaction Model Data Element parser definition
 */
class Parser : public ParserBase
{
public:
    // aReader has to be on the element of DataElement
    CHIP_ERROR Init(const chip::TLV::TLVReader & aReader);

    // Roughly verify the schema is right, including
    // 1) all mandatory tags are present
    // 2) all elements have expected data type
    // 3) any tag can only appear once
    // At the top level of the structure, unknown tags are ignored for foward compatibility
    CHIP_ERROR CheckSchemaValidity() const;

    // CHIP_END_OF_TLV if there is no such element
    // CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not a Path
    CHIP_ERROR GetCommandPath(CommandPath::Parser * const apPath) const;

    // Data could be of any type, so we can only position the reader so the caller has
    // full information of tag, element type, length, and value
    CHIP_ERROR GetData(chip::TLV::TLVReader * const apReader) const;

    // Default is false if there is no such element
    // CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not a boolean
    CHIP_ERROR GetStatusCode(StatusCode::Parser * const apStatusCode) const;

protected:
    // A recursively callable function to parse a data element and pretty-print it.
    CHIP_ERROR ParseData(chip::TLV::TLVReader & aReader, int aDepth) const;
};

/**
 *  @brief
 *    Chip data model Data Element encoder definition
 */
class Builder : public BuilderBase
{
public:
    // DataElement is only used in a Data List, which requires every path to be anonymous
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter);

    CommandPath::Builder & CreateCommandPathBuilder();
    StatusCode::Builder & CreateStatusCodeBuilder();

    CommandDataElement::Builder & EndOfCommandDataElement();

private:
    CommandPath::Builder mCommandPathBuilder;
    StatusCode::Builder mStatusCodeBuilder;
};
}; // namespace CommandDataElement

namespace CommandList {
class Parser : public ListParserBase
{
public:
    // Roughly verify the schema is right, including
    // 1) at least one element is there
    // 2) all elements are anonymous and of Structure type
    // 3) every AttributeData Element is also valid in schema
    CHIP_ERROR CheckSchemaValidity() const;
};

class Builder : public ListBuilderBase
{
public:
    CommandDataElement::Builder & CreateCommandDataElementBuilder();
    CommandList::Builder & EndOfCommandList();

private:
    CommandDataElement::Builder mCommandDataElementBuilder;
};
}; // namespace CommandList

namespace ReportDataRequest {
enum
{
    kCsTag_RequestResponse     = 0,
    kCsTag_SubscriptionId      = 1,
    kCsTag_AttributeStatusList = 2,
    kCsTag_AttributeDataList   = 3,
    kCsTag_EventDataList       = 4,
    kCsTag_IsLastReport        = 5,
};

class Parser : public ParserBase
{
public:
    // aReader has to be on the element of anonymous container
    CHIP_ERROR Init(const chip::TLV::TLVReader & aReader);

    // Roughly verify the schema is right, including
    // 1) all mandatory tags are present
    // 2) no unknown tags
    // 3) all elements have expected data type
    // 4) any tag can only appear once
    CHIP_ERROR CheckSchemaValidity() const;

    // CHIP_END_OF_TLV if there is no such element
    // CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not one of the right types
    CHIP_ERROR GetRequestResponse(bool * const apRequestResponse) const;

    // Get a TLVReader for the Paths. Next() must be called before accessing them.
    CHIP_ERROR GetSubscriptionID(uint64_t * const apSubscriptionID) const;

    // Get a TLVReader for the AttributesStatusList. Next() must be called before accessing them.
    CHIP_ERROR GetAttributeStatusList(AttributeStatusList::Parser * const apAttributeStatusList) const;

    // Get a TLVReader for the AttributesDataList. Next() must be called before accessing them.
    CHIP_ERROR GetAttributeDataList(AttributeDataList::Parser * const apAttributeDataList) const;

    // Get a TLVReader for the events. Next() must be called before accessing them.
    CHIP_ERROR GetEventDataList(EventList::Parser * const apEventDataList) const;

    // CHIP_END_OF_TLV if there is no such element
    // CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not one of the right types
    CHIP_ERROR GetIsLastReport(bool * const apIsLastReport) const;
};
}; // namespace ReportDataRequest
/**
 *  @brief
 *    WDM Custom Command definition
 *
 */
namespace InvokeCommandRequest {

/// @brief Context-Specific tags used in this message
    enum
    {
        kCsTag_CommandList            = 0,
    };

/**
 *  @brief
 *     InvokeCommandRequest Request parser definition
 */
class Parser : public ParserBase
{
public:
    /**
     *  @brief Initialize the parser object with TLVReader
     *
     *  @param [in] aReader A pointer to a TLVReader, which should point to the beginning of this request
     *
     *  @retval #WEAVE_NO_ERROR on success
     */
    CHIP_ERROR Init(const chip::TLV::TLVReader & aReader);

    /**
     *  @brief Roughly verify the message is correctly formed
     *
     *  @note The main use of this function is to print out what we're
     *    receiving during protocol development and debugging.
     *    The encoding rule has changed in WDM Next so this
     *    check is only "roughly" conformant now.
     *
     *  @retval #CHIP_NO_ERROR on success
     */
    CHIP_ERROR CheckSchemaValidity() const;

    // Get a TLVReader for the commands. Next() must be called before accessing them.
    CHIP_ERROR GetCommandList(CommandList::Parser * const apCommandList) const;
};

/**
 *  @brief
 *    Interaction Model InvokeCommandRequest encoder definition
 *
 *  The argument, and the authenticator elements are not directly supported, as they do not have a fixed schema.
 */
class Builder : public BuilderBase
{
public:
    /**
     *  @brief Initialize a InvokeCommandRequest::Builder for writing into a TLV stream
     *
     *  @param [in] apWriter    A pointer to TLVWriter
     *
     *  @retval #CHIP_NO_ERROR on success
     */
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter);

    /**
     *  @brief Initialize a CommandList::Builder for writing into the TLV stream
     *
     *  @return A reference to Path::Builder
     */
    CommandList::Builder & CreateCommandListBuilder();

    /**
     *  @brief Mark the end of this InvokeCommandRequest
     *
     *  @return A reference to *this
     */
    InvokeCommandRequest::Builder & EndOfInvokeCommandRequest();

private:
    CommandList::Builder mCommandListBuilder;
};
}; // namespace InvokeCommandRequest

}; // namespace app
}; // namespace chip

#endif // _CHIP_INTERACTION_MODEL_MESSAGE_DEF_H
