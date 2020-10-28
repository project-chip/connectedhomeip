/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/*
 *
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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
#include <support/logging/CHIPLogging.h>
#include <support/DLLUtil.h>

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
    kMsgType_SubscribeRequest                   = 0x01,
    kMsgType_ReadRequest                        = 0x02,
    kMsgType_ReportData                         = 0x03,
    kMsgType_WriteRequest                       = 0x04,
    kMsgType_WriteResponse                      = 0x05,
    kMsgType_InvokeCommandRequest               = 0x06,
    kMsgType_InvokeCommandResponse              = 0x07,
};

static inline bool IsVersionNewer(const uint64_t &aVersion, const uint64_t &aReference)
{
    return (aVersion != aReference);
}

static inline bool IsVersionNewerOrEqual(const uint64_t &aVersion, const uint64_t &aReference)
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

    ParserBase(void);

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
    ListParserBase(void);

public:
    // aReader has to be on the element of the array element
    CHIP_ERROR Init(const chip::TLV::TLVReader & aReader);

    // aReader has to be at the beginning of some container
    CHIP_ERROR InitIfPresent(const chip::TLV::TLVReader & aReader, const uint8_t aContextTagToFind);

    CHIP_ERROR Next(void);
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
    void ResetError(void);
    void ResetError(CHIP_ERROR aErr);
    CHIP_ERROR GetError(void) const { return mError; };
    chip::TLV::TLVWriter * GetWriter(void) { return mpWriter; };

protected:
    CHIP_ERROR mError;
    chip::TLV::TLVWriter * mpWriter;
    chip::TLV::TLVType mOuterContainerType;

    BuilderBase(void);
    void EndOfContainer(void);

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
    ListBuilderBase(void);

public:
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter, const uint8_t aContextTagToUse);
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter);
};

/**
*  @brief
*    CHIP interaction model Event Path definition
*
*/
namespace AttributePath {
    enum
    {
        kCsTag_EndpointId      = 0,
        kCsTag_ClusterId       = 1,
        kCsTag_FieldTag        = 2,
        kCsTag_ListIndex       = 3,
    };

    class Parser;
    class Builder;
}; // namespace AttributePath

/**
*  @class Parser
*
*  @brief
*    Parses a Attribute Path container
*/

class AttributePath::Parser : public ParserBase
{
public:
    // aReader has to be on the element of EventPath container
    CHIP_ERROR Init(const chip::TLV::TLVReader & aReader);

    // Roughly verify the schema is right, including
    // 1) all mandatory tags are present
    // 2) no unknown tags
    // 3) all elements have expected data type
    // 4) any tag can only appear once
    CHIP_ERROR CheckSchemaValidity(void) const;

    // CHIP_END_OF_TLV if there is no such element
    // CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
    CHIP_ERROR GetEndpointId(uint32_t * const apEndpointId) const;

    // CHIP_END_OF_TLV if there is no such element
    // CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
    CHIP_ERROR GetClusterId(uint32_t * const apClusterId) const;

    // CHIP_END_OF_TLV if there is no such element
    // CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
    CHIP_ERROR GetFieldTag(uint8_t * const apEventId) const;

    // CHIP_END_OF_TLV if there is no such element
    // CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
    CHIP_ERROR GetListIndex(uint8_t * const apListIndex) const;

    // Get a TLVReader at the additional tags section. Next() must be called before accessing it.
    CHIP_ERROR GetTags(chip::TLV::TLVReader * const apReader) const;
};

class AttributePath::Builder : public BuilderBase
{
public:
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter);

    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter, const uint8_t aContextTagToUse);

    AttributePath::Builder & EndpointId(const uint32_t aEndpointId);
    AttributePath::Builder & ClusterId(const uint32_t aClusterId);
    AttributePath::Builder & FieldTag(const uint8_t aFieldTag);
    AttributePath::Builder & ListIndex(const uint16_t aEndpointId);

    AttributePath::Builder & EndOfAttributePath(void);

private:
    CHIP_ERROR _Init(chip::TLV::TLVWriter * const apWriter, const uint64_t aTagInApiForm);
};

/**
*  @brief
*    Chip Event Path List definition
*/
namespace AttributePathList {
    class Parser;
    class Builder;
}; // namespace PathList

class AttributePathList::Parser : public ListParserBase
{
public:
    // Roughly verify the schema is right, including
    // 1) at least one element is there
    // 2) all elements are anonymous and of Path type
    // 3) every path is also valid in schema
    CHIP_ERROR CheckSchemaValidity(void) const;
};

