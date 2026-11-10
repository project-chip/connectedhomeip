/*
 *
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

#include <device/types/electrical-energy-tariff/ElectricalEnergyTariff.h>

#include <devices/Types.h>

namespace chip::app {

ElectricalEnergyTariff::ElectricalEnergyTariff() :
    SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kElectricalEnergyTariff, 1))
{}

CHIP_ERROR ElectricalEnergyTariff::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                            EndpointComposition composition)
{
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    DeviceRegistrationTransaction transaction(*this, provider);

    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));
    ReturnErrorOnFailure(RegisterOptionalClusters(endpoint, provider));
    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));

    transaction.Commit();
    return CHIP_NO_ERROR;
}

void ElectricalEnergyTariff::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);
    UnregisterOptionalClusters(provider);
}

} // namespace chip::app
