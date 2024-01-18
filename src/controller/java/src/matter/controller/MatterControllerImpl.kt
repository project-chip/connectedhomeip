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
import chip.devicecontroller.GetConnectedDeviceCallbackJni.GetConnectedDeviceCallback
import chip.devicecontroller.InvokeCallback
import chip.devicecontroller.ReportCallback
import chip.devicecontroller.ResubscriptionAttemptCallback
import chip.devicecontroller.SubscriptionEstablishedCallback
import chip.devicecontroller.WriteAttributesCallback
import chip.devicecontroller.model.AttributeWriteRequest
import chip.devicecontroller.model.ChipAttributePath
import chip.devicecontroller.model.ChipEventPath
import chip.devicecontroller.model.ChipPathId
import chip.devicecontroller.model.InvokeElement
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
import matter.controller.model.AttributeState
import matter.controller.model.ClusterState
import matter.controller.model.EndpointState
import matter.controller.model.EventPath
import matter.controller.model.EventState
import matter.controller.model.NodeState

/** Controller to interact with the CHIP device. */
class MatterControllerImpl(params: ControllerParams) : MatterController {
  private val deviceController: ChipDeviceController
  private var nodeId: Long? = null

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

    val attributePaths = generateAttributePaths(request)
    val eventPaths = generateEventPaths(request)
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
            override fun onReport(nodeState: chip.devicecontroller.model.NodeState) {
              logger.log(Level.INFO, "Received subscribe update report")

              val tmpNodeState: NodeState = nodeState.wrap()

              for (endpoint in tmpNodeState.endpoints) {
                for (cluster in endpoint.value.clusters) {
                  for (attribute in cluster.value.attributes) {
                    val attributePath =
                      AttributePath(
                        endpointId = endpoint.key.toUShort(),
                        clusterId = cluster.key.toUInt(),
                        attributeId = attribute.key.toUInt()
                      )
                    val readData = ReadData.Attribute(attributePath, attribute.value.tlvValue)
                    successes.add(readData)
                  }

                  for (eventList in cluster.value.events) {
                    for (event in eventList.value) {
                      val timestamp: Timestamp =
                        when (event.timestampType) {
                          chip.devicecontroller.model.EventState.MILLIS_SINCE_BOOT ->
                            Timestamp.MillisSinceBoot(event.timestampValue)
                          chip.devicecontroller.model.EventState.MILLIS_SINCE_EPOCH ->
                            Timestamp.MillisSinceEpoch(event.timestampValue)
                          else -> {
                            logger.log(Level.SEVERE, "Unsupported event timestamp type - ignoring")
                            break
                          }
                        }

                      val eventPath =
                        EventPath(
                          endpointId = endpoint.key.toUShort(),
                          clusterId = cluster.key.toUInt(),
                          eventId = eventList.key.toUInt()
                        )

                      val readData =
                        ReadData.Event(
                          path = eventPath,
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
              attributePath: ChipAttributePath?,
              eventPath: ChipEventPath?,
              ex: Exception
            ) {
              attributePath?.let {
                logger.log(Level.INFO, "Report error for attributePath:%s", it.toString())
                val tmpAttributePath: AttributePath = attributePath.wrap()
                val attributeFailure = ReadFailure.Attribute(path = tmpAttributePath, error = ex)
                failures.add(attributeFailure)
              }
              eventPath?.let {
                logger.log(Level.INFO, "Report error for eventPath:%s", it.toString())
                val tmpEventPath: EventPath = eventPath.wrap()
                val eventFailure = ReadFailure.Event(path = tmpEventPath, error = ex)
                failures.add(eventFailure)
              }

              // The underlying subscription is terminated if both attributePath & eventPath are
              // null
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
              logger.log(Level.INFO, "Subscription update completed")
            }
          }

        deviceController.subscribeToPath(
          subscriptionEstablishedHandler,
          resubscriptionAttemptHandler,
          reportHandler,
          devicePtr,
          attributePaths,
          eventPaths,
          request.minInterval.seconds.toInt(),
          request.maxInterval.seconds.toInt(),
          request.keepSubscriptions,
          request.fabricFiltered,
          CHIP_IM_TIMEOUT_MS
        )

        awaitClose { logger.log(Level.FINE, "Closing flow") }
      }
      .buffer(capacity = UNLIMITED)
  }

  override suspend fun read(request: ReadRequest): ReadResponse {
    // To prevent potential issues related to concurrent modification, assign
    // the value of the mutable property 'nodeId' to a temporary variable.
    val nodeId = this.nodeId
    check(nodeId != null) { "nodeId has not been initialized yet" }

    val devicePtr: Long = getConnectedDevicePointer(nodeId)

    val chipAttributePaths =
      request.attributePaths.map { attributePath ->
        val endpointId = attributePath.endpointId.toInt()
        val clusterId = attributePath.clusterId.toLong()
        val attributeId = attributePath.attributeId.toLong()
        ChipAttributePath.newInstance(endpointId, clusterId, attributeId)
      }

    val chipEventPaths =
      request.eventPaths.map { eventPath ->
        val endpointId = eventPath.endpointId.toInt()
        val clusterId = eventPath.clusterId.toLong()
        val eventId = eventPath.eventId.toLong()
        ChipEventPath.newInstance(endpointId, clusterId, eventId)
      }

    val successes = mutableListOf<ReadData>()
    val failures = mutableListOf<ReadFailure>()

    return suspendCancellableCoroutine { continuation ->
      val reportCallback =
        object : ReportCallback {
          override fun onReport(nodeState: chip.devicecontroller.model.NodeState) {
            logger.log(Level.FINE, "Received read report")

            val tmpNodeState: NodeState = nodeState.wrap()

            for (endpoint in tmpNodeState.endpoints) {
              for (cluster in endpoint.value.clusters) {
                for (attribute in cluster.value.attributes) {
                  val attributePath =
                    AttributePath(
                      endpointId = endpoint.key.toUShort(),
                      clusterId = cluster.key.toUInt(),
                      attributeId = attribute.key.toUInt()
                    )
                  val readData = ReadData.Attribute(attributePath, attribute.value.tlvValue)
                  successes.add(readData)
                }

                for (eventList in cluster.value.events) {
                  for (event in eventList.value) {
                    val timestamp: Timestamp =
                      when (event.timestampType) {
                        chip.devicecontroller.model.EventState.MILLIS_SINCE_BOOT ->
                          Timestamp.MillisSinceBoot(event.timestampValue)
                        chip.devicecontroller.model.EventState.MILLIS_SINCE_EPOCH ->
                          Timestamp.MillisSinceEpoch(event.timestampValue)
                        else -> {
                          logger.log(Level.SEVERE, "Unsupported event timestamp type - ignoring")
                          break
                        }
                      }
                    val eventPath =
                      EventPath(
                        endpointId = endpoint.key.toUShort(),
                        clusterId = cluster.key.toUInt(),
                        eventId = eventList.key.toUInt()
                      )

                    val readData =
                      ReadData.Event(
                        path = eventPath,
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

          override fun onError(
            attributePath: ChipAttributePath?,
            eventPath: ChipEventPath?,
            ex: Exception
          ) {
            attributePath?.let {
              logger.log(Level.INFO, "Report error for attributePath:%s", it.toString())
              val tmpAttributePath: AttributePath = attributePath.wrap()
              val attributeFailure = ReadFailure.Attribute(path = tmpAttributePath, error = ex)
              failures.add(attributeFailure)
            }
            eventPath?.let {
              logger.log(Level.INFO, "Report error for eventPath:%s", it.toString())
              val tmpEventPath: EventPath = eventPath.wrap()
              val eventFailure = ReadFailure.Event(path = tmpEventPath, error = ex)
              failures.add(eventFailure)
            }

            // The underlying subscription is terminated if both attributePath & eventPath are null
            if (attributePath == null && eventPath == null) {
              continuation.resumeWithException(
                Exception("Read command failed with error ${ex.message}")
              )
            }
          }

          override fun onDone() {
            logger.log(Level.FINE, "read command completed")
            continuation.resume(ReadResponse(successes, failures))
          }
        }

      deviceController.readPath(
        reportCallback,
        devicePtr,
        chipAttributePaths,
        chipEventPaths,
        false,
        CHIP_IM_TIMEOUT_MS,
      )
    }
  }

  override suspend fun write(writeRequests: WriteRequests): WriteResponse {
    // To prevent potential issues related to concurrent modification, assign
    // the value of the mutable property 'nodeId' to a temporary variable.
    val nodeId = this.nodeId
    check(nodeId != null) { "nodeId has not been initialized yet" }

    val devicePtr: Long = getConnectedDevicePointer(nodeId)

    val attributeWriteRequests =
      writeRequests.requests.map { request ->
        AttributeWriteRequest.newInstance(
          ChipPathId.forId(request.attributePath.endpointId.toLong()),
          ChipPathId.forId(request.attributePath.clusterId.toLong()),
          ChipPathId.forId(request.attributePath.attributeId.toLong()),
          request.tlvPayload
        )
      }

    val failures = mutableListOf<AttributeWriteError>()

    return suspendCancellableCoroutine { continuation ->
      val writeCallback =
        object : WriteAttributesCallback {
          override fun onResponse(attributePath: ChipAttributePath) {
            logger.log(Level.INFO, "write success for attributePath:%s", attributePath.toString())
          }

          override fun onError(attributePath: ChipAttributePath?, ex: Exception) {
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
              failures.add(AttributeWriteError(attributePath.wrap(), ex))
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

      deviceController.write(
        writeCallback,
        devicePtr,
        attributeWriteRequests.toList(),
        writeRequests.timedRequest?.toMillis()?.toInt() ?: 0,
        CHIP_IM_TIMEOUT_MS,
      )
    }
  }

  override suspend fun invoke(request: InvokeRequest): InvokeResponse {
    // To prevent potential issues related to concurrent modification, assign
    // the value of the mutable property 'nodeId' to a temporary variable.
    val nodeId = this.nodeId
    check(nodeId != null) { "nodeId has not been initialized yet" }

    val devicePtr: Long = getConnectedDevicePointer(nodeId)

    val invokeRequest =
      InvokeElement.newInstance(
        ChipPathId.forId(request.commandPath.endpointId.toLong()),
        ChipPathId.forId(request.commandPath.clusterId.toLong()),
        ChipPathId.forId(request.commandPath.commandId.toLong()),
        request.tlvPayload,
        /* jsonString= */ null
      )

    return suspendCancellableCoroutine { continuation ->
      var invokeCallback =
        object : InvokeCallback {
          override fun onResponse(invokeElement: InvokeElement?, successCode: Long) {
            logger.log(Level.FINE, "Invoke onResponse is received")
            val tlvByteArray = invokeElement?.getTlvByteArray() ?: byteArrayOf()
            continuation.resume(InvokeResponse(tlvByteArray))
          }

          override fun onError(ex: Exception) {
            if (ex is ChipDeviceControllerException) {
              continuation.resumeWithException(MatterControllerException(ex.errorCode, ex.message))
            } else {
              continuation.resumeWithException(ex)
            }
          }
        }

      deviceController.invoke(
        invokeCallback,
        devicePtr,
        invokeRequest,
        request.timedRequest?.toMillis()?.toInt() ?: 0,
        CHIP_IM_TIMEOUT_MS
      )
    }
  }

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

  private fun generateAttributePaths(request: SubscribeRequest): List<ChipAttributePath> {
    return request.attributePaths.map { attributePath ->
      ChipAttributePath.newInstance(
        attributePath.endpointId.toInt(),
        attributePath.clusterId.toLong(),
        attributePath.attributeId.toLong()
      )
    }
  }

  private fun generateEventPaths(request: SubscribeRequest): List<ChipEventPath> {
    return request.eventPaths.map { eventPath ->
      ChipEventPath.newInstance(
        eventPath.endpointId.toInt(),
        eventPath.clusterId.toLong(),
        eventPath.eventId.toLong(),
        false
      )
    }
  }

  private fun ChipAttributePath.wrap(): AttributePath {
    return AttributePath(
      endpointId.getId().toUShort(),
      clusterId.getId().toUInt(),
      attributeId.getId().toUInt()
    )
  }

  private fun ChipEventPath.wrap(): EventPath {
    return EventPath(
      endpointId.getId().toUShort(),
      clusterId.getId().toUInt(),
      eventId.getId().toUInt()
    )
  }

  private fun chip.devicecontroller.model.NodeState.wrap(): NodeState {
    return NodeState(
      endpoints = endpointStates.mapValues { (id, value) -> value.wrap(id) },
    )
  }

  private fun chip.devicecontroller.model.EndpointState.wrap(id: Int): EndpointState {
    return EndpointState(id, clusterStates.mapValues { (id, value) -> value.wrap(id) })
  }

  private fun chip.devicecontroller.model.ClusterState.wrap(id: Long): ClusterState {
    return ClusterState(
      id,
      attributeStates.mapValues { (id, value) -> value.wrap(id) },
      eventStates.mapValues { (id, value) -> value.map { eventState -> eventState.wrap(id) } }
    )
  }

  private fun chip.devicecontroller.model.AttributeState.wrap(id: Long): AttributeState {
    return AttributeState(id, tlv, json.toString())
  }

  private fun chip.devicecontroller.model.EventState.wrap(id: Long): EventState {
    return EventState(id, eventNumber, priorityLevel, timestampType, timestampValue, tlv)
  }

  init {
    val config: OperationalKeyConfig? = params.operationalKeyConfig
    val paramsBuilder =
      chip.devicecontroller.ControllerParams.newBuilder()
        .setUdpListenPort(params.udpListenPort)
        .setControllerVendorId(params.vendorId)
        .setCountryCode(params.countryCode)

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
