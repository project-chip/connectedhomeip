/*
 *
 *    Copyright (c) 2018 Nest Labs, Inc.
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

/**
 *    @file
 *          Defines the Device Layer DeviceDescriptionServer object.
 */

#ifndef DEVICE_DESCRIPTION_SERVER_H
#define DEVICE_DESCRIPTION_SERVER_H

#include <Weave/DeviceLayer/internal/WeaveDeviceLayerInternal.h>
#include <Weave/Profiles/device-description/DeviceDescription.h>

namespace nl {
namespace Weave {
namespace DeviceLayer {
namespace Internal {

/**
 * Implements the Weave Device Description profile for a Weave device.
 */
class DeviceDescriptionServer final
    : public ::nl::Weave::Profiles::DeviceDescription::DeviceDescriptionServer
{
    typedef ::nl::Weave::Profiles::DeviceDescription::DeviceDescriptionServer ServerBaseClass;

public:

    // ===== Members for internal use by other Device Layer components.

    WEAVE_ERROR Init();

    bool IsUserSelectedModeActive(void);
    void SetUserSelectedMode(bool val);
    uint16_t GetUserSelectedModeTimeout(void);
    void SetUserSelectedModeTimeout(uint16_t val);

    void OnPlatformEvent(const WeaveDeviceEvent * event);

private:

    // ===== Members for internal use by the following friends.

    friend DeviceDescriptionServer & DeviceDescriptionSvr(void);

    static DeviceDescriptionServer sInstance;

    // ===== Private members reserved for use by this class only.

    enum
    {
        kUserSelectedModeTimeShift = 10
    };

    uint32_t mUserSelectedModeEndTime; // Monotonic system time scaled to units of 1024ms.
    uint16_t mUserSelectedModeTimeoutSec;

    static void HandleIdentifyRequest(void *appState, uint64_t nodeId, const IPAddress& nodeAddr,
            const ::nl::Weave::Profiles::DeviceDescription::IdentifyRequestMessage& reqMsg, bool& sendResp,
            ::nl::Weave::Profiles::DeviceDescription::IdentifyResponseMessage& respMsg);

protected:

    // Construction/destruction limited to subclasses.
    DeviceDescriptionServer() = default;
    ~DeviceDescriptionServer() = default;

    // No copy, move or assignment.
    DeviceDescriptionServer(const DeviceDescriptionServer &) = delete;
    DeviceDescriptionServer(const DeviceDescriptionServer &&) = delete;
    DeviceDescriptionServer & operator=(const DeviceDescriptionServer &) = delete;
};

/**
 * Returns a reference to the DeviceDescriptionServer singleton object.
 */
inline DeviceDescriptionServer & DeviceDescriptionSvr(void)
{
    return DeviceDescriptionServer::sInstance;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace Weave
} // namespace nl


#endif // DEVICE_DESCRIPTION_SERVER_H
