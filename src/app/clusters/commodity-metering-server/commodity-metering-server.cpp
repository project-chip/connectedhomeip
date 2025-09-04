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

#include <cstdint>
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

// Some constraints for lists limitation
constexpr uint8_t kMaxTariffComponentIDsPerMeteredQuantityEntry = 128;
constexpr uint8_t kMaximumMeteredQuantitiesMinValue             = 1;
namespace {

inline bool operator==(const Span<const uint32_t> & a, const Span<const uint32_t> & b)
{
    return std::equal(a.begin(), a.end(), b.begin(), b.end());
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

    if (a.Value().size() != b.Value().size())
    {
        return false;
    }

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

template <typename T>
struct SpanCopier
{
    static CHIP_ERROR Copy(const Span<const T> & source, DataModel::List<const T> & destination,
                           size_t maxElements = std::numeric_limits<size_t>::max())
    {

        if (!destination.empty())
        {
            return CHIP_ERROR_IN_USE;
        }

        if (source.empty())
        {
            destination = DataModel::List<const T>();
            return CHIP_NO_ERROR;
        }

        size_t elementsToCopy = std::min(source.size(), maxElements);
        auto * buffer         = static_cast<T *>(Platform::MemoryCalloc(elementsToCopy, sizeof(T)));

        if (!buffer)
        {
            return CHIP_ERROR_NO_MEMORY;
        }

        std::copy(source.begin(), source.begin() + elementsToCopy, buffer);
        destination = DataModel::List<const T>(buffer, elementsToCopy);
        return CHIP_NO_ERROR;
    }
};
} // namespace

static void CleanUpIDs(DataModel::List<const uint32_t> & IDs)
{
    if (!IDs.empty() && IDs.data())
    {
        Platform::MemoryFree(const_cast<uint32_t *>(IDs.data()));
        IDs = DataModel::List<const uint32_t>();
    }
}

static void CleanupMeteredQuantityData(DataModel::List<Structs::MeteredQuantityStruct::Type> & aValue)
{
    if (aValue.data() != nullptr)
    {
        for (auto & item : aValue)
        {
            CleanUpIDs(item.tariffComponentIDs);
        }

        Platform::MemoryFree(aValue.data());
        aValue = DataModel::List<Structs::MeteredQuantityStruct::Type>();
    }
}

Instance::~Instance()
{
    if (!mMeteredQuantity.IsNull())
    {
        CleanupMeteredQuantityData(mMeteredQuantity.Value());
    }
    Shutdown();
}

CHIP_ERROR Instance::Init()
{
    mMeteredQuantity.SetNull();
    mMeteredQuantityTimestamp.SetNull();
    mTariffUnit.SetNull();
    mMaximumMeteredQuantities.SetNull();
    VerifyOrReturnError(AttributeAccessInterfaceRegistry::Instance().Register(this), CHIP_ERROR_INCORRECT_STATE);
    return CHIP_NO_ERROR;
}

void Instance::Shutdown()
{
    AttributeAccessInterfaceRegistry::Instance().Unregister(this);
}

static CHIP_ERROR CopyMeteredQuantityEntry(const Structs::MeteredQuantityStruct::Type & src,
                                           Structs::MeteredQuantityStruct::Type & dest)
{
    dest.quantity = src.quantity;

    return SpanCopier<uint32_t>::Copy(src.tariffComponentIDs, dest.tariffComponentIDs,
                                      kMaxTariffComponentIDsPerMeteredQuantityEntry);
}

CHIP_ERROR Instance::SetMeteredQuantity(const DataModel::Nullable<DataModel::List<Structs::MeteredQuantityStruct::Type>> & newValue)
{
    assertChipStackLockedByCurrentThread();

    if (NullableListsEqual(newValue, mMeteredQuantity))
    {
        return CHIP_NO_ERROR;
    }

    if (!mMeteredQuantity.IsNull())
    {
        CleanupMeteredQuantityData(mMeteredQuantity.Value());
    }

    if (newValue.IsNull())
    {
        mMeteredQuantity.SetNull();
    }
    else if (mMaximumMeteredQuantities.IsNull())
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    else
    {
        uint16_t len = static_cast<uint16_t>(newValue.Value().size());

        if (len > mMaximumMeteredQuantities.Value())
        {
            return CHIP_ERROR_INVALID_LIST_LENGTH;
        }

        if (len == 0)
        {
            mMeteredQuantity = MakeNullable(DataModel::List<Structs::MeteredQuantityStruct::Type>());
        }
        else
        {
            Platform::ScopedMemoryBuffer<Structs::MeteredQuantityStruct::Type> buffer;

            if (!buffer.Calloc(len))
            {
                return CHIP_ERROR_NO_MEMORY;
            }

            for (size_t idx = 0; idx < len; idx++)
            {
                CHIP_ERROR err = CopyMeteredQuantityEntry(newValue.Value()[idx], buffer[idx]);
                if (err != CHIP_NO_ERROR)
                {
                    // Clean up any partially copied IDs
                    for (size_t cleanupIdx = 0; cleanupIdx < idx; cleanupIdx++)
                    {
                        CleanUpIDs(buffer[cleanupIdx].tariffComponentIDs);
                    }
                    buffer.Free();
                    return err;
                }
            }

            mMeteredQuantity = MakeNullable(DataModel::List(buffer.Release(), len));
        }
    }

    MatterReportingAttributeChangeCallback(mEndpointId, CommodityMetering::Id, MeteredQuantity::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::SetMeteredQuantityTimestamp(DataModel::Nullable<uint32_t> newValue)
{
    DataModel::Nullable<uint32_t> oldValue = mMeteredQuantityTimestamp;

    if (oldValue != newValue)
    {
        if (newValue.IsNull())
        {
            ChipLogDetail(AppServer, "MeteredQuantityTimestamp updated to Null");
        }
        else
        {
            ChipLogDetail(AppServer, "MeteredQuantityTimestamp updated to %lu", static_cast<unsigned long int>(newValue.Value()));
        }

        mMeteredQuantityTimestamp = newValue;

        MatterReportingAttributeChangeCallback(mEndpointId, CommodityMetering::Id, MeteredQuantityTimestamp::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::SetTariffUnit(DataModel::Nullable<Globals::TariffUnitEnum> newValue)
{
    DataModel::Nullable<Globals::TariffUnitEnum> oldValue = mTariffUnit;

    if (oldValue != newValue)
    {
        if (newValue.IsNull())
        {
            mTariffUnit.SetNull();
        }
        else if (EnsureKnownEnumValue(newValue.Value()) != Globals::TariffUnitEnum::kUnknownEnumValue)
        {
            mTariffUnit = newValue;
            ChipLogDetail(AppServer, "Endpoint: %d - mTariffUnit updated to %d", mEndpointId, to_underlying(mTariffUnit.Value()));
        }
        else
        {
            return CHIP_IM_GLOBAL_STATUS(ConstraintError);
        }

        MatterReportingAttributeChangeCallback(mEndpointId, CommodityMetering::Id, TariffUnit::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::SetMaximumMeteredQuantities(DataModel::Nullable<uint16_t> newValue)
{
    DataModel::Nullable<uint16_t> oldValue = mMaximumMeteredQuantities;

    if (oldValue != newValue)
    {
        if (newValue.IsNull())
        {
            ChipLogDetail(AppServer, "MaximumMeteredQuantities updated to Null");
        }
        else if (newValue.Value() >= kMaximumMeteredQuantitiesMinValue)
        {
            ChipLogDetail(AppServer, "MaximumMeteredQuantities updated to %lu", static_cast<unsigned long int>(newValue.Value()));
        }
        else
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        mMaximumMeteredQuantities = newValue;

        MatterReportingAttributeChangeCallback(mEndpointId, CommodityMetering::Id, MaximumMeteredQuantities::Id);
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
    case MeteredQuantity::Id: {
        if (GetMeteredQuantity().IsNull())
        {
            return aEncoder.EncodeNull();
        }
        else
        {
            auto & list = GetMeteredQuantity().Value();
            ReturnErrorOnFailure(aEncoder.EncodeList([&list](const auto & encoder) {
                for (const auto & item : list)
                {
                    CHIP_ERROR err = encoder.Encode(item);
                    if (err != CHIP_NO_ERROR)
                    {
                        return err;
                    }
                }
                return CHIP_NO_ERROR;
            }));
        }
        break;
    }

    case MeteredQuantityTimestamp::Id:
        ReturnErrorOnFailure(aEncoder.Encode(GetMeteredQuantityTimestamp()));
        break;

    case TariffUnit::Id:
        ReturnErrorOnFailure(aEncoder.Encode(GetTariffUnit()));
        break;

    case MaximumMeteredQuantities::Id:
        ReturnErrorOnFailure(aEncoder.Encode(GetMaximumMeteredQuantities()));
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
