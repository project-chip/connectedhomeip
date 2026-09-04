/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <oob-accessors/clusters/RvcOOBAccessor.h>

#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>

namespace chip::app {

std::optional<CHIP_ERROR> RvcOOBAccessor::HandleAction(CharSpan action, ByteSpan tlvData)
{
    if (action.data_equal("Reset"_span) || action.data_equal("Charged"_span) || action.data_equal("Charging"_span) ||
        action.data_equal("Docked"_span) || action.data_equal("ChargerFound"_span) || action.data_equal("LowCharge"_span) ||
        action.data_equal("ActivityComplete"_span) || action.data_equal("AreaComplete"_span) ||
        action.data_equal("ClearError"_span) || action.data_equal("EmptyingDustBin"_span) ||
        action.data_equal("CleaningMop"_span) || action.data_equal("FillingWaterTank"_span) ||
        action.data_equal("UpdatingMaps"_span))
    {
        return HandleParameterlessAction(action, tlvData);
    }
    if (action.data_equal("ErrorEvent"_span))
    {
        return HandleErrorEvent(tlvData);
    }
    if (action.data_equal("AddMap"_span))
    {
        return HandleAddMap(tlvData);
    }
    if (action.data_equal("RemoveMap"_span))
    {
        return HandleRemoveMap(tlvData);
    }
    if (action.data_equal("AddArea"_span))
    {
        return HandleAddArea(tlvData);
    }
    if (action.data_equal("RemoveArea"_span))
    {
        return HandleRemoveArea(tlvData);
    }
    return std::nullopt;
}

std::optional<CHIP_ERROR> RvcOOBAccessor::HandleParameterlessAction(CharSpan action, ByteSpan tlvData) const
{
    TLV::TLVReader reader;
    reader.Init(tlvData);
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

    TLV::TLVType outerType;
    ReturnErrorOnFailure(reader.EnterContainer(outerType));

    EndpointId endpointId = kInvalidEndpointId;
    bool hasEndpointId    = false;

    while (reader.Next() == CHIP_NO_ERROR)
    {
        TLV::Tag tag = reader.GetTag();
        if (TLV::IsContextTag(tag) && TLV::TagNumFromTag(tag) == 1)
        {
            ReturnErrorOnFailure(reader.Get(endpointId));
            hasEndpointId = true;
        }
    }
    ReturnErrorOnFailure(reader.ExitContainer(outerType));

    VerifyOrReturnError(hasEndpointId, CHIP_ERROR_INVALID_ARGUMENT);
    if (endpointId != mEndpointId)
    {
        return std::nullopt;
    }

    if (action.data_equal("Reset"_span))
    {
        mDelegate.HandleReset();
    }
    else if (action.data_equal("Charged"_span))
    {
        mDelegate.HandleCharged();
    }
    else if (action.data_equal("Charging"_span))
    {
        mDelegate.HandleCharging();
    }
    else if (action.data_equal("Docked"_span))
    {
        mDelegate.HandleDocked();
    }
    else if (action.data_equal("ChargerFound"_span))
    {
        mDelegate.HandleChargerFound();
    }
    else if (action.data_equal("LowCharge"_span))
    {
        mDelegate.HandleLowCharge();
    }
    else if (action.data_equal("ActivityComplete"_span))
    {
        mDelegate.HandleActivityComplete();
    }
    else if (action.data_equal("AreaComplete"_span))
    {
        mDelegate.HandleAreaComplete();
    }
    else if (action.data_equal("ClearError"_span))
    {
        mDelegate.HandleClearError();
    }
    else if (action.data_equal("EmptyingDustBin"_span))
    {
        mDelegate.HandleEmptyingDustBin();
    }
    else if (action.data_equal("CleaningMop"_span))
    {
        mDelegate.HandleCleaningMop();
    }
    else if (action.data_equal("FillingWaterTank"_span))
    {
        mDelegate.HandleFillingWaterTank();
    }
    else if (action.data_equal("UpdatingMaps"_span))
    {
        mDelegate.HandleUpdatingMaps();
    }

    return CHIP_NO_ERROR;
}

std::optional<CHIP_ERROR> RvcOOBAccessor::HandleErrorEvent(ByteSpan tlvData) const
{
    TLV::TLVReader reader;
    reader.Init(tlvData);
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

    TLV::TLVType outerType;
    ReturnErrorOnFailure(reader.EnterContainer(outerType));

    EndpointId endpointId = kInvalidEndpointId;
    CharSpan error;
    bool hasEndpointId = false;
    bool hasError      = false;

    while (reader.Next() == CHIP_NO_ERROR)
    {
        TLV::Tag tag = reader.GetTag();
        if (!TLV::IsContextTag(tag))
        {
            continue;
        }
        switch (TLV::TagNumFromTag(tag))
        {
        case 1:
            ReturnErrorOnFailure(reader.Get(endpointId));
            hasEndpointId = true;
            break;
        case 2:
            ReturnErrorOnFailure(reader.Get(error));
            hasError = true;
            break;
        default:
            break;
        }
    }
    ReturnErrorOnFailure(reader.ExitContainer(outerType));

    VerifyOrReturnError(hasEndpointId && hasError, CHIP_ERROR_INVALID_ARGUMENT);
    if (endpointId != mEndpointId)
    {
        return std::nullopt;
    }

    mDelegate.HandleErrorEvent(std::string(error.data(), error.size()));
    return CHIP_NO_ERROR;
}

std::optional<CHIP_ERROR> RvcOOBAccessor::HandleAddMap(ByteSpan tlvData) const
{
    TLV::TLVReader reader;
    reader.Init(tlvData);
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

    TLV::TLVType outerType;
    ReturnErrorOnFailure(reader.EnterContainer(outerType));

    EndpointId endpointId = kInvalidEndpointId;
    uint32_t mapId        = 0;
    CharSpan mapName;
    bool hasEndpointId = false;
    bool hasMapId      = false;
    bool hasMapName    = false;

    while (reader.Next() == CHIP_NO_ERROR)
    {
        TLV::Tag tag = reader.GetTag();
        if (!TLV::IsContextTag(tag))
        {
            continue;
        }
        switch (TLV::TagNumFromTag(tag))
        {
        case 1:
            ReturnErrorOnFailure(reader.Get(endpointId));
            hasEndpointId = true;
            break;
        case 2:
            ReturnErrorOnFailure(reader.Get(mapId));
            hasMapId = true;
            break;
        case 3:
            ReturnErrorOnFailure(reader.Get(mapName));
            hasMapName = true;
            break;
        default:
            break;
        }
    }
    ReturnErrorOnFailure(reader.ExitContainer(outerType));

    VerifyOrReturnError(hasEndpointId && hasMapId && hasMapName, CHIP_ERROR_INVALID_ARGUMENT);
    if (endpointId != mEndpointId)
    {
        return std::nullopt;
    }

    VerifyOrReturnError(mDelegate.HandleAddMap(mapId, mapName), CHIP_ERROR_INTERNAL);
    return CHIP_NO_ERROR;
}

std::optional<CHIP_ERROR> RvcOOBAccessor::HandleRemoveMap(ByteSpan tlvData) const
{
    TLV::TLVReader reader;
    reader.Init(tlvData);
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

    TLV::TLVType outerType;
    ReturnErrorOnFailure(reader.EnterContainer(outerType));

    EndpointId endpointId = kInvalidEndpointId;
    uint32_t mapId        = 0;
    bool hasEndpointId    = false;
    bool hasMapId         = false;

    while (reader.Next() == CHIP_NO_ERROR)
    {
        TLV::Tag tag = reader.GetTag();
        if (!TLV::IsContextTag(tag))
        {
            continue;
        }
        switch (TLV::TagNumFromTag(tag))
        {
        case 1:
            ReturnErrorOnFailure(reader.Get(endpointId));
            hasEndpointId = true;
            break;
        case 2:
            ReturnErrorOnFailure(reader.Get(mapId));
            hasMapId = true;
            break;
        default:
            break;
        }
    }
    ReturnErrorOnFailure(reader.ExitContainer(outerType));

    VerifyOrReturnError(hasEndpointId && hasMapId, CHIP_ERROR_INVALID_ARGUMENT);
    if (endpointId != mEndpointId)
    {
        return std::nullopt;
    }

    VerifyOrReturnError(mDelegate.HandleRemoveMap(mapId), CHIP_ERROR_INTERNAL);
    return CHIP_NO_ERROR;
}

std::optional<CHIP_ERROR> RvcOOBAccessor::HandleAddArea(ByteSpan tlvData) const
{
    TLV::TLVReader reader;
    reader.Init(tlvData);
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

    TLV::TLVType outerType;
    ReturnErrorOnFailure(reader.EnterContainer(outerType));

    EndpointId endpointId = kInvalidEndpointId;
    uint32_t areaId       = 0;
    std::optional<uint32_t> mapId;
    std::optional<CharSpan> locationName;
    bool hasEndpointId = false;
    bool hasAreaId     = false;

    while (reader.Next() == CHIP_NO_ERROR)
    {
        TLV::Tag tag = reader.GetTag();
        if (!TLV::IsContextTag(tag))
        {
            continue;
        }
        switch (TLV::TagNumFromTag(tag))
        {
        case 1:
            ReturnErrorOnFailure(reader.Get(endpointId));
            hasEndpointId = true;
            break;
        case 2:
            ReturnErrorOnFailure(reader.Get(areaId));
            hasAreaId = true;
            break;
        case 3: {
            uint32_t val = 0;
            ReturnErrorOnFailure(reader.Get(val));
            mapId = val;
            break;
        }
        case 4: {
            CharSpan val;
            ReturnErrorOnFailure(reader.Get(val));
            locationName = val;
            break;
        }
        default:
            break;
        }
    }
    ReturnErrorOnFailure(reader.ExitContainer(outerType));

    VerifyOrReturnError(hasEndpointId && hasAreaId, CHIP_ERROR_INVALID_ARGUMENT);
    if (endpointId != mEndpointId)
    {
        return std::nullopt;
    }

    VerifyOrReturnError(mDelegate.HandleAddArea(areaId, mapId, locationName), CHIP_ERROR_INTERNAL);
    return CHIP_NO_ERROR;
}

std::optional<CHIP_ERROR> RvcOOBAccessor::HandleRemoveArea(ByteSpan tlvData) const
{
    TLV::TLVReader reader;
    reader.Init(tlvData);
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

    TLV::TLVType outerType;
    ReturnErrorOnFailure(reader.EnterContainer(outerType));

    EndpointId endpointId = kInvalidEndpointId;
    uint32_t areaId       = 0;
    bool hasEndpointId    = false;
    bool hasAreaId        = false;

    while (reader.Next() == CHIP_NO_ERROR)
    {
        TLV::Tag tag = reader.GetTag();
        if (!TLV::IsContextTag(tag))
        {
            continue;
        }
        switch (TLV::TagNumFromTag(tag))
        {
        case 1:
            ReturnErrorOnFailure(reader.Get(endpointId));
            hasEndpointId = true;
            break;
        case 2:
            ReturnErrorOnFailure(reader.Get(areaId));
            hasAreaId = true;
            break;
        default:
            break;
        }
    }
    ReturnErrorOnFailure(reader.ExitContainer(outerType));

    VerifyOrReturnError(hasEndpointId && hasAreaId, CHIP_ERROR_INVALID_ARGUMENT);
    if (endpointId != mEndpointId)
    {
        return std::nullopt;
    }

    VerifyOrReturnError(mDelegate.HandleRemoveArea(areaId), CHIP_ERROR_INTERNAL);
    return CHIP_NO_ERROR;
}

} // namespace chip::app