class AttributePathList::Builder : public ListBuilderBase
{
public:
    // Re-initialize the shared EventPathBuilder with anonymous tag
    AttributePath::Builder & CreateAttributePathBuilder(void);

    // Mark the end of this array and recover the type for outer container
    AttributePathList::Builder & EndOfAttributePathList(void);

private:
    AttributePath::Builder mAttributePathBuilder;
};

/**
 *  @brief
 *    CHIP interaction model Event Path definition
 *
 */
namespace EventPath {
    enum
    {
        kCsTag_EndpointId      = 0,
        kCsTag_ClusterId       = 1,
        kCsTag_EventId         = 2,
    };

    class Parser;
    class Builder;
}; // namespace EventPath

/**
 *  @class Parser
 *
 *  @brief
 *    Parses a Event Path container
 */
class EventPath::Parser : public ParserBase
{
public:
    // aReader has to be on the element of EventPath container
    CHIP_ERROR Init(const chip::TLV::TLVReader & aReader);

    // Roughly verify the schema is right, including
    // 1) all mandatory tags are present
    // 2) no unknown tags
    // 3) all elements have expected data type
    // 4) any tag can only appear once
    CHIP_ERROR CheckSchemaValidity(void) const;

    // CHIP_END_OF_TLV if there is no such element
    // CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
    CHIP_ERROR GetEndpointId(uint32_t * const apEndpointId) const;

    // CHIP_END_OF_TLV if there is no such element
    // CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
    CHIP_ERROR GetClusterId(uint32_t * const apClusterId) const;

    // CHIP_END_OF_TLV if there is no such element
    // CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
    CHIP_ERROR GetEventId(uint16_t * const apEventId) const;
};

class EventPath::Builder : public BuilderBase
{
public:
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter);

    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter, const uint8_t aContextTagToUse);

    EventPath::Builder & EndpointId(const uint32_t aEndpointId);
    EventPath::Builder & ClusterId(const uint32_t aClusterId);
    EventPath::Builder & EventId(const uint16_t aEndpointId);

    EventPath::Builder & EndOfEventPath(void);

private:
    CHIP_ERROR _Init(chip::TLV::TLVWriter * const apWriter, const uint64_t aTagInApiForm);
};

/**
 *  @brief
 *    Chip Event Path List definition
 */
namespace EventPathList {
class Parser;
class Builder;
}; // namespace PathList

class EventPathList::Parser : public ListParserBase
{
public:
    // Roughly verify the schema is right, including
    // 1) at least one element is there
    // 2) all elements are anonymous and of Path type
    // 3) every path is also valid in schema
    CHIP_ERROR CheckSchemaValidity(void) const;
};

class EventPathList::Builder : public ListBuilderBase
{
public:
    // Re-initialize the shared EventPathBuilder with anonymous tag
    EventPath::Builder & CreateEventPathBuilder(void);

    // Mark the end of this array and recover the type for outer container
    EventPathList::Builder & EndOfEventPathList(void);

private:
    EventPath::Builder mEventPathBuilder;
};

namespace EventDataElement {
enum
{
    kCsTag_EventPath        = 0,
    kCsTag_ImportanceLevel  = 1,
    kCsTag_Number           = 2,
    kCsTag_UTCTimestamp     = 3,
    kCsTag_SystemTimestamp  = 4,
    kCsTag_DeltaUTCTime     = 5,
    kCsTag_DeltaSystemTime  = 6,
    kCsTag_Data             = 7,
};

class Parser;
class Builder;
}; // namespace EventDataElement

class EventDataElement::Parser : public ParserBase
{
public:
    CHIP_ERROR Init(const chip::TLV::TLVReader & aReader);

    // Roughly verify the schema is right
    CHIP_ERROR CheckSchemaValidity(void) const;

    CHIP_ERROR GetEventPath(EventPath::Parser * const apEventPath);
    CHIP_ERROR GetImportanceLevel(uint8_t * const apImportanceLevel);
    CHIP_ERROR GetNumber(uint64_t * const apNumber);
    CHIP_ERROR GetUTCTimestamp(uint64_t * const apUTCTimestamp);
    CHIP_ERROR GetSystemTimestamp(uint64_t * const apSystemTimestamp);
    CHIP_ERROR GetDeltaUTCTime(uint64_t * const apDeltaUTCTime);
    CHIP_ERROR GetDeltaSystemTime(uint64_t * const apDeltaSystemTime);
    CHIP_ERROR GetReaderOnEvent(chip::TLV::TLVReader * const apReader) const;
};

