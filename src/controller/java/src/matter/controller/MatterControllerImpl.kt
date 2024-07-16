/*
 *   Copyright (c) 2024 Project CHIP Authors
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
import chip.devicecontroller.GetConnectedDeviceCallbackJni.GetConnectedDeviceCallback
import java.util.logging.Level
import java.util.logging.Logger
import kotlin.coroutines.resume
import kotlin.coroutines.resumeWithException
import kotlinx.coroutines.channels.Channel.Factory.UNLIMITED
import kotlinx.coroutines.channels.awaitClose
import kotlinx.coroutines.channels.onFailure
import kotlinx.coroutines.channels.trySendBlocking
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.buffer
import kotlinx.coroutines.flow.callbackFlow
import kotlinx.coroutines.suspendCancellableCoroutine
import matter.controller.model.AttributePath
import matter.controller.model.EventPath
import matter.controller.model.EventState
import matter.controller.model.NodeState
import matter.controller.model.Status

/** Controller to interact with the CHIP device. */
class MatterControllerImpl(params: ControllerParams) : MatterController {
  private val deviceController: ChipDeviceController
  private var nodeId: Long? = null
  private val deviceControllerPtr: Long

  override fun setCompletionListener(listener: MatterController.CompletionListener?) =
    deviceController.setCompletionListener(CompletionListenerAdapter.from(listener))

  override fun pairDevice(
    nodeId: Long,
    address: String,
    port: Int,
    discriminator: Int,
    pinCode: Long,
  ) {
    this.nodeId = nodeId
    deviceController.pairDeviceWithAddress(nodeId, address, port, discriminator, pinCode, null)
  }

  override fun unpairDevice(nodeId: Long) {
    deviceController.unpairDevice(nodeId)
    this.nodeId = null
  }

  override fun establishPaseConnection(
    nodeId: Long,
    address: String,
    port: Int,
    setupPincode: Long
  ) {
    deviceController.establishPaseConnection(nodeId, address, port, setupPincode)
  }

