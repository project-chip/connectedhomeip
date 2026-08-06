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
    constexpr static uint8_t kMaxSpinSpeedLength       = 64;
    constexpr static uint8_t kMaxSpinSpeedsLength      = 16;
    constexpr static uint8_t kMaxSupportedRinsesLength = 4;

public:
    // Spec mandates that at least one of the features declared in LaundryWasherControls::Feature must be supported.
    // This encourages correct construction of the cluster when writing the code

    struct Config
    {
        Config(BitFlags<LaundryWasherControls::Feature> features, LaundryWasherControls::Delegate & delegate) :
            mFeatures(features), mDelegate(&delegate)
        {
            VerifyOrDie(mFeatures.HasAny()); // At least one feature must be set.
        }

        // This config will make the class fail when setting spin speed or number of rinses if a delegate is not set after
        // construction.
        explicit Config(BitFlags<LaundryWasherControls::Feature> features) : mFeatures(features), mDelegate(nullptr)
        {
            VerifyOrDie(mFeatures.HasAny()); // At least one feature must be set.
        }

        BitFlags<LaundryWasherControls::Feature> GetFeatures() const { return mFeatures; }
        LaundryWasherControls::Delegate * GetDelegate() const { return mDelegate; }

    private:
        BitFlags<LaundryWasherControls::Feature> mFeatures;
        LaundryWasherControls::Delegate * mDelegate;
    };

    LaundryWasherControlsCluster(EndpointId endpointId, const Config & config) :
        DefaultServerCluster({ endpointId, LaundryWasherControls::Id }), mFeatures(config.GetFeatures()),
        mDelegate(config.GetDelegate())
    {}

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;

    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    // Delegate should be valid until the cluster is destroyed.
    void SetDelegate(LaundryWasherControls::Delegate & delegate)
    {
        mDelegate = &delegate;

        // delegate change implies change in these attributes.
        NotifySpinSpeedsAttributeChanged();
        NotifySupportedRinsesAttributeChanged();
    }
    CHIP_ERROR SetSpinSpeedCurrent(DataModel::Nullable<uint8_t> spinSpeedCurrent);
    CHIP_ERROR SetNumberOfRinses(LaundryWasherControls::NumberOfRinsesEnum numberOfRinses);
    DataModel::Nullable<uint8_t> GetSpinSpeedCurrent() const { return mSpinSpeedCurrent; }
    LaundryWasherControls::NumberOfRinsesEnum GetNumberOfRinses() const { return mNumberOfRinses; }

    // Notify the data model that the SpinSpeeds attribute has changed.
    // This is expected to be called when the delegate will return a different value for any index of the SpinSpeeds attribute.
    void NotifySpinSpeedsAttributeChanged() { NotifyAttributeChanged(LaundryWasherControls::Attributes::SpinSpeeds::Id); }

    // Notify the data model that the SupportedRinses attribute has changed.
    // This is expected to be called when the delegate will return a different value for any index of the SupportedRinses attribute.
    void NotifySupportedRinsesAttributeChanged() { NotifyAttributeChanged(LaundryWasherControls::Attributes::SupportedRinses::Id); }

private:
    CHIP_ERROR SpinSpeedIndexValidity(DataModel::Nullable<uint8_t> spinSpeedCurrent);
    CHIP_ERROR NumberOfRinsesValidity(LaundryWasherControls::NumberOfRinsesEnum numberOfRinses);

    CHIP_ERROR ReadSpinSpeeds(const ConcreteAttributePath & aPath, AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadSupportedRinses(const ConcreteAttributePath & aPath, AttributeValueEncoder & aEncoder);

    BitFlags<LaundryWasherControls::Feature> mFeatures;
    LaundryWasherControls::Delegate * mDelegate;

    DataModel::Nullable<uint8_t> mSpinSpeedCurrent{};
    LaundryWasherControls::NumberOfRinsesEnum mNumberOfRinses = LaundryWasherControls::NumberOfRinsesEnum::kNone;
};

} // namespace chip::app::Clusters