class EventDataElement::Builder : public BuilderBase
{
public:
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter);

    EventPath::Builder & CreateEventPathBuilder(void);
    EventDataElement::Builder ImportanceLevel(const uint8_t aImportanceLevel);
    EventDataElement::Builder Number(const uint64_t aNumber);
    EventDataElement::Builder UTCTimestamp(const uint64_t aUTCTimestamp);
    EventDataElement::Builder SystemTimestamp(const uint64_t aSystemTimestamp);
    EventDataElement::Builder DeltaUTCTime(const uint64_t aDeltaUTCTime);
    EventDataElement::Builder DeltaSystemTime(const uint64_t aDeltaSystemTime);

    // Mark the end of this array and recover the type for outer container
    EventDataElement::Builder & EndOfEventDataElement(void);
private:
    EventPath::Builder mEventPathBuilder;
};

namespace EventList {
class Parser;
class Builder;
}; // namespace EventList

class EventList::Parser : public ListParserBase
{
public:
    // Roughly verify the schema is right, including
    // 1) at least one element is there
    // 2) all elements are anonymous and of Structure type
    // 3) every Event is also valid in schema
    CHIP_ERROR CheckSchemaValidity(void) const;
};

class EventList::Builder : public ListBuilderBase
{
public:
    // Re-initialize the shared EventBuilder with anonymous tag
    EventDataElement::Builder & CreateEventBuilder(void);

    // Mark the end of this array and recover the type for outer container
    EventList::Builder & EndOfEventList(void);

private:
    EventDataElement::Builder mEventDataElementBuilder;
};

/**
 *  @brief
 *    Chip Status Code definition
 *
 */
namespace StatusCode {
    enum
    {
        kCsTag_ProtocolId         = 0,
        kCsTag_ClusterId          = 1,
        kCsTag_DetailedCode       = 2,
        kCsTag_GeneralCode        = 3,
    };

    class Parser;
    class Builder;
}; // namespace StatusCode

/**
*  @brief
*    Chip Attribute Status Element parser definition
*/
class StatusCode::Parser : public ParserBase
{
public:
    // aReader has to be on the element of DataElement
    CHIP_ERROR Init(const chip::TLV::TLVReader & aReader);

    // Roughly verify the schema is right, including
    // 1) all mandatory tags are present
    // 2) all elements have expected data type
    // 3) any tag can only appear once
    // At the top level of the structure, unknown tags are ignored for foward compatibility
    CHIP_ERROR CheckSchemaValidity(void) const;

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
    CHIP_ERROR GetClusterId(uint32_t * apClusterId) const;

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
class StatusCode::Builder : public BuilderBase
{
public:
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter);

    StatusCode::Builder & ProtocolId(const uint32_t aProtocolId);
    StatusCode::Builder & ClusterId(const uint32_t aClusterId);
    StatusCode::Builder & DetailedCode(const uint16_t aDetailedCode);
    StatusCode::Builder & GeneralCode(const uint16_t aGeneralCode);

    StatusCode::Builder & EndOfStatusCode(void);
};

/**
*  @brief
*    Chip Attribute Status Element definition
*
*/
namespace AttributeStatusElement {
    enum
    {
        kCsTag_AttributePath         = 9,
        kCsTag_StatusCode            = 1,
    };

    class Parser;
    class Builder;
}; // namespace AttributeStatusElement

/**
*  @brief
*    Chip Attribute Status Element builder definition
*/
class AttributeStatusElement::Builder : public BuilderBase
{
public:
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter);

    /**
     *  @brief Initialize a AttributePath::Builder for writing into the TLV stream
     *
     *  @return A reference to AttributePath::Builder
     */
    AttributePath::Builder & CreateAttributePathBuilder(void);

    StatusCode::Builder & CreateStatusCodeBuilder(void);

    AttributeStatusElement::Builder & EndOfAttributeStatusElement(void);

private:
    AttributePath::Builder mAttributePathBuilder;
    StatusCode::Builder mStatusCodeBuilder;
};

/**
*  @brief
*    Chip Attribute Status Element parser definition
*/
class AttributeStatusElement::Parser : public ParserBase
{
public:
    // aReader has to be on the element of DataElement
    CHIP_ERROR Init(const chip::TLV::TLVReader & aReader);

    // Roughly verify the schema is right, including
    // 1) all mandatory tags are present
    // 2) all elements have expected data type
    // 3) any tag can only appear once
    // At the top level of the structure, unknown tags are ignored for foward compatibility
    CHIP_ERROR CheckSchemaValidity(void) const;

    // CHIP_END_OF_TLV if there is no such element
    CHIP_ERROR GetAttributePath(AttributePath::Parser * const apAttributePath) const;