  override fun subscribe(request: SubscribeRequest): Flow<SubscriptionState> {
    // To prevent potential issues related to concurrent modification, assign
    // the value of the mutable property 'nodeId' to a temporary variable.
    val nodeId = this.nodeId
    check(nodeId != null) { "nodeId has not been initialized yet" }

    val attributePaths = request.attributePaths
    val eventPaths = request.eventPaths
    val successes = mutableListOf<ReadData>()
    val failures = mutableListOf<ReadFailure>()

    return callbackFlow {
        val devicePtr: Long = getConnectedDevicePointer(nodeId)
        val subscriptionEstablishedHandler = SubscriptionEstablishedCallback {
          logger.log(Level.INFO, "Subscription to device established")

          trySendBlocking(SubscriptionState.SubscriptionEstablished).onFailure { ex ->
            logger.log(
              Level.SEVERE,
              "Error sending SubscriptionCompletedNotification to subscriber: %s",
              ex
            )
          }
        }

        val resubscriptionAttemptHandler =
          ResubscriptionAttemptCallback { terminationCause, nextResubscribeIntervalMsec ->
            logger.log(
              Level.WARNING,
              "ResubscriptionAttempt terminationCause:${terminationCause}, " +
                "nextResubscribeIntervalMsec:${nextResubscribeIntervalMsec}"
            )

            trySendBlocking(
                SubscriptionState.SubscriptionErrorNotification(terminationCause.toUInt())
              )
              .onFailure { ex ->
                logger.log(
                  Level.SEVERE,
                  "Error sending ResubscriptionNotification to subscriber: %s",
                  ex
                )
              }
          }

        val reportHandler =
          object : ReportCallback {
            override fun onReport(nodeState: NodeState) {
              logger.log(Level.FINE, "Received subscribe report")
              for (endpoint in nodeState.endpoints) {
                for (cluster in endpoint.value.clusters) {
                  for (attribute in cluster.value.attributes) {
                    val readData =
                      ReadData.Attribute(attribute.value.path, attribute.value.tlvValue)
                    successes.add(readData)
                  }

                  for (eventList in cluster.value.events) {
                    for (event in eventList.value) {
                      val timestamp: Timestamp =
                        when (event.getTimestampType()) {
                          EventState.TypeStampTypeEnum.MILLIS_SINCE_BOOT ->
                            Timestamp.MillisSinceBoot(event.timestampValue)
                          EventState.TypeStampTypeEnum.MILLIS_SINCE_EPOCH ->
                            Timestamp.MillisSinceEpoch(event.timestampValue)
                          else -> {
                            logger.log(Level.SEVERE, "Unsupported event timestamp type - ignoring")
                            break
                          }
                        }

                      val readData =
                        ReadData.Event(
                          path = event.path,
                          eventNumber = event.eventNumber.toULong(),
                          priorityLevel = event.priorityLevel.toUByte(),
                          timeStamp = timestamp,
                          data = event.tlvValue
                        )
                      successes.add(readData)
                    }
                  }
                }
              }
              trySendBlocking(SubscriptionState.NodeStateUpdate(ReadResponse(successes, failures)))
                .onFailure { ex ->
                  logger.log(Level.SEVERE, "Error sending NodeStateUpdate to subscriber: %s", ex)
                }
            }

            override fun onError(
              attributePath: AttributePath?,
              eventPath: EventPath?,
              e: Exception
            ) {
              attributePath?.let {
                logger.log(Level.INFO, "Report error for attributePath:%s", it.toString())
                val attributeFailure = ReadFailure.Attribute(path = it, error = e)
                failures.add(attributeFailure)
              }
              eventPath?.let {
                logger.log(Level.INFO, "Report error for eventPath:%s", it.toString())
                val eventFailure = ReadFailure.Event(path = it, error = e)
                failures.add(eventFailure)
              }
              if (attributePath == null && eventPath == null) {
                logger.log(Level.SEVERE, "The underlying subscription is terminated")

                trySendBlocking(
                    SubscriptionState.SubscriptionErrorNotification(CHIP_ERROR_UNEXPECTED_EVENT)
                  )
                  .onFailure { exception ->
                    logger.log(
                      Level.SEVERE,
                      "Error sending SubscriptionErrorNotification to subscriber: %s",
                      exception
                    )
                  }
              }
            }

            override fun onDone() {
              logger.log(Level.FINE, "subscribe command completed")
            }
          }

        val reportCallbackJni =
          ReportCallbackJni(
            subscriptionEstablishedHandler,
            reportHandler,
            resubscriptionAttemptHandler
          )

        val fabricIndex = getFabricIndex(devicePtr)
        val deviceId = getRemoteDeviceId(devicePtr)
        subscribe(
          deviceControllerPtr,
          reportCallbackJni.getJniHandle(),
          devicePtr,
          attributePaths,
          eventPaths,
          request.minInterval.seconds.toInt(),
          request.maxInterval.seconds.toInt(),
          request.keepSubscriptions,
          request.fabricFiltered,
          CHIP_IM_TIMEOUT_MS,
          MatterICDClientImpl.isPeerICDClient(fabricIndex, deviceId)
        )

        awaitClose { logger.log(Level.FINE, "Closing flow") }
      }
      .buffer(capacity = UNLIMITED)
  }

  private external fun subscribe(
    handle: Long,
    callbackHandle: Long,
    devicePtr: Long,
    attributePathList: List<AttributePath>,
    eventPathList: List<EventPath>,
    minInterval: Int,
    maxInterval: Int,
    keepSubscriptions: Boolean,
    isFabricFiltered: Boolean,
    imTimeoutMs: Int,
    isPeerLIT: Boolean
  )

