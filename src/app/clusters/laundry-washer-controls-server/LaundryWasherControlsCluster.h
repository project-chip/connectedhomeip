/*
 *    Copyright (c) 2026 Project CHIP Authors
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
#pragma once

#include "laundry-washer-controls-delegate.h"
#include <app/data-model/Nullable.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/LaundryWasherControls/Attributes.h>

namespace chip::app::Clusters {

class LaundryWasherControlsCluster : public DefaultServerCluster
{
    // Values from spec.
    constexpr static uint8_t kMaxSpinSpeedLength = 64;
    constexpr static uint8_t kMaxSpinSpeedsLength = 16;
    constexpr static uint8_t kMaxSupportedRinsesLength = 4;

public:
    // Spec mandates that at least one of the features declared in LaundryWasherControls::Feature must be supported.
    // This encourages correct construction of the cluster when writing the code
    enum class SupportFeatures
    {
        kSpin  = BitFlags<LaundryWasherControls::Feature>(LaundryWasherControls::Feature::kSpin).Raw(),
        kRinse = BitFlags<LaundryWasherControls::Feature>(LaundryWasherControls::Feature::kRinse).Raw(),
        kSpinAndRinse = BitFlags<LaundryWasherControls::Feature>(LaundryWasherControls::Feature::kSpin, LaundryWasherControls::Feature::kRinse).Raw(),
    };

    struct Config
    {
        Config(SupportFeatures features, LaundryWasherControls::Delegate & delegate): mFeatures(static_cast<uint32_t>(features)), mDelegate(&delegate)
        {
            VerifyOrDie(mFeatures != BitFlags<LaundryWasherControls::Feature>()); // At least one feature must be set.
        }
        explicit Config(SupportFeatures features): Config(features, defaultDelegate) {}

        BitFlags<LaundryWasherControls::Feature> GetFeatures() const { return mFeatures; }
        LaundryWasherControls::Delegate * GetDelegate() const { return mDelegate; }

    private:
        BitFlags<LaundryWasherControls::Feature> mFeatures;
        LaundryWasherControls::Delegate * mDelegate;

        static inline LaundryWasherControls::DefaultDelegate defaultDelegate{};
    };

    LaundryWasherControlsCluster(EndpointId endpointId, const Config & config):
        DefaultServerCluster({ endpointId, LaundryWasherControls::Id }), mFeatures(config.GetFeatures()), mDelegate(config.GetDelegate()) {}

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;

    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    void SetDelegate(LaundryWasherControls::Delegate & delegate) { mDelegate = &delegate; }
    CHIP_ERROR SetSpinSpeedCurrent(const DataModel::Nullable<uint8_t> & spinSpeedCurrent);
    CHIP_ERROR SetNumberOfRinses(LaundryWasherControls::NumberOfRinsesEnum numberOfRinses);
    CHIP_ERROR GetSpinSpeedCurrent(DataModel::Nullable<uint8_t> & spinSpeedCurrent) const;
    CHIP_ERROR GetNumberOfRinses(LaundryWasherControls::NumberOfRinsesEnum & numberOfRinses) const;

private:
    CHIP_ERROR SpinSpeedIndexValidity(const DataModel::Nullable<uint8_t> & spinSpeedCurrent);
    CHIP_ERROR NumberOfRinsesValidity(LaundryWasherControls::NumberOfRinsesEnum numberOfRinses);

    CHIP_ERROR ReadSpinSpeeds(const ConcreteAttributePath & aPath, AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadSupportedRinses(const ConcreteAttributePath & aPath, AttributeValueEncoder & aEncoder);

    BitFlags<LaundryWasherControls::Feature> mFeatures;
    LaundryWasherControls::Delegate * mDelegate;

    DataModel::Nullable<uint8_t> mSpinSpeedCurrent{};
    LaundryWasherControls::NumberOfRinsesEnum mNumberOfRinses = LaundryWasherControls::NumberOfRinsesEnum::kNone;

};

} // namespace chip::app::Clusters
