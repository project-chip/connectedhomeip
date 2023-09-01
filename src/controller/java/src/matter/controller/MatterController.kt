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
import chip.devicecontroller.ChipDeviceControllerException
import chip.devicecontroller.model.AttributeWriteRequest
import chip.devicecontroller.model.ChipAttributePath
import chip.devicecontroller.model.ChipEventPath
import chip.devicecontroller.model.ChipPathId
import java.util.logging.Level
import java.util.logging.Logger

/** Controller to interact with the CHIP device. */
class MatterController(params: ControllerParams) {
  private val deviceController: ChipDeviceController
  private var completionListener: CompletionListener? = null

  /** Interface to listen for callbacks from MatterController. */
  interface CompletionListener {
    /** Notifies the completion of "ConnectDevice" command. */
    fun onConnectDeviceComplete()

    /** Notifies the pairing status. */
    fun onStatusUpdate(status: Int)

    /** Notifies the completion of pairing. */
    fun onPairingComplete(errorCode: Int)

    /** Notifies the deletion of pairing session. */
    fun onPairingDeleted(errorCode: Int)

    /** Notifies the completion of commissioning. */
    fun onCommissioningComplete(nodeId: Long, errorCode: Int)

    /** Notifies the completion of each stage of commissioning. */
    fun onReadCommissioningInfo(
      vendorId: Int,
      productId: Int,
      wifiEndpointId: Int,
      threadEndpointId: Int
    )

    /** Notifies the completion of each stage of commissioning. */
    fun onCommissioningStatusUpdate(nodeId: Long, stage: String?, errorCode: Int)

    /** Notifies the listener of the error. */
    fun onError(error: Throwable)

    /** Notifies the Commissioner when the OpCSR for the Comissionee is generated. */
    fun onOpCSRGenerationComplete(csr: ByteArray)
  }

  fun setCompletionListener(listener: CompletionListener) {
    completionListener = listener
  }

  fun pairDeviceWithAddress(
    deviceId: Long,
    address: String,
    port: Int,
    discriminator: Int,
    pinCode: Long,
    csrNonce: ByteArray?
  ) {
    deviceController.pairDeviceWithAddress(
      deviceId,
      address,
      port,
      discriminator,
      pinCode,
      csrNonce
    )
  }

  /**
   * Establish a secure PASE connection to the given device via IP address.
   *
   * @param deviceId the ID of the node to connect to
   * @param address the IP address at which the node is located
   * @param port the port at which the node is located
   * @param setupPincode the pincode for this node
   */
  fun establishPaseConnection(deviceId: Long, address: String, port: Int, setupPincode: Long) {
    deviceController.establishPaseConnection(deviceId, address, port, setupPincode)
  }

  fun unpairDevice(deviceId: Long) {
    deviceController.unpairDevice(deviceId)
  }

  fun onConnectDeviceComplete() {
    completionListener?.onConnectDeviceComplete()
  }

  fun onStatusUpdate(status: Int) {
    completionListener?.onStatusUpdate(status)
  }

  fun onPairingComplete(errorCode: Int) {
    completionListener?.onPairingComplete(errorCode)
  }

  fun onCommissioningComplete(nodeId: Long, errorCode: Int) {
    completionListener?.onCommissioningComplete(nodeId, errorCode)
  }

  fun onCommissioningStatusUpdate(nodeId: Long, stage: String?, errorCode: Int) {
    completionListener?.onCommissioningStatusUpdate(nodeId, stage, errorCode)
  }

  fun onReadCommissioningInfo(
    vendorId: Int,
    productId: Int,
    wifiEndpointId: Int,
    threadEndpointId: Int
  ) {
    completionListener?.onReadCommissioningInfo(
      vendorId,
      productId,
      wifiEndpointId,
      threadEndpointId
    )
  }

  fun onOpCSRGenerationComplete(csr: ByteArray) {
    completionListener?.onOpCSRGenerationComplete(csr)
  }

  fun onPairingDeleted(errorCode: Int) {
    completionListener?.onPairingDeleted(errorCode)
  }

  fun onError(error: Throwable) {
    completionListener?.onError(error)
  }