    // CHIP_END_OF_TLV if there is no such element
    CHIP_ERROR GetStatusCode(StatusCode::Parser * const apStatusCode) const;
};

namespace AttributeStatusList {
    class Parser;
    class Builder;
}; // namespace AttributeStatusList

/**
* StatusList builder.
* Supports both the current and the deprecated StatusList format.
*/
class AttributeStatusList::Builder : public ListBuilderBase
{
public:
    /**
     *  @brief Initialize a AttributeStatusEElement::Builder for writing into the TLV stream
     *
     *  @return A reference to AttributeStatusElement::Builder
     */
    AttributeStatusElement::Builder & CreateAttributeStatusBuilder(void);
    AttributeStatusList::Builder & EndOfStatusList(void);

private:
    AttributeStatusElement::Builder mAttributeStatusBuilder;
};

class AttributeStatusList::Parser : public ListParserBase
{
public:
    CHIP_ERROR CheckSchemaValidity(void) const;
};


/**
 *  @brief
 *    CHIP Attribute Data Element definition
 *
 */
namespace AttributeDataElement {
    enum
    {
        kCsTag_AttributePath       = 0,
        kCsTag_DataVersion         = 1,
        kCsTag_Data                = 2,
        kCsTag_MoreToComeFlag      = 3,
    };

    class Parser;
    class Builder;
}; // namespace DataElement

/**
 *  @brief
 *    WDM Data Element parser definition
 */
class AttributeDataElement::Parser : public ParserBase
{
public:
    // aReader has to be on the element of DataElement
    CHIP_ERROR Init(const chip::TLV::TLVReader & aReader);

    // Roughly verify the schema is right, including
    // 1) all mandatory tags are present
    // 2) all elements have expected data type
    // 3) any tag can only appear once
    // At the top level of the structure, unknown tags are ignored for foward compatibility
    CHIP_ERROR CheckSchemaValidity(void) const;

    // CHIP_END_OF_TLV if there is no such element
    // CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not a Path
    CHIP_ERROR GetAttributePath(AttributePath::Parser * const apPath) const;

    // CHIP_END_OF_TLV if there is no such element
    // CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
    CHIP_ERROR GetDataVersion(uint64_t * const apVersion) const;

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
class AttributeDataElement::Builder : public BuilderBase
{
public:
    // DataElement is only used in a Data List, which requires every path to be anonymous
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter);

    AttributePath::Builder & CreateAttributePathBuilder(void);

    AttributeDataElement::Builder & DataVersion(const uint64_t aDataVersion);

    // Nothing would be written if MoreToCome == false, as that's the default value
    AttributeDataElement::Builder & MoreToCome(const bool aMoreToCome);

    AttributeDataElement::Builder & EndOfAttributeDataElement(void);

private:
    AttributePath::Builder mAttributePathBuilder;
};

namespace AttributeDataList {
    class Parser;
    class Builder;
}; // namespace AttributeDataList

class AttributeDataList::Parser : public ListParserBase
{
public:
    // Roughly verify the schema is right, including
    // 1) at least one element is there
    // 2) all elements are anonymous and of Structure type
    // 3) every AttributeData Element is also valid in schema
    CHIP_ERROR CheckSchemaValidity(void) const;
};

class AttributeDataList::Builder : public ListBuilderBase
{
public:
    // Re-initialize the shared PathBuilder with anonymous tag
    AttributeDataElement::Builder & CreateAttributeDataElementBuilder(void);

    // Mark the end of this array and recover the type for outer container
    AttributeDataList::Builder & EndOfAttributeDataList(void);

private:
    AttributeDataElement::Builder mAttributeDataElementBuilder;
};

namespace ReportDataRequest {
enum
{
    kCsTag_RequestResponse          = 0,
    kCsTag_SubscriptionId           = 1,
    kCsTag_AttributeStatusList      = 2,
    kCsTag_AttributeDataList        = 3,
    kCsTag_EventDataList            = 4,
    kCsTag_IsLastReport             = 5,
};

class Parser;
}; // namespace ReportDataRequest

class ReportDataRequest::Parser : public ParserBase
{
public:
    // aReader has to be on the element of anonymous container
    CHIP_ERROR Init(const chip::TLV::TLVReader & aReader);

    // Roughly verify the schema is right, including
    // 1) all mandatory tags are present
    // 2) no unknown tags
    // 3) all elements have expected data type
    // 4) any tag can only appear once
    CHIP_ERROR CheckSchemaValidity(void) const;

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

}; // app
}; // namespace chip

#endif // _CHIP_INTERACTION_MODEL_MESSAGE_DEF_H
