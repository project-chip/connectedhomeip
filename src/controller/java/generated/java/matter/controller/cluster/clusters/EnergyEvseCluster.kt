/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

package matter.controller.cluster.clusters

import java.time.Duration
import java.util.logging.Level
import java.util.logging.Logger
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.transform
import matter.controller.InvokeRequest
import matter.controller.InvokeResponse
import matter.controller.LongSubscriptionState
import matter.controller.MatterController
import matter.controller.ReadData
import matter.controller.ReadRequest
import matter.controller.SubscribeRequest
import matter.controller.SubscriptionState
import matter.controller.UByteSubscriptionState
import matter.controller.UIntSubscriptionState
import matter.controller.UShortSubscriptionState
import matter.controller.WriteRequest
import matter.controller.WriteRequests
import matter.controller.WriteResponse
import matter.controller.cluster.structs.*
import matter.controller.model.AttributePath
import matter.controller.model.CommandPath
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class EnergyEvseCluster(private val controller: MatterController, private val endpointId: UShort) {
  class GetTargetsResponse(
    val chargingTargetSchedules: List<EnergyEvseClusterChargingTargetScheduleStruct>
  )

  class StateAttribute(val value: UByte?)

  sealed class StateAttributeSubscriptionState {
    data class Success(val value: UByte?) : StateAttributeSubscriptionState()

    data class Error(val exception: Exception) : StateAttributeSubscriptionState()

    object SubscriptionEstablished : StateAttributeSubscriptionState()
  }

  class ChargingEnabledUntilAttribute(val value: UInt?)

  sealed class ChargingEnabledUntilAttributeSubscriptionState {
    data class Success(val value: UInt?) : ChargingEnabledUntilAttributeSubscriptionState()

    data class Error(val exception: Exception) : ChargingEnabledUntilAttributeSubscriptionState()

    object SubscriptionEstablished : ChargingEnabledUntilAttributeSubscriptionState()
  }

  class DischargingEnabledUntilAttribute(val value: UInt?)

  sealed class DischargingEnabledUntilAttributeSubscriptionState {
    data class Success(val value: UInt?) : DischargingEnabledUntilAttributeSubscriptionState()

    data class Error(val exception: Exception) :
      DischargingEnabledUntilAttributeSubscriptionState()

    object SubscriptionEstablished : DischargingEnabledUntilAttributeSubscriptionState()
  }

  class NextChargeStartTimeAttribute(val value: UInt?)

  sealed class NextChargeStartTimeAttributeSubscriptionState {
    data class Success(val value: UInt?) : NextChargeStartTimeAttributeSubscriptionState()

    data class Error(val exception: Exception) : NextChargeStartTimeAttributeSubscriptionState()

    object SubscriptionEstablished : NextChargeStartTimeAttributeSubscriptionState()
  }

  class NextChargeTargetTimeAttribute(val value: UInt?)

  sealed class NextChargeTargetTimeAttributeSubscriptionState {
    data class Success(val value: UInt?) : NextChargeTargetTimeAttributeSubscriptionState()

    data class Error(val exception: Exception) : NextChargeTargetTimeAttributeSubscriptionState()

    object SubscriptionEstablished : NextChargeTargetTimeAttributeSubscriptionState()
  }

  class NextChargeRequiredEnergyAttribute(val value: Long?)

  sealed class NextChargeRequiredEnergyAttributeSubscriptionState {
    data class Success(val value: Long?) : NextChargeRequiredEnergyAttributeSubscriptionState()

    data class Error(val exception: Exception) :
      NextChargeRequiredEnergyAttributeSubscriptionState()

    object SubscriptionEstablished : NextChargeRequiredEnergyAttributeSubscriptionState()
  }

  class NextChargeTargetSoCAttribute(val value: UByte?)

  sealed class NextChargeTargetSoCAttributeSubscriptionState {
    data class Success(val value: UByte?) : NextChargeTargetSoCAttributeSubscriptionState()

    data class Error(val exception: Exception) : NextChargeTargetSoCAttributeSubscriptionState()

    object SubscriptionEstablished : NextChargeTargetSoCAttributeSubscriptionState()
  }

  class ApproximateEVEfficiencyAttribute(val value: UShort?)

  sealed class ApproximateEVEfficiencyAttributeSubscriptionState {
    data class Success(val value: UShort?) : ApproximateEVEfficiencyAttributeSubscriptionState()

    data class Error(val exception: Exception) :
      ApproximateEVEfficiencyAttributeSubscriptionState()

    object SubscriptionEstablished : ApproximateEVEfficiencyAttributeSubscriptionState()
  }

  class StateOfChargeAttribute(val value: UByte?)

  sealed class StateOfChargeAttributeSubscriptionState {
    data class Success(val value: UByte?) : StateOfChargeAttributeSubscriptionState()

    data class Error(val exception: Exception) : StateOfChargeAttributeSubscriptionState()

    object SubscriptionEstablished : StateOfChargeAttributeSubscriptionState()
  }

  class BatteryCapacityAttribute(val value: Long?)

  sealed class BatteryCapacityAttributeSubscriptionState {
    data class Success(val value: Long?) : BatteryCapacityAttributeSubscriptionState()

    data class Error(val exception: Exception) : BatteryCapacityAttributeSubscriptionState()

    object SubscriptionEstablished : BatteryCapacityAttributeSubscriptionState()
  }

  class VehicleIDAttribute(val value: String?)

  sealed class VehicleIDAttributeSubscriptionState {
    data class Success(val value: String?) : VehicleIDAttributeSubscriptionState()

    data class Error(val exception: Exception) : VehicleIDAttributeSubscriptionState()

    object SubscriptionEstablished : VehicleIDAttributeSubscriptionState()
  }

  class SessionIDAttribute(val value: UInt?)

  sealed class SessionIDAttributeSubscriptionState {
    data class Success(val value: UInt?) : SessionIDAttributeSubscriptionState()

    data class Error(val exception: Exception) : SessionIDAttributeSubscriptionState()

    object SubscriptionEstablished : SessionIDAttributeSubscriptionState()
  }

  class SessionDurationAttribute(val value: UInt?)

  sealed class SessionDurationAttributeSubscriptionState {
    data class Success(val value: UInt?) : SessionDurationAttributeSubscriptionState()

    data class Error(val exception: Exception) : SessionDurationAttributeSubscriptionState()

    object SubscriptionEstablished : SessionDurationAttributeSubscriptionState()
  }

  class SessionEnergyChargedAttribute(val value: Long?)

  sealed class SessionEnergyChargedAttributeSubscriptionState {
    data class Success(val value: Long?) : SessionEnergyChargedAttributeSubscriptionState()

    data class Error(val exception: Exception) : SessionEnergyChargedAttributeSubscriptionState()

    object SubscriptionEstablished : SessionEnergyChargedAttributeSubscriptionState()
  }

  class SessionEnergyDischargedAttribute(val value: Long?)

  sealed class SessionEnergyDischargedAttributeSubscriptionState {
    data class Success(val value: Long?) : SessionEnergyDischargedAttributeSubscriptionState()

    data class Error(val exception: Exception) :
      SessionEnergyDischargedAttributeSubscriptionState()

    object SubscriptionEstablished : SessionEnergyDischargedAttributeSubscriptionState()
  }

  class GeneratedCommandListAttribute(val value: List<UInt>)

  sealed class GeneratedCommandListAttributeSubscriptionState {
    data class Success(val value: List<UInt>) : GeneratedCommandListAttributeSubscriptionState()

    data class Error(val exception: Exception) : GeneratedCommandListAttributeSubscriptionState()

    object SubscriptionEstablished : GeneratedCommandListAttributeSubscriptionState()
  }

  class AcceptedCommandListAttribute(val value: List<UInt>)

  sealed class AcceptedCommandListAttributeSubscriptionState {
    data class Success(val value: List<UInt>) : AcceptedCommandListAttributeSubscriptionState()

    data class Error(val exception: Exception) : AcceptedCommandListAttributeSubscriptionState()

    object SubscriptionEstablished : AcceptedCommandListAttributeSubscriptionState()
  }

  class EventListAttribute(val value: List<UInt>)

  sealed class EventListAttributeSubscriptionState {
    data class Success(val value: List<UInt>) : EventListAttributeSubscriptionState()

    data class Error(val exception: Exception) : EventListAttributeSubscriptionState()

    object SubscriptionEstablished : EventListAttributeSubscriptionState()
  }

  class AttributeListAttribute(val value: List<UInt>)

  sealed class AttributeListAttributeSubscriptionState {
    data class Success(val value: List<UInt>) : AttributeListAttributeSubscriptionState()

    data class Error(val exception: Exception) : AttributeListAttributeSubscriptionState()

    object SubscriptionEstablished : AttributeListAttributeSubscriptionState()
  }

  suspend fun disable(timedInvokeTimeout: Duration) {
    val commandId: UInt = 1u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout,
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")
  }

  suspend fun enableCharging(
    chargingEnabledUntil: UInt?,
    minimumChargeCurrent: Long,
    maximumChargeCurrent: Long,
    timedInvokeTimeout: Duration,
  ) {
    val commandId: UInt = 2u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_CHARGING_ENABLED_UNTIL_REQ: Int = 0
    chargingEnabledUntil?.let {
      tlvWriter.put(ContextSpecificTag(TAG_CHARGING_ENABLED_UNTIL_REQ), chargingEnabledUntil)
    }

    val TAG_MINIMUM_CHARGE_CURRENT_REQ: Int = 1
    tlvWriter.put(ContextSpecificTag(TAG_MINIMUM_CHARGE_CURRENT_REQ), minimumChargeCurrent)

    val TAG_MAXIMUM_CHARGE_CURRENT_REQ: Int = 2
    tlvWriter.put(ContextSpecificTag(TAG_MAXIMUM_CHARGE_CURRENT_REQ), maximumChargeCurrent)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout,
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")
  }

  suspend fun enableDischarging(
    dischargingEnabledUntil: UInt?,
    maximumDischargeCurrent: Long,
    timedInvokeTimeout: Duration,
  ) {
    val commandId: UInt = 3u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_DISCHARGING_ENABLED_UNTIL_REQ: Int = 0
    dischargingEnabledUntil?.let {
      tlvWriter.put(ContextSpecificTag(TAG_DISCHARGING_ENABLED_UNTIL_REQ), dischargingEnabledUntil)
    }

    val TAG_MAXIMUM_DISCHARGE_CURRENT_REQ: Int = 1
    tlvWriter.put(ContextSpecificTag(TAG_MAXIMUM_DISCHARGE_CURRENT_REQ), maximumDischargeCurrent)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout,
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")
  }

  suspend fun startDiagnostics(timedInvokeTimeout: Duration) {
    val commandId: UInt = 4u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout,
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")
  }

  suspend fun setTargets(
    chargingTargetSchedules: List<EnergyEvseClusterChargingTargetScheduleStruct>,
    timedInvokeTimeout: Duration,
  ) {
    val commandId: UInt = 5u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_CHARGING_TARGET_SCHEDULES_REQ: Int = 0
    tlvWriter.startArray(ContextSpecificTag(TAG_CHARGING_TARGET_SCHEDULES_REQ))
    for (item in chargingTargetSchedules.iterator()) {
      item.toTlv(AnonymousTag, tlvWriter)
    }
    tlvWriter.endArray()
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout,
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")
  }

  suspend fun getTargets(timedInvokeTimeout: Duration): GetTargetsResponse {
    val commandId: UInt = 6u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout,
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_CHARGING_TARGET_SCHEDULES: Int = 0
    var chargingTargetSchedules_decoded: List<EnergyEvseClusterChargingTargetScheduleStruct>? = null

    while (!tlvReader.isEndOfContainer()) {
      val tag = tlvReader.peekElement().tag

      if (tag == ContextSpecificTag(TAG_CHARGING_TARGET_SCHEDULES)) {
        chargingTargetSchedules_decoded =
          buildList<EnergyEvseClusterChargingTargetScheduleStruct> {
            tlvReader.enterArray(tag)
            while (!tlvReader.isEndOfContainer()) {
              add(EnergyEvseClusterChargingTargetScheduleStruct.fromTlv(AnonymousTag, tlvReader))
            }
            tlvReader.exitContainer()
          }
      } else {
        tlvReader.skipElement()
      }
    }

    if (chargingTargetSchedules_decoded == null) {
      throw IllegalStateException("chargingTargetSchedules not found in TLV")
    }

    tlvReader.exitContainer()

    return GetTargetsResponse(chargingTargetSchedules_decoded)
  }

  suspend fun clearTargets(timedInvokeTimeout: Duration) {
    val commandId: UInt = 7u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout,
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")
  }

  suspend fun readStateAttribute(): StateAttribute {
    val ATTRIBUTE_ID: UInt = 0u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "State attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UByte? =
      if (!tlvReader.isNull()) {
        tlvReader.getUByte(AnonymousTag)
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return StateAttribute(decodedValue)
  }

  suspend fun subscribeStateAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<StateAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 0u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            StateAttributeSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) { "State attribute not found in Node State update" }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UByte? =
            if (!tlvReader.isNull()) {
              tlvReader.getUByte(AnonymousTag)
            } else {
              tlvReader.getNull(AnonymousTag)
              null
            }

          decodedValue?.let { emit(StateAttributeSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(StateAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readSupplyStateAttribute(): UByte {
    val ATTRIBUTE_ID: UInt = 1u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Supplystate attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UByte = tlvReader.getUByte(AnonymousTag)

    return decodedValue
  }

  suspend fun subscribeSupplyStateAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<UByteSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 1u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            UByteSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) { "Supplystate attribute not found in Node State update" }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UByte = tlvReader.getUByte(AnonymousTag)

          emit(UByteSubscriptionState.Success(decodedValue))
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(UByteSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readFaultStateAttribute(): UByte {
    val ATTRIBUTE_ID: UInt = 2u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Faultstate attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UByte = tlvReader.getUByte(AnonymousTag)

    return decodedValue
  }

  suspend fun subscribeFaultStateAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<UByteSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 2u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            UByteSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) { "Faultstate attribute not found in Node State update" }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UByte = tlvReader.getUByte(AnonymousTag)

          emit(UByteSubscriptionState.Success(decodedValue))
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(UByteSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readChargingEnabledUntilAttribute(): ChargingEnabledUntilAttribute {
    val ATTRIBUTE_ID: UInt = 3u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Chargingenableduntil attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UInt? =
      if (!tlvReader.isNull()) {
        tlvReader.getUInt(AnonymousTag)
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return ChargingEnabledUntilAttribute(decodedValue)
  }

  suspend fun subscribeChargingEnabledUntilAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<ChargingEnabledUntilAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 3u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            ChargingEnabledUntilAttributeSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) {
            "Chargingenableduntil attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UInt? =
            if (!tlvReader.isNull()) {
              tlvReader.getUInt(AnonymousTag)
            } else {
              tlvReader.getNull(AnonymousTag)
              null
            }

          decodedValue?.let { emit(ChargingEnabledUntilAttributeSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(ChargingEnabledUntilAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readDischargingEnabledUntilAttribute(): DischargingEnabledUntilAttribute {
    val ATTRIBUTE_ID: UInt = 4u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Dischargingenableduntil attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UInt? =
      if (!tlvReader.isNull()) {
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getUInt(AnonymousTag)
        } else {
          null
        }
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return DischargingEnabledUntilAttribute(decodedValue)
  }

  suspend fun subscribeDischargingEnabledUntilAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<DischargingEnabledUntilAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 4u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            DischargingEnabledUntilAttributeSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) {
            "Dischargingenableduntil attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UInt? =
            if (!tlvReader.isNull()) {
              if (tlvReader.isNextTag(AnonymousTag)) {
                tlvReader.getUInt(AnonymousTag)
              } else {
                null
              }
            } else {
              tlvReader.getNull(AnonymousTag)
              null
            }

          decodedValue?.let { emit(DischargingEnabledUntilAttributeSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(DischargingEnabledUntilAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readCircuitCapacityAttribute(): Long {
    val ATTRIBUTE_ID: UInt = 5u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Circuitcapacity attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Long = tlvReader.getLong(AnonymousTag)

    return decodedValue
  }

  suspend fun subscribeCircuitCapacityAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<LongSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 5u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            LongSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) {
            "Circuitcapacity attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: Long = tlvReader.getLong(AnonymousTag)

          emit(LongSubscriptionState.Success(decodedValue))
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(LongSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readMinimumChargeCurrentAttribute(): Long {
    val ATTRIBUTE_ID: UInt = 6u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Minimumchargecurrent attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Long = tlvReader.getLong(AnonymousTag)

    return decodedValue
  }

  suspend fun subscribeMinimumChargeCurrentAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<LongSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 6u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            LongSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) {
            "Minimumchargecurrent attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: Long = tlvReader.getLong(AnonymousTag)

          emit(LongSubscriptionState.Success(decodedValue))
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(LongSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readMaximumChargeCurrentAttribute(): Long {
    val ATTRIBUTE_ID: UInt = 7u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Maximumchargecurrent attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Long = tlvReader.getLong(AnonymousTag)

    return decodedValue
  }

  suspend fun subscribeMaximumChargeCurrentAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<LongSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 7u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            LongSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) {
            "Maximumchargecurrent attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: Long = tlvReader.getLong(AnonymousTag)

          emit(LongSubscriptionState.Success(decodedValue))
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(LongSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readMaximumDischargeCurrentAttribute(): Long? {
    val ATTRIBUTE_ID: UInt = 8u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Maximumdischargecurrent attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Long? =
      if (tlvReader.isNextTag(AnonymousTag)) {
        tlvReader.getLong(AnonymousTag)
      } else {
        null
      }

    return decodedValue
  }

  suspend fun subscribeMaximumDischargeCurrentAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<LongSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 8u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            LongSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) {
            "Maximumdischargecurrent attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: Long? =
            if (tlvReader.isNextTag(AnonymousTag)) {
              tlvReader.getLong(AnonymousTag)
            } else {
              null
            }

          decodedValue?.let { emit(LongSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(LongSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readUserMaximumChargeCurrentAttribute(): Long? {
    val ATTRIBUTE_ID: UInt = 9u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Usermaximumchargecurrent attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Long? =
      if (tlvReader.isNextTag(AnonymousTag)) {
        tlvReader.getLong(AnonymousTag)
      } else {
        null
      }

    return decodedValue
  }

  suspend fun writeUserMaximumChargeCurrentAttribute(
    value: Long,
    timedWriteTimeout: Duration? = null,
  ) {
    val ATTRIBUTE_ID: UInt = 9u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded(),
            )
          ),
        timedRequest = timedWriteTimeout,
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun subscribeUserMaximumChargeCurrentAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<LongSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 9u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            LongSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) {
            "Usermaximumchargecurrent attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: Long? =
            if (tlvReader.isNextTag(AnonymousTag)) {
              tlvReader.getLong(AnonymousTag)
            } else {
              null
            }

          decodedValue?.let { emit(LongSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(LongSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readRandomizationDelayWindowAttribute(): UInt? {
    val ATTRIBUTE_ID: UInt = 10u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Randomizationdelaywindow attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UInt? =
      if (tlvReader.isNextTag(AnonymousTag)) {
        tlvReader.getUInt(AnonymousTag)
      } else {
        null
      }

    return decodedValue
  }

  suspend fun writeRandomizationDelayWindowAttribute(
    value: UInt,
    timedWriteTimeout: Duration? = null,
  ) {
    val ATTRIBUTE_ID: UInt = 10u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded(),
            )
          ),
        timedRequest = timedWriteTimeout,
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun subscribeRandomizationDelayWindowAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<UIntSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 10u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            UIntSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) {
            "Randomizationdelaywindow attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UInt? =
            if (tlvReader.isNextTag(AnonymousTag)) {
              tlvReader.getUInt(AnonymousTag)
            } else {
              null
            }

          decodedValue?.let { emit(UIntSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(UIntSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readNextChargeStartTimeAttribute(): NextChargeStartTimeAttribute {
    val ATTRIBUTE_ID: UInt = 35u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Nextchargestarttime attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UInt? =
      if (!tlvReader.isNull()) {
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getUInt(AnonymousTag)
        } else {
          null
        }
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return NextChargeStartTimeAttribute(decodedValue)
  }

  suspend fun subscribeNextChargeStartTimeAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<NextChargeStartTimeAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 35u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            NextChargeStartTimeAttributeSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) {
            "Nextchargestarttime attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UInt? =
            if (!tlvReader.isNull()) {
              if (tlvReader.isNextTag(AnonymousTag)) {
                tlvReader.getUInt(AnonymousTag)
              } else {
                null
              }
            } else {
              tlvReader.getNull(AnonymousTag)
              null
            }

          decodedValue?.let { emit(NextChargeStartTimeAttributeSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(NextChargeStartTimeAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readNextChargeTargetTimeAttribute(): NextChargeTargetTimeAttribute {
    val ATTRIBUTE_ID: UInt = 36u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Nextchargetargettime attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UInt? =
      if (!tlvReader.isNull()) {
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getUInt(AnonymousTag)
        } else {
          null
        }
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return NextChargeTargetTimeAttribute(decodedValue)
  }

  suspend fun subscribeNextChargeTargetTimeAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<NextChargeTargetTimeAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 36u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            NextChargeTargetTimeAttributeSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) {
            "Nextchargetargettime attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UInt? =
            if (!tlvReader.isNull()) {
              if (tlvReader.isNextTag(AnonymousTag)) {
                tlvReader.getUInt(AnonymousTag)
              } else {
                null
              }
            } else {
              tlvReader.getNull(AnonymousTag)
              null
            }

          decodedValue?.let { emit(NextChargeTargetTimeAttributeSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(NextChargeTargetTimeAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readNextChargeRequiredEnergyAttribute(): NextChargeRequiredEnergyAttribute {
    val ATTRIBUTE_ID: UInt = 37u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Nextchargerequiredenergy attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Long? =
      if (!tlvReader.isNull()) {
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getLong(AnonymousTag)
        } else {
          null
        }
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return NextChargeRequiredEnergyAttribute(decodedValue)
  }

  suspend fun subscribeNextChargeRequiredEnergyAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<NextChargeRequiredEnergyAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 37u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            NextChargeRequiredEnergyAttributeSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) {
            "Nextchargerequiredenergy attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: Long? =
            if (!tlvReader.isNull()) {
              if (tlvReader.isNextTag(AnonymousTag)) {
                tlvReader.getLong(AnonymousTag)
              } else {
                null
              }
            } else {
              tlvReader.getNull(AnonymousTag)
              null
            }

          decodedValue?.let { emit(NextChargeRequiredEnergyAttributeSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(NextChargeRequiredEnergyAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readNextChargeTargetSoCAttribute(): NextChargeTargetSoCAttribute {
    val ATTRIBUTE_ID: UInt = 38u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Nextchargetargetsoc attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UByte? =
      if (!tlvReader.isNull()) {
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getUByte(AnonymousTag)
        } else {
          null
        }
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return NextChargeTargetSoCAttribute(decodedValue)
  }

  suspend fun subscribeNextChargeTargetSoCAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<NextChargeTargetSoCAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 38u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            NextChargeTargetSoCAttributeSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) {
            "Nextchargetargetsoc attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UByte? =
            if (!tlvReader.isNull()) {
              if (tlvReader.isNextTag(AnonymousTag)) {
                tlvReader.getUByte(AnonymousTag)
              } else {
                null
              }
            } else {
              tlvReader.getNull(AnonymousTag)
              null
            }

          decodedValue?.let { emit(NextChargeTargetSoCAttributeSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(NextChargeTargetSoCAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readApproximateEVEfficiencyAttribute(): ApproximateEVEfficiencyAttribute {
    val ATTRIBUTE_ID: UInt = 39u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Approximateevefficiency attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UShort? =
      if (!tlvReader.isNull()) {
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getUShort(AnonymousTag)
        } else {
          null
        }
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return ApproximateEVEfficiencyAttribute(decodedValue)
  }

  suspend fun writeApproximateEVEfficiencyAttribute(
    value: UShort,
    timedWriteTimeout: Duration? = null,
  ) {
    val ATTRIBUTE_ID: UInt = 39u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded(),
            )
          ),
        timedRequest = timedWriteTimeout,
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun subscribeApproximateEVEfficiencyAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<ApproximateEVEfficiencyAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 39u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            ApproximateEVEfficiencyAttributeSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) {
            "Approximateevefficiency attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UShort? =
            if (!tlvReader.isNull()) {
              if (tlvReader.isNextTag(AnonymousTag)) {
                tlvReader.getUShort(AnonymousTag)
              } else {
                null
              }
            } else {
              tlvReader.getNull(AnonymousTag)
              null
            }

          decodedValue?.let { emit(ApproximateEVEfficiencyAttributeSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(ApproximateEVEfficiencyAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readStateOfChargeAttribute(): StateOfChargeAttribute {
    val ATTRIBUTE_ID: UInt = 48u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Stateofcharge attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UByte? =
      if (!tlvReader.isNull()) {
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getUByte(AnonymousTag)
        } else {
          null
        }
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return StateOfChargeAttribute(decodedValue)
  }

  suspend fun subscribeStateOfChargeAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<StateOfChargeAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 48u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            StateOfChargeAttributeSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) { "Stateofcharge attribute not found in Node State update" }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UByte? =
            if (!tlvReader.isNull()) {
              if (tlvReader.isNextTag(AnonymousTag)) {
                tlvReader.getUByte(AnonymousTag)
              } else {
                null
              }
            } else {
              tlvReader.getNull(AnonymousTag)
              null
            }

          decodedValue?.let { emit(StateOfChargeAttributeSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(StateOfChargeAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readBatteryCapacityAttribute(): BatteryCapacityAttribute {
    val ATTRIBUTE_ID: UInt = 49u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Batterycapacity attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Long? =
      if (!tlvReader.isNull()) {
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getLong(AnonymousTag)
        } else {
          null
        }
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return BatteryCapacityAttribute(decodedValue)
  }

  suspend fun subscribeBatteryCapacityAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<BatteryCapacityAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 49u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            BatteryCapacityAttributeSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) {
            "Batterycapacity attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: Long? =
            if (!tlvReader.isNull()) {
              if (tlvReader.isNextTag(AnonymousTag)) {
                tlvReader.getLong(AnonymousTag)
              } else {
                null
              }
            } else {
              tlvReader.getNull(AnonymousTag)
              null
            }

          decodedValue?.let { emit(BatteryCapacityAttributeSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(BatteryCapacityAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readVehicleIDAttribute(): VehicleIDAttribute {
    val ATTRIBUTE_ID: UInt = 50u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Vehicleid attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: String? =
      if (!tlvReader.isNull()) {
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getString(AnonymousTag)
        } else {
          null
        }
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return VehicleIDAttribute(decodedValue)
  }

  suspend fun subscribeVehicleIDAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<VehicleIDAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 50u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            VehicleIDAttributeSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) { "Vehicleid attribute not found in Node State update" }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: String? =
            if (!tlvReader.isNull()) {
              if (tlvReader.isNextTag(AnonymousTag)) {
                tlvReader.getString(AnonymousTag)
              } else {
                null
              }
            } else {
              tlvReader.getNull(AnonymousTag)
              null
            }

          decodedValue?.let { emit(VehicleIDAttributeSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(VehicleIDAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readSessionIDAttribute(): SessionIDAttribute {
    val ATTRIBUTE_ID: UInt = 64u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Sessionid attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UInt? =
      if (!tlvReader.isNull()) {
        tlvReader.getUInt(AnonymousTag)
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return SessionIDAttribute(decodedValue)
  }

  suspend fun subscribeSessionIDAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<SessionIDAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 64u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            SessionIDAttributeSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) { "Sessionid attribute not found in Node State update" }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UInt? =
            if (!tlvReader.isNull()) {
              tlvReader.getUInt(AnonymousTag)
            } else {
              tlvReader.getNull(AnonymousTag)
              null
            }

          decodedValue?.let { emit(SessionIDAttributeSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(SessionIDAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readSessionDurationAttribute(): SessionDurationAttribute {
    val ATTRIBUTE_ID: UInt = 65u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Sessionduration attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UInt? =
      if (!tlvReader.isNull()) {
        tlvReader.getUInt(AnonymousTag)
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return SessionDurationAttribute(decodedValue)
  }

  suspend fun subscribeSessionDurationAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<SessionDurationAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 65u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            SessionDurationAttributeSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) {
            "Sessionduration attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UInt? =
            if (!tlvReader.isNull()) {
              tlvReader.getUInt(AnonymousTag)
            } else {
              tlvReader.getNull(AnonymousTag)
              null
            }

          decodedValue?.let { emit(SessionDurationAttributeSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(SessionDurationAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readSessionEnergyChargedAttribute(): SessionEnergyChargedAttribute {
    val ATTRIBUTE_ID: UInt = 66u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Sessionenergycharged attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Long? =
      if (!tlvReader.isNull()) {
        tlvReader.getLong(AnonymousTag)
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return SessionEnergyChargedAttribute(decodedValue)
  }

  suspend fun subscribeSessionEnergyChargedAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<SessionEnergyChargedAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 66u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            SessionEnergyChargedAttributeSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) {
            "Sessionenergycharged attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: Long? =
            if (!tlvReader.isNull()) {
              tlvReader.getLong(AnonymousTag)
            } else {
              tlvReader.getNull(AnonymousTag)
              null
            }

          decodedValue?.let { emit(SessionEnergyChargedAttributeSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(SessionEnergyChargedAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readSessionEnergyDischargedAttribute(): SessionEnergyDischargedAttribute {
    val ATTRIBUTE_ID: UInt = 67u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Sessionenergydischarged attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Long? =
      if (!tlvReader.isNull()) {
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getLong(AnonymousTag)
        } else {
          null
        }
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return SessionEnergyDischargedAttribute(decodedValue)
  }

  suspend fun subscribeSessionEnergyDischargedAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<SessionEnergyDischargedAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 67u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            SessionEnergyDischargedAttributeSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) {
            "Sessionenergydischarged attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: Long? =
            if (!tlvReader.isNull()) {
              if (tlvReader.isNextTag(AnonymousTag)) {
                tlvReader.getLong(AnonymousTag)
              } else {
                null
              }
            } else {
              tlvReader.getNull(AnonymousTag)
              null
            }

          decodedValue?.let { emit(SessionEnergyDischargedAttributeSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(SessionEnergyDischargedAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readGeneratedCommandListAttribute(): GeneratedCommandListAttribute {
    val ATTRIBUTE_ID: UInt = 65528u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Generatedcommandlist attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: List<UInt> =
      buildList<UInt> {
        tlvReader.enterArray(AnonymousTag)
        while (!tlvReader.isEndOfContainer()) {
          add(tlvReader.getUInt(AnonymousTag))
        }
        tlvReader.exitContainer()
      }

    return GeneratedCommandListAttribute(decodedValue)
  }

  suspend fun subscribeGeneratedCommandListAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<GeneratedCommandListAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 65528u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            GeneratedCommandListAttributeSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) {
            "Generatedcommandlist attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: List<UInt> =
            buildList<UInt> {
              tlvReader.enterArray(AnonymousTag)
              while (!tlvReader.isEndOfContainer()) {
                add(tlvReader.getUInt(AnonymousTag))
              }
              tlvReader.exitContainer()
            }

          emit(GeneratedCommandListAttributeSubscriptionState.Success(decodedValue))
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(GeneratedCommandListAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readAcceptedCommandListAttribute(): AcceptedCommandListAttribute {
    val ATTRIBUTE_ID: UInt = 65529u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Acceptedcommandlist attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: List<UInt> =
      buildList<UInt> {
        tlvReader.enterArray(AnonymousTag)
        while (!tlvReader.isEndOfContainer()) {
          add(tlvReader.getUInt(AnonymousTag))
        }
        tlvReader.exitContainer()
      }

    return AcceptedCommandListAttribute(decodedValue)
  }

  suspend fun subscribeAcceptedCommandListAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<AcceptedCommandListAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 65529u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            AcceptedCommandListAttributeSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) {
            "Acceptedcommandlist attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: List<UInt> =
            buildList<UInt> {
              tlvReader.enterArray(AnonymousTag)
              while (!tlvReader.isEndOfContainer()) {
                add(tlvReader.getUInt(AnonymousTag))
              }
              tlvReader.exitContainer()
            }

          emit(AcceptedCommandListAttributeSubscriptionState.Success(decodedValue))
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(AcceptedCommandListAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readEventListAttribute(): EventListAttribute {
    val ATTRIBUTE_ID: UInt = 65530u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Eventlist attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: List<UInt> =
      buildList<UInt> {
        tlvReader.enterArray(AnonymousTag)
        while (!tlvReader.isEndOfContainer()) {
          add(tlvReader.getUInt(AnonymousTag))
        }
        tlvReader.exitContainer()
      }

    return EventListAttribute(decodedValue)
  }

  suspend fun subscribeEventListAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<EventListAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 65530u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            EventListAttributeSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) { "Eventlist attribute not found in Node State update" }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: List<UInt> =
            buildList<UInt> {
              tlvReader.enterArray(AnonymousTag)
              while (!tlvReader.isEndOfContainer()) {
                add(tlvReader.getUInt(AnonymousTag))
              }
              tlvReader.exitContainer()
            }

          emit(EventListAttributeSubscriptionState.Success(decodedValue))
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(EventListAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readAttributeListAttribute(): AttributeListAttribute {
    val ATTRIBUTE_ID: UInt = 65531u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Attributelist attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: List<UInt> =
      buildList<UInt> {
        tlvReader.enterArray(AnonymousTag)
        while (!tlvReader.isEndOfContainer()) {
          add(tlvReader.getUInt(AnonymousTag))
        }
        tlvReader.exitContainer()
      }

    return AttributeListAttribute(decodedValue)
  }

  suspend fun subscribeAttributeListAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<AttributeListAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 65531u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            AttributeListAttributeSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) { "Attributelist attribute not found in Node State update" }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: List<UInt> =
            buildList<UInt> {
              tlvReader.enterArray(AnonymousTag)
              while (!tlvReader.isEndOfContainer()) {
                add(tlvReader.getUInt(AnonymousTag))
              }
              tlvReader.exitContainer()
            }

          emit(AttributeListAttributeSubscriptionState.Success(decodedValue))
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(AttributeListAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readFeatureMapAttribute(): UInt {
    val ATTRIBUTE_ID: UInt = 65532u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Featuremap attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UInt = tlvReader.getUInt(AnonymousTag)

    return decodedValue
  }

  suspend fun subscribeFeatureMapAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<UIntSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 65532u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            UIntSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) { "Featuremap attribute not found in Node State update" }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UInt = tlvReader.getUInt(AnonymousTag)

          emit(UIntSubscriptionState.Success(decodedValue))
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(UIntSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readClusterRevisionAttribute(): UShort {
    val ATTRIBUTE_ID: UInt = 65533u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Clusterrevision attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UShort = tlvReader.getUShort(AnonymousTag)

    return decodedValue
  }

  suspend fun subscribeClusterRevisionAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<UShortSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 65533u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            UShortSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) {
            "Clusterrevision attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UShort = tlvReader.getUShort(AnonymousTag)

          emit(UShortSubscriptionState.Success(decodedValue))
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(UShortSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  companion object {
    private val logger = Logger.getLogger(EnergyEvseCluster::class.java.name)
    const val CLUSTER_ID: UInt = 153u
  }
}
