/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#ifndef CHIP_APP_SERVER_RENDEZVOUS_DELEGATE_H_
#define CHIP_APP_SERVER_RENDEZVOUS_DELEGATE_H_

#include <platform/CHIPDeviceLayer.h>
#include <transport/RendezvousSession.h>

namespace chip {

class RendezvousServer : public RendezvousSessionDelegate
{
public:
    RendezvousServer();

    CHIP_ERROR Init(const RendezvousParameters & params);

    void OnRendezvousConnectionOpened() override;
    void OnRendezvousConnectionClosed() override;
    void OnRendezvousError(CHIP_ERROR err) override;
    void OnRendezvousMessageReceived(PacketBuffer * buffer) override;

private:
    RendezvousSession mRendezvousSession;
};

} // namespace chip

#endif // CHIP_APP_SERVER_RENDEZVOUS_DELEGATE_H_
