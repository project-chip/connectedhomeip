/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <meter-identification-instance.h>
#include <app/util/af-types.h>

using namespace chip::app::Clusters;
using namespace chip::app::Clusters::MeterIdentification;

namespace {
static std::unique_ptr<Instance> gMeterIdentificationCluster;

CHIP_ERROR JsonToPowerThreshold(const Json::Value & root, Globals::Structs::PowerThresholdStruct::Type & value)
{
    Json::Value t;
    if (root.isMember("PowerThreshold"))
    {
        t = root.get("PowerThreshold", Json::Value());
        if (t.empty() || !t.isInt64())
        {
            return CHIP_ERROR_DECODE_FAILED;
        }

        value.powerThreshold = chip::Optional<int64_t>(t.asUInt());
    }

    if (root.isMember("ApparentPowerThreshold"))
    {
        t = root.get("ApparentPowerThreshold", Json::Value());
        if (t.empty() || !t.isInt64())
        {
            return CHIP_ERROR_DECODE_FAILED;
        }

        value.apparentPowerThreshold = chip::Optional<int64_t>(t.asUInt());
    }

    if (root.isMember("PowerThresholdSource"))
    {
        t = root.get("PowerThresholdSource", Json::Value());
        if (t.empty() || !t.isUInt())
        {
            return CHIP_ERROR_DECODE_FAILED;
        }

        value.powerThresholdSource.SetNonNull(static_cast<Globals::PowerThresholdSourceEnum>(t.asUInt()));
    }

    return CHIP_NO_ERROR;
}
} // namespace

Instance * MeterIdentification::GetInstance()
{
    return gMeterIdentificationCluster.get();
}

void MeterIdentification::Shutdown()
{
    VerifyOrDie(gMeterIdentificationCluster);
    gMeterIdentificationCluster->Shutdown();
    gMeterIdentificationCluster.reset(nullptr);
}

void emberAfMeterIdentificationClusterInitCallback(chip::EndpointId endpointId)
{
    ChipLogProgress(Zcl, "emberAfMeterIdentificationClusterInitCallback %d", static_cast<int>(endpointId));
    VerifyOrDie(1 == endpointId); // this cluster is only enabled for endpoint 1.
    VerifyOrDie(!gMeterIdentificationCluster);
    gMeterIdentificationCluster = std::make_unique<Instance>(1, chip::BitMask<Feature, uint32_t>(Feature::kPowerThreshold));
    gMeterIdentificationCluster->Init();
}

CHIP_ERROR MeterIdentification::LoadJson(const Json::Value & root)
{
    VerifyOrDie(gMeterIdentificationCluster);
    CHIP_ERROR ret = CHIP_NO_ERROR;
    Json::Value value;

    if (root.isMember("Features"))
    {
        value = root.get("Features", Json::Value());
        if (value.isUInt())
        {
            if(1 >= value.asUInt())
            {
                gMeterIdentificationCluster.reset(nullptr);
                gMeterIdentificationCluster = std::make_unique<Instance>(1, chip::BitMask<Feature, uint32_t>(Feature::kPowerThreshold));
                gMeterIdentificationCluster->Init();
                return ret;
            }
        }
        else
        {
            ret = CHIP_ERROR_DECODE_FAILED;
        }
    }

    if (root.isMember("MeterType"))
    {
        value = root.get("MeterType", Json::Value());
        if(value.isUInt())
        {
            ret = CHIP_NO_ERROR == ret ? gMeterIdentificationCluster->SetMeterType(chip::app::DataModel::MakeNullable(static_cast<MeterTypeEnum>(value.asUInt()))) : ret;
        }
        else
        {
            ret = CHIP_ERROR_DECODE_FAILED;
        }
    }

    if (root.isMember("PointOfDelivery"))
    {
        value = root.get("PointOfDelivery", Json::Value());
        if(value.isString())
        {
            ret = CHIP_NO_ERROR == ret ? gMeterIdentificationCluster->SetPointOfDelivery(chip::CharSpan::fromCharString(value.asCString())) : ret;
        }
        else
        {
            ret = CHIP_ERROR_DECODE_FAILED;
        }
    }

    if (root.isMember("MeterSerialNumber"))
    {
        value = root.get("MeterSerialNumber", Json::Value());
        if(value.isString())
        {
            ret = CHIP_NO_ERROR == ret ? gMeterIdentificationCluster->SetMeterSerialNumber(chip::CharSpan::fromCharString(value.asCString())) : ret;
        }
        else
        {
            ret = CHIP_ERROR_DECODE_FAILED;
        }
    }

    if (root.isMember("ProtocolVersion"))
    {
        value = root.get("ProtocolVersion", Json::Value());
        if(value.isString())
        {
            ret = CHIP_NO_ERROR == ret ? gMeterIdentificationCluster->SetProtocolVersion(chip::CharSpan::fromCharString(value.asCString())) : ret;
        }
        else
        {
            ret = CHIP_ERROR_DECODE_FAILED;
        }
    }

    if (root.isMember("PowerThreshold"))
    {
        value = root.get("PowerThreshold", Json::Value());
        if (!value.empty() && value.isObject())
        {
            Globals::Structs::PowerThresholdStruct::Type powerThreshopld;
            const CHIP_ERROR error = JsonToPowerThreshold(value, powerThreshopld);
            if (CHIP_NO_ERROR == error)
            {
                ret = CHIP_NO_ERROR == ret ? gMeterIdentificationCluster->SetPowerThreshold(chip::app::DataModel::MakeNullable(powerThreshopld)) : ret;
            }
            else
            {
                ret = error;
            }
        }
        else
        {
            ret = CHIP_ERROR_DECODE_FAILED;
        }
    }

    return ret;
}
