#pragma once

#include <app-common/zap-generated/attributes/Accessors.h>

namespace chip {
namespace app {
namespace Clusters {

class ThermostatManager
{
public:
    ThermostatManager(EndpointId aEndpointId) : mEndpointId(aEndpointId){};

    void Init();

    void HeatingSetpointWriteCallback(int16_t newValue);
    void SystemModeWriteCallback(uint8_t newValue);

    void OnLocalTemperatureChangeCallback(int16_t temperature);

private:
    EndpointId mEndpointId;

    void SetHeating(bool isHeating);
};

} // namespace Clusters
} // namespace app
} // namespace chip