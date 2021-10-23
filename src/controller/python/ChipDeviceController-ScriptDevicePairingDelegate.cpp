/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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

#include "ChipDeviceController-ScriptDevicePairingDelegate.h"

namespace chip {
namespace Controller {

void ScriptDevicePairingDelegate::SetKeyExchangeCallback(DevicePairingDelegate_OnPairingCompleteFunct callback)
{
    mOnPairingCompleteCallback = callback;
}

void ScriptDevicePairingDelegate::SetCommissioningCompleteCallback(DevicePairingDelegate_OnCommissioningCompleteFunct callback)
{
    mOnCommissioningCompleteCallback = callback;
}

void ScriptDevicePairingDelegate::OnPairingComplete(CHIP_ERROR error)
{
    if (mOnPairingCompleteCallback != nullptr)
    {
        mOnPairingCompleteCallback(error.AsInteger());
    }
}

void ScriptDevicePairingDelegate::OnCommissioningComplete(NodeId nodeId, CHIP_ERROR error)
{
    if (mOnCommissioningCompleteCallback != nullptr)
    {
        mOnCommissioningCompleteCallback(nodeId, error.AsInteger());
    }
}

} // namespace Controller
} // namespace chip