  /**
   * Subscribe to periodic updates of all attributes and events from a device.
   *
   * @param SubscriptionEstablishedCallback Callback when a subscribe response has been received and
   *   processed
   * @param ResubscriptionAttemptCallback Callback when a resubscirption haoppens, the termination
   *   cause is provided to help inform subsequent re-subscription logic.
   * @param ReportCallback Callback when a report data has been received and processed for the given
   *   paths.
   * @param devicePtr connected device pointer
   * @param subscribeElement subscribe command's path and arguments
   */
  fun subscribeDevice(
    subscriptionEstablishedCallback: SubscriptionEstablishedCallback,
    resubscriptionAttemptCallback: ResubscriptionAttemptCallback,
    reportCallback: ReportCallback,
    devicePtr: Long,
    subscribeElement: SubscribeElement
  ) {
    val subscriptionEstablishedHandler =
      chip.devicecontroller.SubscriptionEstablishedCallback { subscriptionId ->
        logger.log(Level.INFO, "Subscription to device established")
        subscriptionEstablishedCallback.onSubscriptionEstablished(subscriptionId)
      }

    val resubscriptionAttemptHandler =
      chip.devicecontroller.ResubscriptionAttemptCallback {
        terminationCause,
        nextResubscribeIntervalMsec ->
        logger.log(
          Level.WARNING,
          String.format(
            "ResubscriptionAttempt terminationCause:%d, nextResubscribeIntervalMsec:%d",
            terminationCause,
            nextResubscribeIntervalMsec
          )
        )
        resubscriptionAttemptCallback.onResubscriptionAttempt(
          terminationCause,
          nextResubscribeIntervalMsec
        )
      }

    val reportHandler =
      object : chip.devicecontroller.ReportCallback {
        override fun onError(
          attributePath: chip.devicecontroller.model.ChipAttributePath?,
          eventPath: chip.devicecontroller.model.ChipEventPath?,
          ex: Exception
        ) {
          val tmpAttributePath: AttributePath? = attributePath?.wrap()
          val tmpEventPath: EventPath? = eventPath?.wrap()

          reportCallback.onError(tmpAttributePath, tmpEventPath, ex)
        }

        override fun onReport(nodeState: chip.devicecontroller.model.NodeState) {
          val tmpNodeState: NodeState = nodeState.wrap()
          reportCallback.onReport(tmpNodeState)
        }

        override fun onDone() {
          reportCallback.onDone()
        }
      }

    deviceController.subscribeToPath(
      subscriptionEstablishedHandler,
      resubscriptionAttemptHandler,
      reportHandler,
      devicePtr,
      listOf(
        chip.devicecontroller.model.ChipAttributePath.newInstance(
          /* endpointId= */ WILDCARD_ENDPOINT_ID,
          /* clusterId=*/ WILDCARD_CLUSTER_ID,
          /* attributeId= */ WILDCARD_ATTRIBUTE_ID
        )
      ),
      listOf(
        chip.devicecontroller.model.ChipEventPath.newInstance(
          /* endpointId= */ WILDCARD_ENDPOINT_ID,
          /* clusterId=*/ WILDCARD_CLUSTER_ID,
          /* eventId= */ WILDCARD_EVENT_ID
        )
      ),
      subscribeElement.minInterval,
      subscribeElement.maxInterval,
      subscribeElement.keepSubscriptions,
      subscribeElement.isFabricFiltered,
      CHIP_IM_TIMEOUT_MS
    )
  }

  /**
   * Issue attribute write requests to target device
   *
   * @param WriteAttributesCallback Callback when a write response has been received and processed
   *   for the given path.
   * @param devicePtr connected device pointer
   * @param attributeWriteRequests a list of attribute WriteRequest
   * @param timedRequestTimeoutMs this is timed request if this value is set
   */
  fun writeAttributes(
    callback: WriteAttributesCallback,
    devicePtr: Long,
    writeRequests: List<WriteRequest>,
    timedRequestTimeoutMs: Int?
  ) {
    val attributeWriteRequests =
      writeRequests.map { request ->
        AttributeWriteRequest.newInstance(
          ChipPathId.forId(request.endpoint.toLong()),
          ChipPathId.forId(request.cluster.toLong()),
          ChipPathId.forId(request.attribute.toLong()),
          request.tlv
        )
      }

    val writeCallback =
      object : chip.devicecontroller.WriteAttributesCallback {
        override fun onResponse(attributePath: ChipAttributePath) {
          logger.log(Level.INFO, "write success for attributePath:%s", attributePath.toString())

          val tmpAttributePath: AttributePath = attributePath.wrap()
          callback.onSuccess(tmpAttributePath)
        }

        override fun onError(attributePath: ChipAttributePath?, ex: Exception) {
          logger.log(Level.SEVERE, "Failed to write attribute at path: %s", attributePath)

          val tmpAttributePath: AttributePath? = attributePath?.wrap()
          callback.onError(tmpAttributePath, ex)
        }

        override fun onDone() {
          logger.log(Level.INFO, "writeAttributes onDone is received")

          callback.onDone()
        }
      }

    deviceController.write(
      writeCallback,
      devicePtr,
      attributeWriteRequests.toList(),
      timedRequestTimeoutMs ?: 0,
      CHIP_IM_TIMEOUT_MS,
    )
  }

