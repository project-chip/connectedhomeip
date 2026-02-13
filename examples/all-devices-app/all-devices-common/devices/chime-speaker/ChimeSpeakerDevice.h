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

#pragma once

#include <devices/chime/impl/LoggingChimeDevice.h>
#include <devices/interface/DeviceInterface.h>
#include <devices/speaker/impl/LoggingSpeakerDevice.h>

namespace chip {
namespace app {

class ChimeSpeakerDevice : public DeviceInterface
{
public:
    ChimeSpeakerDevice(TimerDelegate & timerDelegate) :
        DeviceInterface(Span<const DataModel::DeviceTypeEntry>{}), mSpeakerDevice({ .timerDelegate = timerDelegate })
    {}

    CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointId parentId = kInvalidEndpointId) override
    {
        ReturnErrorOnFailure(mChimeDevice.Register(endpoint, provider, parentId));
        ReturnErrorOnFailure(mSpeakerDevice.Register(endpoint + 1, provider, parentId));
        return CHIP_NO_ERROR;
    }

    void UnRegister(CodeDrivenDataModelProvider & provider) override
    {
        mChimeDevice.UnRegister(provider);
        mSpeakerDevice.UnRegister(provider);
    }

    // Since this device registers its children as endpoints, it doesn't need to implement these.
    // They won't be called because this instance is never registered as an endpoint itself.
    CHIP_ERROR DeviceTypes(ReadOnlyBufferBuilder<DataModel::DeviceTypeEntry> & out) const override { return CHIP_NO_ERROR; }
    CHIP_ERROR ClientClusters(ReadOnlyBufferBuilder<ClusterId> & out) const override { return CHIP_NO_ERROR; }

private:
    LoggingChimeDevice mChimeDevice;
    LoggingSpeakerDevice mSpeakerDevice;
};

} // namespace app
} // namespace chip
