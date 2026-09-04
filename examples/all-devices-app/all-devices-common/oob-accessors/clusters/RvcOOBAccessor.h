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

#pragma once

#include <lib/core/DataModelTypes.h>
#include <lib/support/Span.h>
#include <oob-accessors/OOBAccessor.h>
#include <optional>
#include <string>

namespace chip::app {

// Abstract simulation delegate for RVC actions handled out-of-band.
class RvcSimulationDelegate
{
public:
    virtual ~RvcSimulationDelegate() = default;

    virtual void HandleCharged()                                                                                     = 0;
    virtual void HandleCharging()                                                                                    = 0;
    virtual void HandleDocked()                                                                                      = 0;
    virtual void HandleChargerFound()                                                                                = 0;
    virtual void HandleLowCharge()                                                                                   = 0;
    virtual void HandleActivityComplete()                                                                            = 0;
    virtual void HandleAreaComplete()                                                                                = 0;
    virtual void HandleClearError()                                                                                  = 0;
    virtual void HandleReset()                                                                                       = 0;
    virtual void HandleErrorEvent(const std::string & error)                                                         = 0;
    virtual void HandleEmptyingDustBin()                                                                             = 0;
    virtual void HandleCleaningMop()                                                                                 = 0;
    virtual void HandleFillingWaterTank()                                                                            = 0;
    virtual void HandleUpdatingMaps()                                                                                = 0;
    virtual bool HandleAddMap(uint32_t mapId, CharSpan mapName)                                                      = 0;
    virtual bool HandleRemoveMap(uint32_t mapId)                                                                     = 0;
    virtual bool HandleAddArea(uint32_t areaId, std::optional<uint32_t> mapId, std::optional<CharSpan> locationName) = 0;
    virtual bool HandleRemoveArea(uint32_t areaId)                                                                   = 0;
};

class RvcOOBAccessor : public OOBAccessor
{
public:
    RvcOOBAccessor(RvcSimulationDelegate & delegate, EndpointId endpointId) :
        mDelegate(delegate), mEndpointId(endpointId)
    {}

    std::optional<CHIP_ERROR> HandleAction(CharSpan action, ByteSpan tlvData) override;

private:
    std::optional<CHIP_ERROR> HandleParameterlessAction(CharSpan action, ByteSpan tlvData) const;
    std::optional<CHIP_ERROR> HandleErrorEvent(ByteSpan tlvData) const;
    std::optional<CHIP_ERROR> HandleAddMap(ByteSpan tlvData) const;
    std::optional<CHIP_ERROR> HandleRemoveMap(ByteSpan tlvData) const;
    std::optional<CHIP_ERROR> HandleAddArea(ByteSpan tlvData) const;
    std::optional<CHIP_ERROR> HandleRemoveArea(ByteSpan tlvData) const;

    RvcSimulationDelegate & mDelegate;
    EndpointId mEndpointId;
};

} // namespace chip::app