  override suspend fun read(request: ReadRequest): ReadResponse {
    // To prevent potential issues related to concurrent modification, assign
    // the value of the mutable property 'nodeId' to a temporary variable.
    val nodeId = this.nodeId
    check(nodeId != null) { "nodeId has not been initialized yet" }

    val devicePtr: Long = getConnectedDevicePointer(nodeId)

    val successes = mutableListOf<ReadData>()
    val failures = mutableListOf<ReadFailure>()

    return suspendCancellableCoroutine { continuation ->
      val reportCallback =
        object : ReportCallback {
          override fun onReport(nodeState: NodeState) {
            logger.log(Level.FINE, "Received read report")
            for (endpoint in nodeState.endpoints) {
              for (cluster in endpoint.value.clusters) {
                for (attribute in cluster.value.attributes) {
                  val readData = ReadData.Attribute(attribute.value.path, attribute.value.tlvValue)
                  successes.add(readData)
                }

                for (eventList in cluster.value.events) {
                  for (event in eventList.value) {
                    val timestamp: Timestamp =
                      when (event.getTimestampType()) {
                        EventState.TypeStampTypeEnum.MILLIS_SINCE_BOOT ->
                          Timestamp.MillisSinceBoot(event.timestampValue)
                        EventState.TypeStampTypeEnum.MILLIS_SINCE_EPOCH ->
                          Timestamp.MillisSinceEpoch(event.timestampValue)
                        else -> {
                          logger.log(Level.SEVERE, "Unsupported event timestamp type - ignoring")
                          break
                        }
                      }

                    val readData =
                      ReadData.Event(
                        path = event.path,
                        eventNumber = event.eventNumber.toULong(),
                        priorityLevel = event.priorityLevel.toUByte(),
                        timeStamp = timestamp,
                        data = event.tlvValue
                      )
                    successes.add(readData)
                  }
                }
              }
            }
          }

          override fun onError(attributePath: AttributePath?, eventPath: EventPath?, e: Exception) {
            attributePath?.let {
              logger.log(Level.INFO, "Report error for attributePath:%s", it.toString())
              val attributeFailure = ReadFailure.Attribute(path = it, error = e)
              failures.add(attributeFailure)
            }
            eventPath?.let {
              logger.log(Level.INFO, "Report error for eventPath:%s", it.toString())
              val eventFailure = ReadFailure.Event(path = it, error = e)
              failures.add(eventFailure)
            }
          }

          override fun onDone() {
            logger.log(Level.FINE, "read command completed")
            continuation.resume(ReadResponse(successes, failures))
          }
        }
      val reportCallbackJni = ReportCallbackJni(null, reportCallback, null)
      read(
        deviceControllerPtr,
        reportCallbackJni.getJniHandle(),
        devicePtr,
        request.attributePaths,
        request.eventPaths,
        false,
        CHIP_IM_TIMEOUT_MS
      )
    }
  }

  private external fun read(
    handle: Long,
    callbackHandle: Long,
    devicePtr: Long,
    attributePathList: List<AttributePath>,
    eventPathList: List<EventPath>,
    isFabricFiltered: Boolean,
    imTimeoutMs: Int
  )

  override suspend fun write(writeRequests: WriteRequests): WriteResponse {
    // To prevent potential issues related to concurrent modification, assign
    // the value of the mutable property 'nodeId' to a temporary variable.
    val nodeId = this.nodeId
    check(nodeId != null) { "nodeId has not been initialized yet" }

    val devicePtr: Long = getConnectedDevicePointer(nodeId)

    val failures = mutableListOf<AttributeWriteError>()

    return suspendCancellableCoroutine { continuation ->
      val writeCallback =
        object : WriteAttributesCallback {
          override fun onResponse(attributePath: AttributePath, status: Status) {
            logger.log(
              Level.INFO,
              "Receive write response for attributePath: ${attributePath} and status ${status}"
            )
          }

          override fun onError(attributePath: AttributePath?, ex: Exception) {
            logger.log(
              Level.SEVERE,
              "Failed to write attribute at path: %s",
              attributePath.toString()
            )

            if (attributePath == null) {
              if (ex is ChipDeviceControllerException) {
                continuation.resumeWithException(
                  MatterControllerException(ex.errorCode, ex.message)
                )
              } else {
                continuation.resumeWithException(ex)
              }
            } else {
              failures.add(AttributeWriteError(attributePath, ex))
            }
          }

          override fun onDone() {
            logger.log(Level.INFO, "writeAttributes onDone is received")

            if (failures.isNotEmpty()) {
              continuation.resume(WriteResponse.PartialWriteFailure(failures))
            } else {
              continuation.resume(WriteResponse.Success)
            }
          }
        }
      val writeAttributeCallbackJni = WriteAttributesCallbackJni(writeCallback)
      write(
        deviceControllerPtr,
        writeAttributeCallbackJni.getCallbackHandle(),
        devicePtr,
        writeRequests.requests,
        writeRequests.timedRequest?.toMillis()?.toInt() ?: 0,
        CHIP_IM_TIMEOUT_MS,
      )
    }
  }

