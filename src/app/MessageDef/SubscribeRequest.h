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

#pragma once

#include "AttributeDataVersionList.h"
#include "AttributePathList.h"
#include "Builder.h"
#include "EventPathList.h"
#include "Parser.h"
#include <app/AppBuildConfig.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPTLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {
namespace SubscribeRequest {
enum
{
    kCsTag_AttributePathList         = 0,
    kCsTag_EventPathList             = 1,
    kCsTag_AttributeDataVersionList  = 2,
    kCsTag_EventNumber               = 3,
    kCsTag_MinIntervalSeconds        = 4,
    kCsTag_MaxIntervalSeconds        = 5,
    kCsTag_KeepExistingSubscriptions = 6,
    kCsTag_IsProxy                   = 7,
};

class Parser : public chip::app::Parser
{
public:
    /**
     *  @param [in] aReader A pointer to a TLVReader, which should point to the beginning of this request
     */
    CHIP_ERROR Init(const chip::TLV::TLVReader & aReader);
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
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
     */
    CHIP_ERROR CheckSchemaValidity() const;
#endif

    /**
     *  @brief Get a TLVReader for the AttributePathList. Next() must be called before accessing them.
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetAttributePathList(AttributePathList::Parser * const apAttributePathList) const;

    /**
     *  @brief Get a TLVReader for the EventPathList. Next() must be called before accessing them.
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetEventPathList(EventPathList::Parser * const apEventPathList) const;

    /**
     *  @brief Get a parser for the AttributeDataVersionList. Next() must be called before accessing them.
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetAttributeDataVersionList(AttributeDataVersionList::Parser * const apAttributeDataVersionList) const;

    /**
     *  @brief Get Event Number. Next() must be called before accessing them.
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetEventNumber(uint64_t * const apEventNumber) const;

    /**
     *  @brief Get MinIntervalSeconds. Next() must be called before accessing them.
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetMinIntervalSeconds(uint16_t * const apMinIntervalSeconds) const;

    /**
     *  @brief Get MaxIntervalSeconds. Next() must be called before accessing them.
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetMaxIntervalSeconds(uint16_t * const apMaxIntervalSeconds) const;

    /**
     *  @brief Check if subscription is kept. Next() must be called before accessing them.
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetKeepExistingSubscriptions(bool * const apKeepExistingSubscription) const;

    /**
     *  @brief Check if subscription is kept. Next() must be called before accessing them.
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetIsProxy(bool * const apIsProxy) const;
};

class Builder : public chip::app::Builder
{
public:
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter);

    AttributePathList::Builder & CreateAttributePathListBuilder();

    /**
     *  @brief Initialize a EventPathList::Builder for writing into the TLV stream
     */
    EventPathList::Builder & CreateEventPathListBuilder();

    /**
     *  @brief Initialize a AttributeDataVersionList::Builder for writing into the TLV stream
     */
    AttributeDataVersionList::Builder & CreateAttributeDataVersionListBuilder();

    /**
     *  @brief An initiator can optionally specify an EventNumber it has already to limit the
     *  set of retrieved events on the server for optimization purposes.
     */
    SubscribeRequest::Builder & EventNumber(const uint64_t aEventNumber);

    SubscribeRequest::Builder & MinIntervalSeconds(const uint16_t aMinIntervalSeconds);

    SubscribeRequest::Builder & MaxIntervalSeconds(const uint16_t aMinIntervalSeconds);

    /**
     *  @brief This is set to 'true' by the subscriber to indicate preservation of previous subscriptions. If omitted, it implies
     * 'false' as a value.
     */
    SubscribeRequest::Builder & KeepExistingSubscriptions(const bool aKeepExistingSubscriptions);

    /**
     *  @brief This is set to true by the subscriber if it is a proxy-type device proxying for another client. This
     *  confers it special privileges on the publisher that might result in evictions of other non-proxy subscriptions
     *  to make way for the proxy.
     */
    SubscribeRequest::Builder & IsProxy(const bool aIsProxy);

    /**
     *  @brief Mark the end of this SubscribeRequest
     */
    SubscribeRequest::Builder & EndOfSubscribeRequest();

private:
    AttributePathList::Builder mAttributePathListBuilder;
    EventPathList::Builder mEventPathListBuilder;
    AttributeDataVersionList::Builder mAttributeDataVersionListBuilder;
};
} // namespace SubscribeRequest
} // namespace app
} // namespace chip
