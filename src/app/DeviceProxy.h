/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
 *  @file
 *    This file contains definitions for DeviceProxy base class. The objects of this
 *    class will be used by applications to interact with peer CHIP devices.
 *    The class provides mechanism to construct, send and receive messages to and
 *    from the corresponding CHIP devices.
 */

#pragma once

#include <app/CommandSender.h>
#include <app/DeviceControllerInteractionModelDelegate.h>
#include <app/InteractionModelEngine.h>
#include <app/util/CHIPDeviceCallbacksMgr.h>
#include <lib/core/CHIPCallback.h>
#include <lib/core/CHIPCore.h>
#include <lib/support/DLLUtil.h>

namespace chip {

class DLL_EXPORT DeviceProxy
{
public:
    virtual ~DeviceProxy() {}
    DeviceProxy() {}

    /**
     *  Mark any open session with the device as expired.
     */
    virtual CHIP_ERROR Disconnect() = 0;

    virtual NodeId GetDeviceId() const = 0;

    virtual bool GetAddress(Inet::IPAddress & addr, uint16_t & port) const { return false; }

    virtual CHIP_ERROR SendSubscribeAttributeRequest(app::AttributePathParams aPath, uint16_t mMinIntervalFloorSeconds,
                                                     uint16_t mMaxIntervalCeilingSeconds, Callback::Cancelable * onSuccessCallback,
                                                     Callback::Cancelable * onFailureCallback);
    virtual CHIP_ERROR ShutdownSubscriptions() { return CHIP_ERROR_NOT_IMPLEMENTED; }

    virtual CHIP_ERROR SendWriteAttributeRequest(app::WriteClientHandle aHandle, Callback::Cancelable * onSuccessCallback,
                                                 Callback::Cancelable * onFailureCallback);

    virtual CHIP_ERROR SendCommands(app::CommandSender * commandObj);

    virtual void AddReportHandler(EndpointId endpoint, ClusterId cluster, AttributeId attribute,
                                  Callback::Cancelable * onReportCallback, app::TLVDataFilter tlvDataFilter);

    // Interaction model uses the object and callback interface instead of sequence number to mark different transactions.
    virtual void AddIMResponseHandler(void * commandObj, Callback::Cancelable * onSuccessCallback,
                                      Callback::Cancelable * onFailureCallback, app::TLVDataFilter tlvDataFilter = nullptr);

    virtual void CancelIMResponseHandler(void * commandObj);

    virtual Controller::DeviceControllerInteractionModelDelegate * GetInteractionModelDelegate() = 0;

    virtual Messaging::ExchangeManager * GetExchangeManager() const = 0;

    virtual chip::Optional<SessionHandle> GetSecureSession() const = 0;

    virtual bool IsActive() const { return true; }

    const ReliableMessageProtocolConfig & GetMRPConfig() const { return mMRPConfig; }

protected:
    virtual bool IsSecureConnected() const = 0;

    virtual uint8_t GetNextSequenceNumber() = 0;

    app::CHIPDeviceCallbacksMgr & mCallbacksMgr = app::CHIPDeviceCallbacksMgr::GetInstance();

    ReliableMessageProtocolConfig mMRPConfig = gDefaultMRPConfig;
};

} // namespace chip