  private external fun write(
    handle: Long,
    callbackHandle: Long,
    devicePtr: Long,
    writeRequestList: List<WriteRequest>,
    timedRequestTimeoutMs: Int,
    imTimeoutMs: Int
  )

  override suspend fun invoke(request: InvokeRequest): InvokeResponse {
    // To prevent potential issues related to concurrent modification, assign
    // the value of the mutable property 'nodeId' to a temporary variable.
    val nodeId = this.nodeId
    check(nodeId != null) { "nodeId has not been initialized yet" }

    val devicePtr: Long = getConnectedDevicePointer(nodeId)

    return suspendCancellableCoroutine { continuation ->
      var invokeCallback =
        object : InvokeCallback {
          override fun onResponse(invokeResponse: InvokeResponse?, successCode: Long) {
            logger.log(Level.FINE, "Invoke onResponse is received")
            val ret =
              if (invokeResponse == null) {
                InvokeResponse(byteArrayOf(), request.commandPath, null)
              } else {
                invokeResponse
              }
            continuation.resume(ret)
          }

          override fun onError(ex: Exception) {
            if (ex is ChipDeviceControllerException) {
              continuation.resumeWithException(MatterControllerException(ex.errorCode, ex.message))
            } else {
              continuation.resumeWithException(ex)
            }
          }
        }
      val invokeCallbackJni = InvokeCallbackJni(invokeCallback)
      invoke(
        deviceControllerPtr,
        invokeCallbackJni.getJniHandle(),
        devicePtr,
        request,
        request.timedRequest?.toMillis()?.toInt() ?: 0,
        CHIP_IM_TIMEOUT_MS
      )
    }
  }

  private external fun invoke(
    handle: Long,
    callbackHandle: Long,
    devicePtr: Long,
    invokeRequest: InvokeRequest,
    timedRequestTimeoutMs: Int,
    imTimeoutMs: Int
  )

  external fun getRemoteDeviceId(devicePtr: Long): Long

  external fun getFabricIndex(devicePtr: Long): Int

  override fun close() {
    logger.log(Level.INFO, "MatterController is closed")
    deviceController.shutdownCommissioning()
  }

  private suspend fun getConnectedDevicePointer(nodeId: Long): Long {
    return suspendCancellableCoroutine { cont ->
      logger.log(Level.INFO, "Looking up pointer for %016X", nodeId)
      deviceController.getConnectedDevicePointer(
        nodeId,
        object : GetConnectedDeviceCallback {
          override fun onDeviceConnected(devicePointer: Long) {
            logger.log(Level.INFO, "Resolved pointer ${devicePointer} for device ${nodeId}")
            cont.resume(devicePointer)
          }

          override fun onConnectionFailure(nodeId: Long, error: Exception) {
            logger.log(Level.SEVERE, "Failed to establish CASE session for device ${nodeId}")
            cont.resumeWithException(
              Exception("Failed to establish CASE session for device %016X".format(nodeId))
            )
          }
        }
      )
    }
  }

  init {
    val config: OperationalKeyConfig? = params.operationalKeyConfig
    val paramsBuilder =
      chip.devicecontroller.ControllerParams.newBuilder()
        .setUdpListenPort(params.udpListenPort)
        .setControllerVendorId(params.vendorId)
        .setCountryCode(params.countryCode)
        .setEnableServerInteractions(params.enableServerInteractions)

    if (config != null) {
      val intermediateCertificate = config.certificateData.intermediateCertificate
      paramsBuilder
        .setRootCertificate(config.certificateData.trustedRootCertificate)
        .setIntermediateCertificate(intermediateCertificate ?: byteArrayOf())
        .setOperationalCertificate(config.certificateData.operationalCertificate)
        .setKeypairDelegate(config.keypairDelegate)
        .setIpk(config.ipk)
    }

    deviceController = ChipDeviceController(paramsBuilder.build())
    deviceControllerPtr = deviceController.deviceControllerPtr
  }

  companion object {
    private val logger = Logger.getLogger(MatterController::class.java.simpleName)

    // im interaction time out value, it would override the default value in c++ im
    // layer if this value is non-zero.
    private const val CHIP_IM_TIMEOUT_MS = 3000

    // CHIP error values, lift from ChipError.h in the Matter SDK.
    private const val CHIP_ERROR_UNEXPECTED_EVENT: UInt = 0xc0u
  }
}
