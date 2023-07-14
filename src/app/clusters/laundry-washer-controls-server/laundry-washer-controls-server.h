/**
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/util/af.h>

namespace chip {
namespace app {
namespace Clusters {
namespace LaundryWasherControls {

class LaundryWasherManager
{
public:
    template <typename T>
    struct AttributeProvider
    {
        using pointer = const T *;

        inline pointer begin() const { return mBegin; }
        inline pointer end() const { return mEnd; }

        AttributeProvider() : mBegin(nullptr), mEnd(nullptr) {}

        AttributeProvider(const pointer aBegin, const pointer aEnd) : mBegin(aBegin), mEnd(aEnd) {}

        pointer mBegin;
        pointer mEnd;
    };

    virtual AttributeProvider<const CharSpan> getSpinSpeedProvider(const EndpointId endpointId) const                = 0;
    virtual AttributeProvider<const NumberOfRinsesEnum> getSupportedRinseProvider(const EndpointId endpointId) const = 0;

    virtual ~LaundryWasherManager() {}
};

const LaundryWasherManager * getLaundryWasherManager();

/**
 * @brief LaundryWasher Server Plugin class
 */
class LaundryWasherServer
{
public:
    static LaundryWasherServer & Instance();

    /**
     * @brief Set/Get the attribute newSpinSpeedCurrent
     *
     * @param endpointId ID of the endpoint
     * @param newSpinSpeedCurrent attribute SpinSpeedCurrent
     * @return true on success, false on failure
     */
    EmberAfStatus SetSpinSpeedCurrent(chip::EndpointId endpointId, DataModel::Nullable<uint8_t> newSpinSpeedCurrent);
    EmberAfStatus GetSpinSpeedCurrent(chip::EndpointId endpointId, DataModel::Nullable<uint8_t> & spinSpeedCurrent);

    /**
     * @brief Set/Get the attribute NumberOfRinses
     *
     * @param endpointId ID of the endpoint
     * @param newNumberOfRinses attribute NumberOfRinses
     * @return true on success, false on failure
     */
    EmberAfStatus SetNumberOfRinses(chip::EndpointId endpointId, NumberOfRinsesEnum newNumberOfRinses);
    EmberAfStatus GetNumberOfRinses(chip::EndpointId endpointId, NumberOfRinsesEnum & numberOfRinses);

private:
    static LaundryWasherServer sInstance;
};

} // namespace LaundryWasherControls
} // namespace Clusters
} // namespace app
} // namespace chip
