/*
 * Copyright (c) 2026 Project CHIP Authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include <app/AttributeAccessInterface.h>
#include <clusters/OperationalCredentials/ClusterId.h>
#include <lib/support/Span.h>

// Linux/macOS all-clusters test hook. Access only from the Matter thread.
class PQCDeviceAttestationProfileReadOverride : public chip::app::AttributeAccessInterface
{
public:
    PQCDeviceAttestationProfileReadOverride() :
        AttributeAccessInterface(chip::MakeOptional(chip::kRootEndpointId), chip::app::Clusters::OperationalCredentials::Id)
    {}

    // Accepts Normal, UnsupportedAttribute, or Failure. Invalid input preserves the current mode.
    CHIP_ERROR SetReadMode(chip::CharSpan mode);
    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & path, chip::app::AttributeValueEncoder & encoder) override;

private:
    CHIP_ERROR mReadError = CHIP_NO_ERROR;
};

PQCDeviceAttestationProfileReadOverride & GetPQCDeviceAttestationProfileReadOverride();
