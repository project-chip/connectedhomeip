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

#include <app-common/app-common/zap-generated/enums.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/CommandHandler.h>
#include <app/util/af-event.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <lib/support/logging/CHIPLogging.h>
#include <system/SystemClock.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

struct TimeSyncTimeSource
{
    uint64_t GetUtcTime(void) { return (GetLocalTime() - mTimeStamp) + mTimeBase; }
    uint64_t GetLocalTime(void) { return System::Clock::GetMonotonicMicroseconds(); }
    void SetTimeBase(uint64_t timeBase) { 
        mTimeBase = timeBase;
        mTimeStamp = GetLocalTime();
    }

private:
    uint64_t mTimeBase = 0; // TODO store TimeBase in Flash ?
    uint64_t mTimeStamp = 0;
};

namespace {

TimeSyncTimeSource gTimeSource;

class TimeSyncAttrAccess : public AttributeAccessInterface
{
public:
    // Register for the TimeSync cluster on all endpoints.
    TimeSyncAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), TimeSync::Id) {}
    CHIP_ERROR Read(ClusterInfo & aClusterInfo, const AttributeValueEncoder & aEncoder, bool * aDataRead) override;
};

TimeSyncAttrAccess gAttrAccess;

CHIP_ERROR TimeSyncAttrAccess::Read(ClusterInfo & aClusterInfo, const AttributeValueEncoder & aEncoder, bool * aDataRead)
{
    if (aClusterInfo.mClusterId != TimeSync::Id)
    {
        // We shouldn't have been called at all.
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    *aDataRead = true;
    switch (aClusterInfo.mFieldId)
    {
    case TimeSync::Attributes::UTCTime::Id: {
        uint64_t utcTime = gTimeSource.GetUtcTime();
        return aEncoder.Encode(utcTime);
        break;
    }
    case TimeSync::Attributes::LocalTime::Id: {
        uint64_t localTime = gTimeSource.GetLocalTime();
        return aEncoder.Encode(localTime);
        break;
    }
    default: {
        *aDataRead = false;
        break;
    }
    }

    return CHIP_NO_ERROR;
}
} // anonymous namespace

bool emberAfTimeSyncClusterSetUtcTimeCallback(chip::EndpointId endpoint, chip::app::CommandHandler * commandObj, uint64_t UtcTime,
                                              uint8_t Granularity, uint8_t TimeSource)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    uint8_t granularity  = EMBER_ZCL_TIME_SYNC_GRANULARITY_NO_TIME_GRANULARITY;

    if (EMBER_ZCL_STATUS_SUCCESS == TimeSync::Attributes::SetGranularity(endpoint, granularity) &&
        (EMBER_ZCL_TIME_SYNC_GRANULARITY_NO_TIME_GRANULARITY == granularity || Granularity <= granularity))
    {
        gTimeSource.SetTimeBase(UtcTime);
        status = TimeSync::Attributes::SetGranularity(endpoint, Granularity);
        (void) TimeSync::Attributes::SetTimeSource(endpoint, TimeSource);
    }
    else
    {
        status = EMBER_ZCL_STATUS_FAILURE;
    }

    return EMBER_SUCCESS == emberAfSendImmediateDefaultResponse(status);
}

void emberAfTimeSyncClusterServerInitCallback(EndpointId endpoint)
{
    static bool attrAccessRegistered = false;
    if (!attrAccessRegistered)
    {
        attrAccessRegistered = true;
        registerAttributeAccessOverride(&gAttrAccess);

        EmberAfTimeSyncGranularity granularity = EMBER_ZCL_TIME_SYNC_GRANULARITY_NO_TIME_GRANULARITY;
        (void) TimeSync::Attributes::SetGranularity(endpoint, (uint8_t) granularity);
    }
}