  /**
   * Invoke command to target device
   *
   * @param InvokeCallback Callback when an invoke response has been received and processed for the
   *   given invoke command.
   * @param devicePtr connected device pointer
   * @param invokeElement invoke command's path and arguments
   * @param timedRequestTimeoutMs this is timed request if this value is set
   */
  fun invokeCommand(
    callback: InvokeCallback,
    devicePtr: Long,
    invokeElement: InvokeElement,
    timedRequestTimeoutMs: Int?
  ) {
    val invokeRequest =
      chip.devicecontroller.model.InvokeElement.newInstance(
        ChipPathId.forId(invokeElement.endpointId.toLong()),
        ChipPathId.forId(invokeElement.clusterId),
        ChipPathId.forId(invokeElement.commandId),
        invokeElement.tlvValue,
        /* jsonString= */ null
      )

    var invokeCallback =
      object : chip.devicecontroller.InvokeCallback {
        override fun onResponse(
          invokeElement: chip.devicecontroller.model.InvokeElement,
          successCode: Long
        ) {
          logger.log(Level.INFO, "Invoke onResponse is received")

          val tmpInvokeElement: InvokeElement = invokeElement.wrap()
          callback.onResponse(tmpInvokeElement, successCode)
        }

        override fun onError(ex: Exception) {
          logger.log(Level.SEVERE, "Invoke onError is received: %s", ex.message)
          callback.onError(ex)
        }
      }

    deviceController.invoke(
      invokeCallback,
      devicePtr,
      invokeRequest,
      timedRequestTimeoutMs ?: 0,
      CHIP_IM_TIMEOUT_MS
    )
  }

  private fun ChipAttributePath.wrap(): AttributePath {
    return AttributePath(endpointId.getId().toInt(), clusterId.getId(), attributeId.getId())
  }

  private fun ChipEventPath.wrap(): EventPath {
    return EventPath(endpointId.getId().toInt(), clusterId.getId(), eventId.getId())
  }

  private fun chip.devicecontroller.model.NodeState.wrap(): NodeState {
    return NodeState(
      endpoints = endpointStates.mapValues { (id, value) -> value.wrap(id) },
      events = arrayListOf()
    )
  }

  private fun chip.devicecontroller.model.EndpointState.wrap(id: Int): EndpointState {
    return EndpointState(id, clusterStates.mapValues { (id, value) -> value.wrap(id) })
  }

  private fun chip.devicecontroller.model.ClusterState.wrap(id: Long): ClusterState {
    return ClusterState(id, attributeStates.mapValues { (id, value) -> value.wrap(id) })
  }

  private fun chip.devicecontroller.model.AttributeState.wrap(id: Long): AttributeState {
    return AttributeState(id, tlv, json.toString())
  }

  private fun chip.devicecontroller.model.InvokeElement.wrap(): InvokeElement {
    return matter.controller.InvokeElement(
      endpointId.getId().toInt(),
      clusterId.getId(),
      commandId.getId(),
      getTlvByteArray()!!
    )
  }

  init {
    val config: OperationalKeyConfig? = params.operationalKeyConfig
    val paramsBuilder =
      chip.devicecontroller.ControllerParams.newBuilder()
        .setUdpListenPort(params.udpListenPort)
        .setControllerVendorId(params.vendorId)
        .setCountryCode(params.countryCode)

    if (config != null) {
      val intermediateCertificate = config.intermediateCertificate
      paramsBuilder
        .setRootCertificate(config.trustedRootCertificate)
        .setIntermediateCertificate(intermediateCertificate ?: byteArrayOf())
        .setOperationalCertificate(config.operationalCertificate)
        .setKeypairDelegate(config.keypairDelegate)
        .setIpk(config.ipk)
    }

    try {
      deviceController = ChipDeviceController(paramsBuilder.build())
    } catch (ex: ChipDeviceControllerException) {
      throw MatterControllerException(ex.errorCode, "Failed to initialize ChipDeviceController.")
    }
  }

  companion object {
    private val logger = Logger.getLogger(MatterController::class.java.simpleName)
    private const val WILDCARD_ENDPOINT_ID = 0xffff
    private const val WILDCARD_CLUSTER_ID = 0xffffffff
    private const val WILDCARD_ATTRIBUTE_ID = 0xffffffff
    private const val WILDCARD_EVENT_ID = 0xffffffff
    // im interaction time out value, it would override the default value in c++ im
    // layer if this value is non-zero.
    private const val CHIP_IM_TIMEOUT_MS = 0
  }
}
