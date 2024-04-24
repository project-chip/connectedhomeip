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

import chip.devicecontroller.ICDDeviceInfo
import java.io.Closeable

/** Controller interface for interacting with a CHIP device. */
interface MatterController : Closeable, InteractionClient {
  /** Interface for listening to callbacks from the MatterController. */
  interface CompletionListener {
    /** Notifies the completion of the "ConnectDevice" command. */
    fun onConnectDeviceComplete()

    /** Notifies the pairing status. */
    fun onStatusUpdate(status: Int)

    /** Notifies the completion of pairing. */
    fun onPairingComplete(errorCode: UInt)

    /** Notifies the deletion of a pairing session. */
    fun onPairingDeleted(errorCode: UInt)

    /** Notifies that the CHIP connection has been closed. */
    fun onNotifyChipConnectionClosed()

    /** Notifies the completion of commissioning. */
    fun onCommissioningComplete(nodeId: Long, errorCode: UInt)

    /** Notifies the completion of reading commissioning information. */
    fun onReadCommissioningInfo(
      vendorId: Int,
      productId: Int,
      wifiEndpointId: Int,
      threadEndpointId: Int
    )

    /** Notifies the completion of each stage of commissioning. */
    fun onCommissioningStatusUpdate(nodeId: Long, stage: String?, errorCode: UInt)

    /** Notifies the listener of an error. */
    fun onError(error: Throwable)

    /** Notifies the Commissioner when the OpCSR for the Comissionee is generated. */
    fun onOpCSRGenerationComplete(csr: ByteArray)

    /**
     * Notifies when the ICD registration information (ICD symmetric key, check-in node ID and
     * monitored subject) is required.
     */
    fun onICDRegistrationInfoRequired()

    /** Notifies when the registration flow for the ICD completes. */
    fun onICDRegistrationComplete(errorCode: UInt, icdDeviceInfo: ICDDeviceInfo)
  }

  /**
   * Sets a completion listener for receiving controller events.
   *
   * @param listener The listener to set.
   */
  fun setCompletionListener(listener: CompletionListener?)

  /**
   * Commissions a device into a Matter fabric.
   *
   * @param nodeId The ID of the node to connect to.
   * @param address The IP address at which the node is located.
   * @param port The port at which the node is located.
   * @param discriminator A 12-bit value used to discern between multiple commissionable Matter
   *   device advertisements.
   * @param pinCode The pincode for this node.
   */
  fun pairDevice(
    nodeId: Long,
    address: String,
    port: Int,
    discriminator: Int,
    pinCode: Long,
  )

  /**
   * Removes pairing for a paired device. If the device is currently being paired, it will stop the
   * pairing process.
   *
   * @param nodeId The remote device ID.
   */
  fun unpairDevice(nodeId: Long)

  /**
   * Establishes a secure PASE connection to the given device via IP address.
   *
   * @param nodeId The ID of the node to connect to.
   * @param address The IP address at which the node is located.
   * @param port The port at which the node is located.
   * @param setupPincode The pincode for this node.
   */
  fun establishPaseConnection(nodeId: Long, address: String, port: Int, setupPincode: Long)

  /** Closes any active connections through this device controller. */
  override fun close()
}
