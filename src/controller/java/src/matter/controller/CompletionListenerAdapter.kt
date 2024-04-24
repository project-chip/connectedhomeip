/*
 *   Copyright (c) 2023 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
package matter.controller

import chip.devicecontroller.ChipDeviceController
import chip.devicecontroller.ICDDeviceInfo
import java.util.logging.Level
import java.util.logging.Logger

class CompletionListenerAdapter(val listener: MatterController.CompletionListener) :
  chip.devicecontroller.ChipDeviceController.CompletionListener {

  override fun onConnectDeviceComplete() = listener.onConnectDeviceComplete()

  override fun onStatusUpdate(status: Int) = listener.onStatusUpdate(status)

  override fun onPairingComplete(errorCode: Long) = listener.onPairingComplete(errorCode.toUInt())

  override fun onPairingDeleted(errorCode: Long) = listener.onPairingDeleted(errorCode.toUInt())

  override fun onNotifyChipConnectionClosed() = listener.onNotifyChipConnectionClosed()

  override fun onCommissioningComplete(nodeId: Long, errorCode: Long) =
    listener.onCommissioningComplete(nodeId, errorCode.toUInt())

  override fun onCommissioningStatusUpdate(nodeId: Long, stage: String?, errorCode: Long) =
    listener.onCommissioningStatusUpdate(nodeId, stage, errorCode.toUInt())

  override fun onReadCommissioningInfo(
    vendorId: Int,
    productId: Int,
    wifiEndpointId: Int,
    threadEndpointId: Int
  ) = listener.onReadCommissioningInfo(vendorId, productId, wifiEndpointId, threadEndpointId)

  override fun onOpCSRGenerationComplete(csr: ByteArray) = listener.onOpCSRGenerationComplete(csr)

  override fun onICDRegistrationInfoRequired() = listener.onICDRegistrationInfoRequired()

  override fun onICDRegistrationComplete(errorCode: Long, icdDeviceInfo: ICDDeviceInfo) =
    listener.onICDRegistrationComplete(errorCode.toUInt(), icdDeviceInfo)

  override fun onError(error: Throwable) = listener.onError(error)

  override fun onCloseBleComplete() {
    logger.log(Level.INFO, "Not implemented, override the abstract function.")
  }

  companion object {
    private val logger = Logger.getLogger(MatterController::class.java.simpleName)

    fun from(
      listener: MatterController.CompletionListener?
    ): chip.devicecontroller.ChipDeviceController.CompletionListener? {
      if (listener == null) {
        return null
      }
      return CompletionListenerAdapter(listener)
    }
  }
}
