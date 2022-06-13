/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "wifi_service/wifi_service.rpc.pb.h"

namespace chip {
namespace rpc {

class WiFi : public pw_rpc::nanopb::WiFi::Service<WiFi>
{
public:
    virtual ~WiFi() = default;

    virtual pw::Status GetChannel(const pw_protobuf_Empty & request, chip_rpc_Channel & response)
    {
        return pw::Status::Unimplemented();
    };

    virtual pw::Status GetSsid(const pw_protobuf_Empty & request, chip_rpc_Ssid & response) { return pw::Status::Unimplemented(); };
    virtual pw::Status GetState(const pw_protobuf_Empty & request, chip_rpc_State & response)
    {
        return pw::Status::Unimplemented();
    }
    virtual pw::Status GetMacAddress(const pw_protobuf_Empty & request, chip_rpc_MacAddress & response)
    {
        return pw::Status::Unimplemented();
    }
    virtual pw::Status GetWiFiInterface(const pw_protobuf_Empty & request, chip_rpc_WiFiInterface & response)
    {
        return pw::Status::Unimplemented();
    }
    virtual pw::Status GetIP4Address(const pw_protobuf_Empty & request, chip_rpc_IP4Address & response)
    {
        return pw::Status::Unimplemented();
    }
    virtual pw::Status GetIP6Address(const pw_protobuf_Empty & request, chip_rpc_IP6Address & response)
    {
        return pw::Status::Unimplemented();
    }
    virtual void StartScan(const chip_rpc_ScanConfig & request, ServerWriter<chip_rpc_ScanResults> & writer) {}
    virtual pw::Status StopScan(const pw_protobuf_Empty & request, pw_protobuf_Empty & response)
    {
        return pw::Status::Unimplemented();
    }
    virtual pw::Status Connect(const chip_rpc_ConnectionData & request, chip_rpc_ConnectionResult & response)
    {
        return pw::Status::Unimplemented();
    }
    virtual pw::Status Disconnect(const pw_protobuf_Empty & request, pw_protobuf_Empty & response)
    {
        return pw::Status::Unimplemented();
    }
};

} // namespace rpc
} // namespace chip
