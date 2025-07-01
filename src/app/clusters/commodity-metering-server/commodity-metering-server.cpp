/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
#include "commodity-metering-server.h"

#include <protocols/interaction_model/StatusCode.h>

#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/EventLogging.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>
#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Globals;
using namespace chip::app::Clusters::Globals::Structs;
using namespace chip::app::Clusters::CommodityMetering;
using namespace chip::app::Clusters::CommodityMetering::Attributes;

using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace CommodityMetering {

namespace {

inline bool operator==(const Span<const uint32_t> & a, const Span<const uint32_t> & b)
{
    return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin());
}

inline bool operator==(const Structs::MeteredQuantityStruct::Type & lhs, const Structs::MeteredQuantityStruct::Type & rhs)
{
    return (lhs.tariffComponentIDs == rhs.tariffComponentIDs) && (lhs.quantity == rhs.quantity);
}

template <typename T>
bool NullableListsEqual(const DataModel::Nullable<DataModel::List<T>> & a, const DataModel::Nullable<DataModel::List<T>> & b)
{
    if (a.IsNull() || b.IsNull())
    {
        return a.IsNull() == b.IsNull();
    }
    else if (a.Value().size() == b.Value().size())
    {
        for (size_t i = 0; i < a.Value().size(); i++)
        {
            if (a.Value()[i] == b.Value()[i])
            {
                continue;
            }
            return false;
        }
        return true;
    }
    return false;
}

template <typename T>
struct SpanCopier
{
    static bool Copy(const Span<const T> & source, DataModel::List<const T> & destination,
                     Platform::ScopedMemoryBuffer<T> * bufferOut, size_t maxElements = std::numeric_limits<size_t>::max())
    {
        if (source.empty())
        {
            destination = DataModel::List<const T>();
            return true;
        }

        size_t elementsToCopy = std::min(source.size(), maxElements);

        if (!bufferOut->Calloc(elementsToCopy))
        {
            return false;
        }

        std::copy(source.begin(), source.begin() + elementsToCopy, bufferOut->Get());
        destination = DataModel::List<const T>(bufferOut->Get(), elementsToCopy);
        return true;
    }
};
} // namespace

Instance::~Instance()
{
    Shutdown();
}

CHIP_ERROR Instance::Init()
{
    mMeteredQuantity.SetNull();
    mMeteredQuantityTimestamp.SetNull();
    VerifyOrReturnError(AttributeAccessInterfaceRegistry::Instance().Register(this), CHIP_ERROR_INCORRECT_STATE);
    return CHIP_NO_ERROR;
}

void Instance::Shutdown()
{
    AttributeAccessInterfaceRegistry::Instance().Unregister(this);
}

CHIP_ERROR Instance::CopyMeteredQuantityEntry(Structs::MeteredQuantityStruct::Type & dest,
                                              Platform::ScopedMemoryBuffer<uint32_t> * destTariffComponentIDsBuffer,
                                              const Structs::MeteredQuantityStruct::Type & src)
{
    dest.quantity = src.quantity;

    if (!SpanCopier<uint32_t>::Copy(src.tariffComponentIDs, dest.tariffComponentIDs, destTariffComponentIDsBuffer,
                                    kMaxTariffComponentIDsPerMeteredQuantityEntry))
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::SetMeteredQuantity(const DataModel::Nullable<DataModel::List<Structs::MeteredQuantityStruct::Type>> & newValue)
{
    assertChipStackLockedByCurrentThread();

    if (NullableListEqual(newValue, mMeteredQuantity))
    {
        return CHIP_NO_ERROR;
    }

    if (mOwnedMeteredQuantityStructBuffer.Get() != nullptr)
    {
        mOwnedMeteredQuantityStructBuffer.Free();
    }

    for (size_t idx = 0; idx < kMaxMeteredQuantityEntries; idx++)
    {
        if (mOwnedMeteredQuantityTariffComponentIDsBuffer[idx]->Get() != nullptr)
        {
            mOwnedMeteredQuantityTariffComponentIDsBuffer[idx]->Free();
        }
    }

    const size_t len = newValue.IsNull() ? 0 : newValue.Value().size();

    if (len)
    {
        if (!mOwnedMeteredQuantityStructBuffer.Calloc(len))
        {
            return CHIP_ERROR_NO_MEMORY;
        }

        for (size_t idx = 0; idx < len; idx++)
        {
            // Deep copy each MeteredQuantityStruct in the newValue list
            ReturnLogErrorOnFailure(CopyMeteredQuantityEntry(
                mOwnedMeteredQuantityStructBuffer[idx], mOwnedMeteredQuantityTariffComponentIDsBuffer[idx], newValue.Value()[idx]));
        }

        mMeteredQuantity =
            MakeNullable(DataModel::List<Structs::MeteredQuantityStruct::Type>(mOwnedMeteredQuantityStructBuffer.Get(), len));
    }
    else
    {
        mMeteredQuantity.SetNull();
    }

    MatterReportingAttributeChangeCallback(mEndpointId, CommodityMetering::Id, MeteredQuantity::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::SetMeteredQuantityTimestamp(DataModel::Nullable<uint32_t> newValue)
{
    DataModel::Nullable<uint32_t> oldValue = mMeteredQuantityTimestamp;

    mMeteredQuantityTimestamp = newValue;
    if (oldValue != newValue)
    {
        if (newValue.IsNull())
        {
            ChipLogDetail(AppServer, "MeteredQuantityTimestamp updated to Null");
        }
        else
        {
            ChipLogDetail(AppServer, "MeteredQuantityTimestamp updated to %lu",
                          static_cast<unsigned long int>(mMeteredQuantityTimestamp.Value()));
        }

        mMeteredQuantityTimestamp = newValue;

        MatterReportingAttributeChangeCallback(mEndpointId, CommodityMetering::Id, MeteredQuantityTimestamp::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::SetTariffUnit(TariffUnitEnum newValue)
{
    TariffUnitEnum oldValue = mTariffUnit;

    if (EnsureKnownEnumValue(newValue) == TariffUnitEnum::kUnknownEnumValue)
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    mTariffUnit = newValue;
    if (oldValue != newValue)
    {
        ChipLogDetail(AppServer, "Endpoint: %d - mTariffUnit updated to %d", mEndpointId, to_underlying(mTariffUnit));
        MatterReportingAttributeChangeCallback(mEndpointId, CommodityMetering::Id, TariffUnit::Id);
    }

    return CHIP_NO_ERROR;
}

// AttributeAccessInterface
CHIP_ERROR Instance::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    ChipLogProgress(Zcl, "Commodity Metering read attr %" PRIu32 " on endpoint %" PRIu32, static_cast<uint32_t>(aPath.mAttributeId),
                    static_cast<uint32_t>(mEndpointId));

    switch (aPath.mAttributeId)
    {
    case MeteredQuantity::Id:
        ReturnErrorOnFailure(aEncoder.Encode(GetMeteredQuantity()));
        break;

    case MeteredQuantityTimestamp::Id:
        ReturnErrorOnFailure(aEncoder.Encode(GetMeteredQuantityTimestamp()));
        break;

    case TariffUnit::Id:
        ReturnErrorOnFailure(aEncoder.Encode(GetTariffUnit()));
        break;

    default:
        break;
    }
    return CHIP_NO_ERROR;
}

} // namespace CommodityMetering
} // namespace Clusters
} // namespace app
} // namespace chip

// -----------------------------------------------------------------------------
// Plugin initialization

void MatterCommodityMeteringPluginServerInitCallback() {}
