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
import matter.controller.BooleanSubscriptionState
import matter.controller.ByteArraySubscriptionState
import matter.controller.InvokeRequest
import matter.controller.InvokeResponse
import matter.controller.MatterController
import matter.controller.ReadData
import matter.controller.ReadRequest
import matter.controller.StringSubscriptionState
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

class DoorLockCluster(private val controller: MatterController, private val endpointId: UShort) {
  class GetWeekDayScheduleResponse(
    val weekDayIndex: UByte,
    val userIndex: UShort,
    val status: UByte,
    val daysMask: UByte?,
    val startHour: UByte?,
    val startMinute: UByte?,
    val endHour: UByte?,
    val endMinute: UByte?
  )

  class GetYearDayScheduleResponse(
    val yearDayIndex: UByte,
    val userIndex: UShort,
    val status: UByte,
    val localStartTime: UInt?,
    val localEndTime: UInt?
  )

  class GetHolidayScheduleResponse(
    val holidayIndex: UByte,
    val status: UByte,
    val localStartTime: UInt?,
    val localEndTime: UInt?,
    val operatingMode: UByte?
  )

  class GetUserResponse(
    val userIndex: UShort,
    val userName: String?,
    val userUniqueID: UInt?,
    val userStatus: UByte?,
    val userType: UByte?,
    val credentialRule: UByte?,
    val credentials: List<DoorLockClusterCredentialStruct>?,
    val creatorFabricIndex: UByte?,
    val lastModifiedFabricIndex: UByte?,
    val nextUserIndex: UShort?
  )

  class SetCredentialResponse(
    val status: UByte,
    val userIndex: UShort?,
    val nextCredentialIndex: UShort?
  )

  class GetCredentialStatusResponse(
    val credentialExists: Boolean,
    val userIndex: UShort?,
    val creatorFabricIndex: UByte?,
    val lastModifiedFabricIndex: UByte?,
    val nextCredentialIndex: UShort?
  )

  class LockStateAttribute(val value: UByte?)

  sealed class LockStateAttributeSubscriptionState {
    data class Success(val value: UByte?) : LockStateAttributeSubscriptionState()

    data class Error(val exception: Exception) : LockStateAttributeSubscriptionState()

    object SubscriptionEstablished : LockStateAttributeSubscriptionState()
  }

  class DoorStateAttribute(val value: UByte?)

  sealed class DoorStateAttributeSubscriptionState {
    data class Success(val value: UByte?) : DoorStateAttributeSubscriptionState()

    data class Error(val exception: Exception) : DoorStateAttributeSubscriptionState()

    object SubscriptionEstablished : DoorStateAttributeSubscriptionState()
  }

  class AliroReaderVerificationKeyAttribute(val value: ByteArray?)

  sealed class AliroReaderVerificationKeyAttributeSubscriptionState {
    data class Success(val value: ByteArray?) :
      AliroReaderVerificationKeyAttributeSubscriptionState()

    data class Error(val exception: Exception) :
      AliroReaderVerificationKeyAttributeSubscriptionState()

    object SubscriptionEstablished : AliroReaderVerificationKeyAttributeSubscriptionState()
  }

  class AliroReaderGroupIdentifierAttribute(val value: ByteArray?)

  sealed class AliroReaderGroupIdentifierAttributeSubscriptionState {
    data class Success(val value: ByteArray?) :
      AliroReaderGroupIdentifierAttributeSubscriptionState()

    data class Error(val exception: Exception) :
      AliroReaderGroupIdentifierAttributeSubscriptionState()

    object SubscriptionEstablished : AliroReaderGroupIdentifierAttributeSubscriptionState()
  }

  class AliroExpeditedTransactionSupportedProtocolVersionsAttribute(val value: List<ByteArray>?)

  sealed class AliroExpeditedTransactionSupportedProtocolVersionsAttributeSubscriptionState {
    data class Success(val value: List<ByteArray>?) :
      AliroExpeditedTransactionSupportedProtocolVersionsAttributeSubscriptionState()

    data class Error(val exception: Exception) :
      AliroExpeditedTransactionSupportedProtocolVersionsAttributeSubscriptionState()

    object SubscriptionEstablished :
      AliroExpeditedTransactionSupportedProtocolVersionsAttributeSubscriptionState()
  }

  class AliroGroupResolvingKeyAttribute(val value: ByteArray?)

  sealed class AliroGroupResolvingKeyAttributeSubscriptionState {
    data class Success(val value: ByteArray?) : AliroGroupResolvingKeyAttributeSubscriptionState()

    data class Error(val exception: Exception) : AliroGroupResolvingKeyAttributeSubscriptionState()

    object SubscriptionEstablished : AliroGroupResolvingKeyAttributeSubscriptionState()
  }

  class AliroSupportedBLEUWBProtocolVersionsAttribute(val value: List<ByteArray>?)

  sealed class AliroSupportedBLEUWBProtocolVersionsAttributeSubscriptionState {
    data class Success(val value: List<ByteArray>?) :
      AliroSupportedBLEUWBProtocolVersionsAttributeSubscriptionState()

    data class Error(val exception: Exception) :
      AliroSupportedBLEUWBProtocolVersionsAttributeSubscriptionState()

    object SubscriptionEstablished :
      AliroSupportedBLEUWBProtocolVersionsAttributeSubscriptionState()
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

  suspend fun lockDoor(PINCode: ByteArray?, timedInvokeTimeout: Duration) {
    val commandId: UInt = 0u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_P_I_N_CODE_REQ: Int = 0
    PINCode?.let { tlvWriter.put(ContextSpecificTag(TAG_P_I_N_CODE_REQ), PINCode) }
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")
  }

  suspend fun unlockDoor(PINCode: ByteArray?, timedInvokeTimeout: Duration) {
    val commandId: UInt = 1u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_P_I_N_CODE_REQ: Int = 0
    PINCode?.let { tlvWriter.put(ContextSpecificTag(TAG_P_I_N_CODE_REQ), PINCode) }
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")
  }

  suspend fun unlockWithTimeout(
    timeout: UShort,
    PINCode: ByteArray?,
    timedInvokeTimeout: Duration
  ) {
    val commandId: UInt = 3u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_TIMEOUT_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_TIMEOUT_REQ), timeout)

    val TAG_P_I_N_CODE_REQ: Int = 1
    PINCode?.let { tlvWriter.put(ContextSpecificTag(TAG_P_I_N_CODE_REQ), PINCode) }
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")
  }

  suspend fun setWeekDaySchedule(
    weekDayIndex: UByte,
    userIndex: UShort,
    daysMask: UByte,
    startHour: UByte,
    startMinute: UByte,
    endHour: UByte,
    endMinute: UByte,
    timedInvokeTimeout: Duration? = null
  ) {
    val commandId: UInt = 11u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_WEEK_DAY_INDEX_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_WEEK_DAY_INDEX_REQ), weekDayIndex)

    val TAG_USER_INDEX_REQ: Int = 1
    tlvWriter.put(ContextSpecificTag(TAG_USER_INDEX_REQ), userIndex)

    val TAG_DAYS_MASK_REQ: Int = 2
    tlvWriter.put(ContextSpecificTag(TAG_DAYS_MASK_REQ), daysMask)

    val TAG_START_HOUR_REQ: Int = 3
    tlvWriter.put(ContextSpecificTag(TAG_START_HOUR_REQ), startHour)

    val TAG_START_MINUTE_REQ: Int = 4
    tlvWriter.put(ContextSpecificTag(TAG_START_MINUTE_REQ), startMinute)

    val TAG_END_HOUR_REQ: Int = 5
    tlvWriter.put(ContextSpecificTag(TAG_END_HOUR_REQ), endHour)

    val TAG_END_MINUTE_REQ: Int = 6
    tlvWriter.put(ContextSpecificTag(TAG_END_MINUTE_REQ), endMinute)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")
  }

  suspend fun getWeekDaySchedule(
    weekDayIndex: UByte,
    userIndex: UShort,
    timedInvokeTimeout: Duration? = null
  ): GetWeekDayScheduleResponse {
    val commandId: UInt = 12u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_WEEK_DAY_INDEX_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_WEEK_DAY_INDEX_REQ), weekDayIndex)

    val TAG_USER_INDEX_REQ: Int = 1
    tlvWriter.put(ContextSpecificTag(TAG_USER_INDEX_REQ), userIndex)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_WEEK_DAY_INDEX: Int = 0
    var weekDayIndex_decoded: UByte? = null

    val TAG_USER_INDEX: Int = 1
    var userIndex_decoded: UShort? = null

    val TAG_STATUS: Int = 2
    var status_decoded: UByte? = null

    val TAG_DAYS_MASK: Int = 3
    var daysMask_decoded: UByte? = null

    val TAG_START_HOUR: Int = 4
    var startHour_decoded: UByte? = null

    val TAG_START_MINUTE: Int = 5
    var startMinute_decoded: UByte? = null

    val TAG_END_HOUR: Int = 6
    var endHour_decoded: UByte? = null

    val TAG_END_MINUTE: Int = 7
    var endMinute_decoded: UByte? = null

    while (!tlvReader.isEndOfContainer()) {
      val tag = tlvReader.peekElement().tag

      if (tag == ContextSpecificTag(TAG_WEEK_DAY_INDEX)) {
        weekDayIndex_decoded = tlvReader.getUByte(tag)
      }

      if (tag == ContextSpecificTag(TAG_USER_INDEX)) {
        userIndex_decoded = tlvReader.getUShort(tag)
      }

      if (tag == ContextSpecificTag(TAG_STATUS)) {
        status_decoded = tlvReader.getUByte(tag)
      }

      if (tag == ContextSpecificTag(TAG_DAYS_MASK)) {
        daysMask_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getUByte(tag)
            } else {
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_START_HOUR)) {
        startHour_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getUByte(tag)
            } else {
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_START_MINUTE)) {
        startMinute_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getUByte(tag)
            } else {
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_END_HOUR)) {
        endHour_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getUByte(tag)
            } else {
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_END_MINUTE)) {
        endMinute_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getUByte(tag)
            } else {
              null
            }
          }
      } else {
        tlvReader.skipElement()
      }
    }

    if (weekDayIndex_decoded == null) {
      throw IllegalStateException("weekDayIndex not found in TLV")
    }

    if (userIndex_decoded == null) {
      throw IllegalStateException("userIndex not found in TLV")
    }

    if (status_decoded == null) {
      throw IllegalStateException("status not found in TLV")
    }

    tlvReader.exitContainer()

    return GetWeekDayScheduleResponse(
      weekDayIndex_decoded,
      userIndex_decoded,
      status_decoded,
      daysMask_decoded,
      startHour_decoded,
      startMinute_decoded,
      endHour_decoded,
      endMinute_decoded
    )
  }

  suspend fun clearWeekDaySchedule(
    weekDayIndex: UByte,
    userIndex: UShort,
    timedInvokeTimeout: Duration? = null
  ) {
    val commandId: UInt = 13u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_WEEK_DAY_INDEX_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_WEEK_DAY_INDEX_REQ), weekDayIndex)

    val TAG_USER_INDEX_REQ: Int = 1
    tlvWriter.put(ContextSpecificTag(TAG_USER_INDEX_REQ), userIndex)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")
  }

  suspend fun setYearDaySchedule(
    yearDayIndex: UByte,
    userIndex: UShort,
    localStartTime: UInt,
    localEndTime: UInt,
    timedInvokeTimeout: Duration? = null
  ) {
    val commandId: UInt = 14u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_YEAR_DAY_INDEX_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_YEAR_DAY_INDEX_REQ), yearDayIndex)

    val TAG_USER_INDEX_REQ: Int = 1
    tlvWriter.put(ContextSpecificTag(TAG_USER_INDEX_REQ), userIndex)

    val TAG_LOCAL_START_TIME_REQ: Int = 2
    tlvWriter.put(ContextSpecificTag(TAG_LOCAL_START_TIME_REQ), localStartTime)

    val TAG_LOCAL_END_TIME_REQ: Int = 3
    tlvWriter.put(ContextSpecificTag(TAG_LOCAL_END_TIME_REQ), localEndTime)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")
  }

  suspend fun getYearDaySchedule(
    yearDayIndex: UByte,
    userIndex: UShort,
    timedInvokeTimeout: Duration? = null
  ): GetYearDayScheduleResponse {
    val commandId: UInt = 15u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_YEAR_DAY_INDEX_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_YEAR_DAY_INDEX_REQ), yearDayIndex)

    val TAG_USER_INDEX_REQ: Int = 1
    tlvWriter.put(ContextSpecificTag(TAG_USER_INDEX_REQ), userIndex)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_YEAR_DAY_INDEX: Int = 0
    var yearDayIndex_decoded: UByte? = null

    val TAG_USER_INDEX: Int = 1
    var userIndex_decoded: UShort? = null

    val TAG_STATUS: Int = 2
    var status_decoded: UByte? = null

    val TAG_LOCAL_START_TIME: Int = 3
    var localStartTime_decoded: UInt? = null

    val TAG_LOCAL_END_TIME: Int = 4
    var localEndTime_decoded: UInt? = null

    while (!tlvReader.isEndOfContainer()) {
      val tag = tlvReader.peekElement().tag

      if (tag == ContextSpecificTag(TAG_YEAR_DAY_INDEX)) {
        yearDayIndex_decoded = tlvReader.getUByte(tag)
      }

      if (tag == ContextSpecificTag(TAG_USER_INDEX)) {
        userIndex_decoded = tlvReader.getUShort(tag)
      }

      if (tag == ContextSpecificTag(TAG_STATUS)) {
        status_decoded = tlvReader.getUByte(tag)
      }

      if (tag == ContextSpecificTag(TAG_LOCAL_START_TIME)) {
        localStartTime_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getUInt(tag)
            } else {
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_LOCAL_END_TIME)) {
        localEndTime_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getUInt(tag)
            } else {
              null
            }
          }
      } else {
        tlvReader.skipElement()
      }
    }

    if (yearDayIndex_decoded == null) {
      throw IllegalStateException("yearDayIndex not found in TLV")
    }

    if (userIndex_decoded == null) {
      throw IllegalStateException("userIndex not found in TLV")
    }

    if (status_decoded == null) {
      throw IllegalStateException("status not found in TLV")
    }

    tlvReader.exitContainer()

    return GetYearDayScheduleResponse(
      yearDayIndex_decoded,
      userIndex_decoded,
      status_decoded,
      localStartTime_decoded,
      localEndTime_decoded
    )
  }

  suspend fun clearYearDaySchedule(
    yearDayIndex: UByte,
    userIndex: UShort,
    timedInvokeTimeout: Duration? = null
  ) {
    val commandId: UInt = 16u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_YEAR_DAY_INDEX_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_YEAR_DAY_INDEX_REQ), yearDayIndex)

    val TAG_USER_INDEX_REQ: Int = 1
    tlvWriter.put(ContextSpecificTag(TAG_USER_INDEX_REQ), userIndex)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")
  }

  suspend fun setHolidaySchedule(
    holidayIndex: UByte,
    localStartTime: UInt,
    localEndTime: UInt,
    operatingMode: UByte,
    timedInvokeTimeout: Duration? = null
  ) {
    val commandId: UInt = 17u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_HOLIDAY_INDEX_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_HOLIDAY_INDEX_REQ), holidayIndex)

    val TAG_LOCAL_START_TIME_REQ: Int = 1
    tlvWriter.put(ContextSpecificTag(TAG_LOCAL_START_TIME_REQ), localStartTime)

    val TAG_LOCAL_END_TIME_REQ: Int = 2
    tlvWriter.put(ContextSpecificTag(TAG_LOCAL_END_TIME_REQ), localEndTime)

    val TAG_OPERATING_MODE_REQ: Int = 3
    tlvWriter.put(ContextSpecificTag(TAG_OPERATING_MODE_REQ), operatingMode)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")
  }

  suspend fun getHolidaySchedule(
    holidayIndex: UByte,
    timedInvokeTimeout: Duration? = null
  ): GetHolidayScheduleResponse {
    val commandId: UInt = 18u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_HOLIDAY_INDEX_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_HOLIDAY_INDEX_REQ), holidayIndex)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_HOLIDAY_INDEX: Int = 0
    var holidayIndex_decoded: UByte? = null

    val TAG_STATUS: Int = 1
    var status_decoded: UByte? = null

    val TAG_LOCAL_START_TIME: Int = 2
    var localStartTime_decoded: UInt? = null

    val TAG_LOCAL_END_TIME: Int = 3
    var localEndTime_decoded: UInt? = null

    val TAG_OPERATING_MODE: Int = 4
    var operatingMode_decoded: UByte? = null

    while (!tlvReader.isEndOfContainer()) {
      val tag = tlvReader.peekElement().tag

      if (tag == ContextSpecificTag(TAG_HOLIDAY_INDEX)) {
        holidayIndex_decoded = tlvReader.getUByte(tag)
      }

      if (tag == ContextSpecificTag(TAG_STATUS)) {
        status_decoded = tlvReader.getUByte(tag)
      }

      if (tag == ContextSpecificTag(TAG_LOCAL_START_TIME)) {
        localStartTime_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getUInt(tag)
            } else {
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_LOCAL_END_TIME)) {
        localEndTime_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getUInt(tag)
            } else {
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_OPERATING_MODE)) {
        operatingMode_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getUByte(tag)
            } else {
              null
            }
          }
      } else {
        tlvReader.skipElement()
      }
    }

    if (holidayIndex_decoded == null) {
      throw IllegalStateException("holidayIndex not found in TLV")
    }

    if (status_decoded == null) {
      throw IllegalStateException("status not found in TLV")
    }

    tlvReader.exitContainer()

    return GetHolidayScheduleResponse(
      holidayIndex_decoded,
      status_decoded,
      localStartTime_decoded,
      localEndTime_decoded,
      operatingMode_decoded
    )
  }

  suspend fun clearHolidaySchedule(holidayIndex: UByte, timedInvokeTimeout: Duration? = null) {
    val commandId: UInt = 19u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_HOLIDAY_INDEX_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_HOLIDAY_INDEX_REQ), holidayIndex)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")
  }

  suspend fun setUser(
    operationType: UByte,
    userIndex: UShort,
    userName: String?,
    userUniqueID: UInt?,
    userStatus: UByte?,
    userType: UByte?,
    credentialRule: UByte?,
    timedInvokeTimeout: Duration
  ) {
    val commandId: UInt = 26u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_OPERATION_TYPE_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_OPERATION_TYPE_REQ), operationType)

    val TAG_USER_INDEX_REQ: Int = 1
    tlvWriter.put(ContextSpecificTag(TAG_USER_INDEX_REQ), userIndex)

    val TAG_USER_NAME_REQ: Int = 2
    userName?.let { tlvWriter.put(ContextSpecificTag(TAG_USER_NAME_REQ), userName) }

    val TAG_USER_UNIQUE_I_D_REQ: Int = 3
    userUniqueID?.let { tlvWriter.put(ContextSpecificTag(TAG_USER_UNIQUE_I_D_REQ), userUniqueID) }

    val TAG_USER_STATUS_REQ: Int = 4
    userStatus?.let { tlvWriter.put(ContextSpecificTag(TAG_USER_STATUS_REQ), userStatus) }

    val TAG_USER_TYPE_REQ: Int = 5
    userType?.let { tlvWriter.put(ContextSpecificTag(TAG_USER_TYPE_REQ), userType) }

    val TAG_CREDENTIAL_RULE_REQ: Int = 6
    credentialRule?.let {
      tlvWriter.put(ContextSpecificTag(TAG_CREDENTIAL_RULE_REQ), credentialRule)
    }
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")
  }

  suspend fun getUser(userIndex: UShort, timedInvokeTimeout: Duration? = null): GetUserResponse {
    val commandId: UInt = 27u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_USER_INDEX_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_USER_INDEX_REQ), userIndex)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_USER_INDEX: Int = 0
    var userIndex_decoded: UShort? = null

    val TAG_USER_NAME: Int = 1
    var userName_decoded: String? = null

    val TAG_USER_UNIQUE_I_D: Int = 2
    var userUniqueID_decoded: UInt? = null

    val TAG_USER_STATUS: Int = 3
    var userStatus_decoded: UByte? = null

    val TAG_USER_TYPE: Int = 4
    var userType_decoded: UByte? = null

    val TAG_CREDENTIAL_RULE: Int = 5
    var credentialRule_decoded: UByte? = null

    val TAG_CREDENTIALS: Int = 6
    var credentials_decoded: List<DoorLockClusterCredentialStruct>? = null

    val TAG_CREATOR_FABRIC_INDEX: Int = 7
    var creatorFabricIndex_decoded: UByte? = null

    val TAG_LAST_MODIFIED_FABRIC_INDEX: Int = 8
    var lastModifiedFabricIndex_decoded: UByte? = null

    val TAG_NEXT_USER_INDEX: Int = 9
    var nextUserIndex_decoded: UShort? = null

    while (!tlvReader.isEndOfContainer()) {
      val tag = tlvReader.peekElement().tag

      if (tag == ContextSpecificTag(TAG_USER_INDEX)) {
        userIndex_decoded = tlvReader.getUShort(tag)
      }

      if (tag == ContextSpecificTag(TAG_USER_NAME)) {
        userName_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (!tlvReader.isNull()) {
              tlvReader.getString(tag)
            } else {
              tlvReader.getNull(tag)
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_USER_UNIQUE_I_D)) {
        userUniqueID_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (!tlvReader.isNull()) {
              tlvReader.getUInt(tag)
            } else {
              tlvReader.getNull(tag)
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_USER_STATUS)) {
        userStatus_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (!tlvReader.isNull()) {
              tlvReader.getUByte(tag)
            } else {
              tlvReader.getNull(tag)
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_USER_TYPE)) {
        userType_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (!tlvReader.isNull()) {
              tlvReader.getUByte(tag)
            } else {
              tlvReader.getNull(tag)
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_CREDENTIAL_RULE)) {
        credentialRule_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (!tlvReader.isNull()) {
              tlvReader.getUByte(tag)
            } else {
              tlvReader.getNull(tag)
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_CREDENTIALS)) {
        credentials_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (!tlvReader.isNull()) {
              buildList<DoorLockClusterCredentialStruct> {
                tlvReader.enterArray(tag)
                while (!tlvReader.isEndOfContainer()) {
                  add(DoorLockClusterCredentialStruct.fromTlv(AnonymousTag, tlvReader))
                }
                tlvReader.exitContainer()
              }
            } else {
              tlvReader.getNull(tag)
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_CREATOR_FABRIC_INDEX)) {
        creatorFabricIndex_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (!tlvReader.isNull()) {
              tlvReader.getUByte(tag)
            } else {
              tlvReader.getNull(tag)
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_LAST_MODIFIED_FABRIC_INDEX)) {
        lastModifiedFabricIndex_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (!tlvReader.isNull()) {
              tlvReader.getUByte(tag)
            } else {
              tlvReader.getNull(tag)
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_NEXT_USER_INDEX)) {
        nextUserIndex_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (!tlvReader.isNull()) {
              tlvReader.getUShort(tag)
            } else {
              tlvReader.getNull(tag)
              null
            }
          }
      } else {
        tlvReader.skipElement()
      }
    }

    if (userIndex_decoded == null) {
      throw IllegalStateException("userIndex not found in TLV")
    }

    tlvReader.exitContainer()

    return GetUserResponse(
      userIndex_decoded,
      userName_decoded,
      userUniqueID_decoded,
      userStatus_decoded,
      userType_decoded,
      credentialRule_decoded,
      credentials_decoded,
      creatorFabricIndex_decoded,
      lastModifiedFabricIndex_decoded,
      nextUserIndex_decoded
    )
  }

  suspend fun clearUser(userIndex: UShort, timedInvokeTimeout: Duration) {
    val commandId: UInt = 29u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_USER_INDEX_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_USER_INDEX_REQ), userIndex)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")
  }

  suspend fun setCredential(
    operationType: UByte,
    credential: DoorLockClusterCredentialStruct,
    credentialData: ByteArray,
    userIndex: UShort?,
    userStatus: UByte?,
    userType: UByte?,
    timedInvokeTimeout: Duration
  ): SetCredentialResponse {
    val commandId: UInt = 34u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_OPERATION_TYPE_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_OPERATION_TYPE_REQ), operationType)

    val TAG_CREDENTIAL_REQ: Int = 1
    credential.toTlv(ContextSpecificTag(TAG_CREDENTIAL_REQ), tlvWriter)

    val TAG_CREDENTIAL_DATA_REQ: Int = 2
    tlvWriter.put(ContextSpecificTag(TAG_CREDENTIAL_DATA_REQ), credentialData)

    val TAG_USER_INDEX_REQ: Int = 3
    userIndex?.let { tlvWriter.put(ContextSpecificTag(TAG_USER_INDEX_REQ), userIndex) }

    val TAG_USER_STATUS_REQ: Int = 4
    userStatus?.let { tlvWriter.put(ContextSpecificTag(TAG_USER_STATUS_REQ), userStatus) }

    val TAG_USER_TYPE_REQ: Int = 5
    userType?.let { tlvWriter.put(ContextSpecificTag(TAG_USER_TYPE_REQ), userType) }
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_STATUS: Int = 0
    var status_decoded: UByte? = null

    val TAG_USER_INDEX: Int = 1
    var userIndex_decoded: UShort? = null

    val TAG_NEXT_CREDENTIAL_INDEX: Int = 2
    var nextCredentialIndex_decoded: UShort? = null

    while (!tlvReader.isEndOfContainer()) {
      val tag = tlvReader.peekElement().tag

      if (tag == ContextSpecificTag(TAG_STATUS)) {
        status_decoded = tlvReader.getUByte(tag)
      }

      if (tag == ContextSpecificTag(TAG_USER_INDEX)) {
        userIndex_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (!tlvReader.isNull()) {
              tlvReader.getUShort(tag)
            } else {
              tlvReader.getNull(tag)
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_NEXT_CREDENTIAL_INDEX)) {
        nextCredentialIndex_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (!tlvReader.isNull()) {
              tlvReader.getUShort(tag)
            } else {
              tlvReader.getNull(tag)
              null
            }
          }
      } else {
        tlvReader.skipElement()
      }
    }

    if (status_decoded == null) {
      throw IllegalStateException("status not found in TLV")
    }

    tlvReader.exitContainer()

    return SetCredentialResponse(status_decoded, userIndex_decoded, nextCredentialIndex_decoded)
  }

  suspend fun getCredentialStatus(
    credential: DoorLockClusterCredentialStruct,
    timedInvokeTimeout: Duration? = null
  ): GetCredentialStatusResponse {
    val commandId: UInt = 36u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_CREDENTIAL_REQ: Int = 0
    credential.toTlv(ContextSpecificTag(TAG_CREDENTIAL_REQ), tlvWriter)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_CREDENTIAL_EXISTS: Int = 0
    var credentialExists_decoded: Boolean? = null

    val TAG_USER_INDEX: Int = 1
    var userIndex_decoded: UShort? = null

    val TAG_CREATOR_FABRIC_INDEX: Int = 2
    var creatorFabricIndex_decoded: UByte? = null

    val TAG_LAST_MODIFIED_FABRIC_INDEX: Int = 3
    var lastModifiedFabricIndex_decoded: UByte? = null

    val TAG_NEXT_CREDENTIAL_INDEX: Int = 4
    var nextCredentialIndex_decoded: UShort? = null

    while (!tlvReader.isEndOfContainer()) {
      val tag = tlvReader.peekElement().tag

      if (tag == ContextSpecificTag(TAG_CREDENTIAL_EXISTS)) {
        credentialExists_decoded = tlvReader.getBoolean(tag)
      }

      if (tag == ContextSpecificTag(TAG_USER_INDEX)) {
        userIndex_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (!tlvReader.isNull()) {
              tlvReader.getUShort(tag)
            } else {
              tlvReader.getNull(tag)
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_CREATOR_FABRIC_INDEX)) {
        creatorFabricIndex_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (!tlvReader.isNull()) {
              tlvReader.getUByte(tag)
            } else {
              tlvReader.getNull(tag)
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_LAST_MODIFIED_FABRIC_INDEX)) {
        lastModifiedFabricIndex_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (!tlvReader.isNull()) {
              tlvReader.getUByte(tag)
            } else {
              tlvReader.getNull(tag)
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_NEXT_CREDENTIAL_INDEX)) {
        nextCredentialIndex_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (!tlvReader.isNull()) {
              tlvReader.getUShort(tag)
            } else {
              tlvReader.getNull(tag)
              null
            }
          }
      } else {
        tlvReader.skipElement()
      }
    }

    if (credentialExists_decoded == null) {
      throw IllegalStateException("credentialExists not found in TLV")
    }

    tlvReader.exitContainer()

    return GetCredentialStatusResponse(
      credentialExists_decoded,
      userIndex_decoded,
      creatorFabricIndex_decoded,
      lastModifiedFabricIndex_decoded,
      nextCredentialIndex_decoded
    )
  }

  suspend fun clearCredential(
    credential: DoorLockClusterCredentialStruct?,
    timedInvokeTimeout: Duration
  ) {
    val commandId: UInt = 38u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_CREDENTIAL_REQ: Int = 0
    credential?.let { credential.toTlv(ContextSpecificTag(TAG_CREDENTIAL_REQ), tlvWriter) }
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")
  }

  suspend fun unboltDoor(PINCode: ByteArray?, timedInvokeTimeout: Duration) {
    val commandId: UInt = 39u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_P_I_N_CODE_REQ: Int = 0
    PINCode?.let { tlvWriter.put(ContextSpecificTag(TAG_P_I_N_CODE_REQ), PINCode) }
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")
  }

  suspend fun setAliroReaderConfig(
    signingKey: ByteArray,
    verificationKey: ByteArray,
    groupIdentifier: ByteArray,
    groupResolvingKey: ByteArray?,
    timedInvokeTimeout: Duration
  ) {
    val commandId: UInt = 40u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_SIGNING_KEY_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_SIGNING_KEY_REQ), signingKey)

    val TAG_VERIFICATION_KEY_REQ: Int = 1
    tlvWriter.put(ContextSpecificTag(TAG_VERIFICATION_KEY_REQ), verificationKey)

    val TAG_GROUP_IDENTIFIER_REQ: Int = 2
    tlvWriter.put(ContextSpecificTag(TAG_GROUP_IDENTIFIER_REQ), groupIdentifier)

    val TAG_GROUP_RESOLVING_KEY_REQ: Int = 3
    groupResolvingKey?.let {
      tlvWriter.put(ContextSpecificTag(TAG_GROUP_RESOLVING_KEY_REQ), groupResolvingKey)
    }
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")
  }

  suspend fun clearAliroReaderConfig(timedInvokeTimeout: Duration) {
    val commandId: UInt = 41u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")
  }

  suspend fun readLockStateAttribute(): LockStateAttribute {
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

    requireNotNull(attributeData) { "Lockstate attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UByte? =
      if (!tlvReader.isNull()) {
        tlvReader.getUByte(AnonymousTag)
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return LockStateAttribute(decodedValue)
  }

  suspend fun subscribeLockStateAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<LockStateAttributeSubscriptionState> {
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
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            LockStateAttributeSubscriptionState.Error(
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

          requireNotNull(attributeData) { "Lockstate attribute not found in Node State update" }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UByte? =
            if (!tlvReader.isNull()) {
              tlvReader.getUByte(AnonymousTag)
            } else {
              tlvReader.getNull(AnonymousTag)
              null
            }

          decodedValue?.let { emit(LockStateAttributeSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(LockStateAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readLockTypeAttribute(): UByte {
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

    requireNotNull(attributeData) { "Locktype attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UByte = tlvReader.getUByte(AnonymousTag)

    return decodedValue
  }

  suspend fun subscribeLockTypeAttribute(
    minInterval: Int,
    maxInterval: Int
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
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
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

          requireNotNull(attributeData) { "Locktype attribute not found in Node State update" }

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

  suspend fun readActuatorEnabledAttribute(): Boolean {
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

    requireNotNull(attributeData) { "Actuatorenabled attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Boolean = tlvReader.getBoolean(AnonymousTag)

    return decodedValue
  }

  suspend fun subscribeActuatorEnabledAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<BooleanSubscriptionState> {
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
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            BooleanSubscriptionState.Error(
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
            "Actuatorenabled attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: Boolean = tlvReader.getBoolean(AnonymousTag)

          emit(BooleanSubscriptionState.Success(decodedValue))
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(BooleanSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readDoorStateAttribute(): DoorStateAttribute {
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

    requireNotNull(attributeData) { "Doorstate attribute not found in response" }

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

    return DoorStateAttribute(decodedValue)
  }

  suspend fun subscribeDoorStateAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<DoorStateAttributeSubscriptionState> {
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
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            DoorStateAttributeSubscriptionState.Error(
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

          requireNotNull(attributeData) { "Doorstate attribute not found in Node State update" }

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

          decodedValue?.let { emit(DoorStateAttributeSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(DoorStateAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readDoorOpenEventsAttribute(): UInt? {
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

    requireNotNull(attributeData) { "Dooropenevents attribute not found in response" }

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

  suspend fun writeDoorOpenEventsAttribute(value: UInt, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 4u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
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

  suspend fun subscribeDoorOpenEventsAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<UIntSubscriptionState> {
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
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
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
            "Dooropenevents attribute not found in Node State update"
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

  suspend fun readDoorClosedEventsAttribute(): UInt? {
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

    requireNotNull(attributeData) { "Doorclosedevents attribute not found in response" }

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

  suspend fun writeDoorClosedEventsAttribute(value: UInt, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 5u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
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

  suspend fun subscribeDoorClosedEventsAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<UIntSubscriptionState> {
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
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
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
            "Doorclosedevents attribute not found in Node State update"
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

  suspend fun readOpenPeriodAttribute(): UShort? {
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

    requireNotNull(attributeData) { "Openperiod attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UShort? =
      if (tlvReader.isNextTag(AnonymousTag)) {
        tlvReader.getUShort(AnonymousTag)
      } else {
        null
      }

    return decodedValue
  }

  suspend fun writeOpenPeriodAttribute(value: UShort, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 6u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
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

  suspend fun subscribeOpenPeriodAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<UShortSubscriptionState> {
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
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
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

          requireNotNull(attributeData) { "Openperiod attribute not found in Node State update" }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UShort? =
            if (tlvReader.isNextTag(AnonymousTag)) {
              tlvReader.getUShort(AnonymousTag)
            } else {
              null
            }

          decodedValue?.let { emit(UShortSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(UShortSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readNumberOfTotalUsersSupportedAttribute(): UShort? {
    val ATTRIBUTE_ID: UInt = 17u

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

    requireNotNull(attributeData) { "Numberoftotaluserssupported attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UShort? =
      if (tlvReader.isNextTag(AnonymousTag)) {
        tlvReader.getUShort(AnonymousTag)
      } else {
        null
      }

    return decodedValue
  }

  suspend fun subscribeNumberOfTotalUsersSupportedAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<UShortSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 17u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
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
            "Numberoftotaluserssupported attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UShort? =
            if (tlvReader.isNextTag(AnonymousTag)) {
              tlvReader.getUShort(AnonymousTag)
            } else {
              null
            }

          decodedValue?.let { emit(UShortSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(UShortSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readNumberOfPINUsersSupportedAttribute(): UShort? {
    val ATTRIBUTE_ID: UInt = 18u

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

    requireNotNull(attributeData) { "Numberofpinuserssupported attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UShort? =
      if (tlvReader.isNextTag(AnonymousTag)) {
        tlvReader.getUShort(AnonymousTag)
      } else {
        null
      }

    return decodedValue
  }

  suspend fun subscribeNumberOfPINUsersSupportedAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<UShortSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 18u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
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
            "Numberofpinuserssupported attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UShort? =
            if (tlvReader.isNextTag(AnonymousTag)) {
              tlvReader.getUShort(AnonymousTag)
            } else {
              null
            }

          decodedValue?.let { emit(UShortSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(UShortSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readNumberOfRFIDUsersSupportedAttribute(): UShort? {
    val ATTRIBUTE_ID: UInt = 19u

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

    requireNotNull(attributeData) { "Numberofrfiduserssupported attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UShort? =
      if (tlvReader.isNextTag(AnonymousTag)) {
        tlvReader.getUShort(AnonymousTag)
      } else {
        null
      }

    return decodedValue
  }

  suspend fun subscribeNumberOfRFIDUsersSupportedAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<UShortSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 19u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
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
            "Numberofrfiduserssupported attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UShort? =
            if (tlvReader.isNextTag(AnonymousTag)) {
              tlvReader.getUShort(AnonymousTag)
            } else {
              null
            }

          decodedValue?.let { emit(UShortSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(UShortSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readNumberOfWeekDaySchedulesSupportedPerUserAttribute(): UByte? {
    val ATTRIBUTE_ID: UInt = 20u

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

    requireNotNull(attributeData) {
      "Numberofweekdayschedulessupportedperuser attribute not found in response"
    }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UByte? =
      if (tlvReader.isNextTag(AnonymousTag)) {
        tlvReader.getUByte(AnonymousTag)
      } else {
        null
      }

    return decodedValue
  }

  suspend fun subscribeNumberOfWeekDaySchedulesSupportedPerUserAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<UByteSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 20u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
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

          requireNotNull(attributeData) {
            "Numberofweekdayschedulessupportedperuser attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UByte? =
            if (tlvReader.isNextTag(AnonymousTag)) {
              tlvReader.getUByte(AnonymousTag)
            } else {
              null
            }

          decodedValue?.let { emit(UByteSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(UByteSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readNumberOfYearDaySchedulesSupportedPerUserAttribute(): UByte? {
    val ATTRIBUTE_ID: UInt = 21u

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

    requireNotNull(attributeData) {
      "Numberofyeardayschedulessupportedperuser attribute not found in response"
    }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UByte? =
      if (tlvReader.isNextTag(AnonymousTag)) {
        tlvReader.getUByte(AnonymousTag)
      } else {
        null
      }

    return decodedValue
  }

  suspend fun subscribeNumberOfYearDaySchedulesSupportedPerUserAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<UByteSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 21u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
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

          requireNotNull(attributeData) {
            "Numberofyeardayschedulessupportedperuser attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UByte? =
            if (tlvReader.isNextTag(AnonymousTag)) {
              tlvReader.getUByte(AnonymousTag)
            } else {
              null
            }

          decodedValue?.let { emit(UByteSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(UByteSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readNumberOfHolidaySchedulesSupportedAttribute(): UByte? {
    val ATTRIBUTE_ID: UInt = 22u

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

    requireNotNull(attributeData) {
      "Numberofholidayschedulessupported attribute not found in response"
    }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UByte? =
      if (tlvReader.isNextTag(AnonymousTag)) {
        tlvReader.getUByte(AnonymousTag)
      } else {
        null
      }

    return decodedValue
  }

  suspend fun subscribeNumberOfHolidaySchedulesSupportedAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<UByteSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 22u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
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

          requireNotNull(attributeData) {
            "Numberofholidayschedulessupported attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UByte? =
            if (tlvReader.isNextTag(AnonymousTag)) {
              tlvReader.getUByte(AnonymousTag)
            } else {
              null
            }

          decodedValue?.let { emit(UByteSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(UByteSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readMaxPINCodeLengthAttribute(): UByte? {
    val ATTRIBUTE_ID: UInt = 23u

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

    requireNotNull(attributeData) { "Maxpincodelength attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UByte? =
      if (tlvReader.isNextTag(AnonymousTag)) {
        tlvReader.getUByte(AnonymousTag)
      } else {
        null
      }

    return decodedValue
  }

  suspend fun subscribeMaxPINCodeLengthAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<UByteSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 23u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
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

          requireNotNull(attributeData) {
            "Maxpincodelength attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UByte? =
            if (tlvReader.isNextTag(AnonymousTag)) {
              tlvReader.getUByte(AnonymousTag)
            } else {
              null
            }

          decodedValue?.let { emit(UByteSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(UByteSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readMinPINCodeLengthAttribute(): UByte? {
    val ATTRIBUTE_ID: UInt = 24u

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

    requireNotNull(attributeData) { "Minpincodelength attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UByte? =
      if (tlvReader.isNextTag(AnonymousTag)) {
        tlvReader.getUByte(AnonymousTag)
      } else {
        null
      }

    return decodedValue
  }

  suspend fun subscribeMinPINCodeLengthAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<UByteSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 24u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
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

          requireNotNull(attributeData) {
            "Minpincodelength attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UByte? =
            if (tlvReader.isNextTag(AnonymousTag)) {
              tlvReader.getUByte(AnonymousTag)
            } else {
              null
            }

          decodedValue?.let { emit(UByteSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(UByteSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readMaxRFIDCodeLengthAttribute(): UByte? {
    val ATTRIBUTE_ID: UInt = 25u

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

    requireNotNull(attributeData) { "Maxrfidcodelength attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UByte? =
      if (tlvReader.isNextTag(AnonymousTag)) {
        tlvReader.getUByte(AnonymousTag)
      } else {
        null
      }

    return decodedValue
  }

  suspend fun subscribeMaxRFIDCodeLengthAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<UByteSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 25u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
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

          requireNotNull(attributeData) {
            "Maxrfidcodelength attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UByte? =
            if (tlvReader.isNextTag(AnonymousTag)) {
              tlvReader.getUByte(AnonymousTag)
            } else {
              null
            }

          decodedValue?.let { emit(UByteSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(UByteSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readMinRFIDCodeLengthAttribute(): UByte? {
    val ATTRIBUTE_ID: UInt = 26u

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

    requireNotNull(attributeData) { "Minrfidcodelength attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UByte? =
      if (tlvReader.isNextTag(AnonymousTag)) {
        tlvReader.getUByte(AnonymousTag)
      } else {
        null
      }

    return decodedValue
  }

  suspend fun subscribeMinRFIDCodeLengthAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<UByteSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 26u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
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

          requireNotNull(attributeData) {
            "Minrfidcodelength attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UByte? =
            if (tlvReader.isNextTag(AnonymousTag)) {
              tlvReader.getUByte(AnonymousTag)
            } else {
              null
            }

          decodedValue?.let { emit(UByteSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(UByteSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readCredentialRulesSupportAttribute(): UByte? {
    val ATTRIBUTE_ID: UInt = 27u

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

    requireNotNull(attributeData) { "Credentialrulessupport attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UByte? =
      if (tlvReader.isNextTag(AnonymousTag)) {
        tlvReader.getUByte(AnonymousTag)
      } else {
        null
      }

    return decodedValue
  }

  suspend fun subscribeCredentialRulesSupportAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<UByteSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 27u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
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

          requireNotNull(attributeData) {
            "Credentialrulessupport attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UByte? =
            if (tlvReader.isNextTag(AnonymousTag)) {
              tlvReader.getUByte(AnonymousTag)
            } else {
              null
            }

          decodedValue?.let { emit(UByteSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(UByteSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readNumberOfCredentialsSupportedPerUserAttribute(): UByte? {
    val ATTRIBUTE_ID: UInt = 28u

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

    requireNotNull(attributeData) {
      "Numberofcredentialssupportedperuser attribute not found in response"
    }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UByte? =
      if (tlvReader.isNextTag(AnonymousTag)) {
        tlvReader.getUByte(AnonymousTag)
      } else {
        null
      }

    return decodedValue
  }

  suspend fun subscribeNumberOfCredentialsSupportedPerUserAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<UByteSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 28u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
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

          requireNotNull(attributeData) {
            "Numberofcredentialssupportedperuser attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UByte? =
            if (tlvReader.isNextTag(AnonymousTag)) {
              tlvReader.getUByte(AnonymousTag)
            } else {
              null
            }

          decodedValue?.let { emit(UByteSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(UByteSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readLanguageAttribute(): String? {
    val ATTRIBUTE_ID: UInt = 33u

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

    requireNotNull(attributeData) { "Language attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: String? =
      if (tlvReader.isNextTag(AnonymousTag)) {
        tlvReader.getString(AnonymousTag)
      } else {
        null
      }

    return decodedValue
  }

  suspend fun writeLanguageAttribute(value: String, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 33u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
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

  suspend fun subscribeLanguageAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<StringSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 33u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            StringSubscriptionState.Error(
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

          requireNotNull(attributeData) { "Language attribute not found in Node State update" }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: String? =
            if (tlvReader.isNextTag(AnonymousTag)) {
              tlvReader.getString(AnonymousTag)
            } else {
              null
            }

          decodedValue?.let { emit(StringSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(StringSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readLEDSettingsAttribute(): UByte? {
    val ATTRIBUTE_ID: UInt = 34u

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

    requireNotNull(attributeData) { "Ledsettings attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UByte? =
      if (tlvReader.isNextTag(AnonymousTag)) {
        tlvReader.getUByte(AnonymousTag)
      } else {
        null
      }

    return decodedValue
  }

  suspend fun writeLEDSettingsAttribute(value: UByte, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 34u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
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

  suspend fun subscribeLEDSettingsAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<UByteSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 34u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
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

          requireNotNull(attributeData) { "Ledsettings attribute not found in Node State update" }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UByte? =
            if (tlvReader.isNextTag(AnonymousTag)) {
              tlvReader.getUByte(AnonymousTag)
            } else {
              null
            }

          decodedValue?.let { emit(UByteSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(UByteSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readAutoRelockTimeAttribute(): UInt {
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

    requireNotNull(attributeData) { "Autorelocktime attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UInt = tlvReader.getUInt(AnonymousTag)

    return decodedValue
  }

  suspend fun writeAutoRelockTimeAttribute(value: UInt, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 35u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
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

  suspend fun subscribeAutoRelockTimeAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<UIntSubscriptionState> {
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
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
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
            "Autorelocktime attribute not found in Node State update"
          }

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

  suspend fun readSoundVolumeAttribute(): UByte? {
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

    requireNotNull(attributeData) { "Soundvolume attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UByte? =
      if (tlvReader.isNextTag(AnonymousTag)) {
        tlvReader.getUByte(AnonymousTag)
      } else {
        null
      }

    return decodedValue
  }

  suspend fun writeSoundVolumeAttribute(value: UByte, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 36u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
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

  suspend fun subscribeSoundVolumeAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<UByteSubscriptionState> {
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
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
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

          requireNotNull(attributeData) { "Soundvolume attribute not found in Node State update" }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UByte? =
            if (tlvReader.isNextTag(AnonymousTag)) {
              tlvReader.getUByte(AnonymousTag)
            } else {
              null
            }

          decodedValue?.let { emit(UByteSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(UByteSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readOperatingModeAttribute(): UByte {
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

    requireNotNull(attributeData) { "Operatingmode attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UByte = tlvReader.getUByte(AnonymousTag)

    return decodedValue
  }

  suspend fun writeOperatingModeAttribute(value: UByte, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 37u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
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

  suspend fun subscribeOperatingModeAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<UByteSubscriptionState> {
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
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
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

          requireNotNull(attributeData) { "Operatingmode attribute not found in Node State update" }

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

  suspend fun readSupportedOperatingModesAttribute(): UShort {
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

    requireNotNull(attributeData) { "Supportedoperatingmodes attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UShort = tlvReader.getUShort(AnonymousTag)

    return decodedValue
  }

  suspend fun subscribeSupportedOperatingModesAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<UShortSubscriptionState> {
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
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
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
            "Supportedoperatingmodes attribute not found in Node State update"
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

  suspend fun readDefaultConfigurationRegisterAttribute(): UShort? {
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

    requireNotNull(attributeData) { "Defaultconfigurationregister attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UShort? =
      if (tlvReader.isNextTag(AnonymousTag)) {
        tlvReader.getUShort(AnonymousTag)
      } else {
        null
      }

    return decodedValue
  }

  suspend fun subscribeDefaultConfigurationRegisterAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<UShortSubscriptionState> {
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
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
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
            "Defaultconfigurationregister attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UShort? =
            if (tlvReader.isNextTag(AnonymousTag)) {
              tlvReader.getUShort(AnonymousTag)
            } else {
              null
            }

          decodedValue?.let { emit(UShortSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(UShortSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readEnableLocalProgrammingAttribute(): Boolean? {
    val ATTRIBUTE_ID: UInt = 40u

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

    requireNotNull(attributeData) { "Enablelocalprogramming attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Boolean? =
      if (tlvReader.isNextTag(AnonymousTag)) {
        tlvReader.getBoolean(AnonymousTag)
      } else {
        null
      }

    return decodedValue
  }

  suspend fun writeEnableLocalProgrammingAttribute(
    value: Boolean,
    timedWriteTimeout: Duration? = null
  ) {
    val ATTRIBUTE_ID: UInt = 40u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
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

  suspend fun subscribeEnableLocalProgrammingAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<BooleanSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 40u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            BooleanSubscriptionState.Error(
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
            "Enablelocalprogramming attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: Boolean? =
            if (tlvReader.isNextTag(AnonymousTag)) {
              tlvReader.getBoolean(AnonymousTag)
            } else {
              null
            }

          decodedValue?.let { emit(BooleanSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(BooleanSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readEnableOneTouchLockingAttribute(): Boolean? {
    val ATTRIBUTE_ID: UInt = 41u

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

    requireNotNull(attributeData) { "Enableonetouchlocking attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Boolean? =
      if (tlvReader.isNextTag(AnonymousTag)) {
        tlvReader.getBoolean(AnonymousTag)
      } else {
        null
      }

    return decodedValue
  }

  suspend fun writeEnableOneTouchLockingAttribute(
    value: Boolean,
    timedWriteTimeout: Duration? = null
  ) {
    val ATTRIBUTE_ID: UInt = 41u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
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

  suspend fun subscribeEnableOneTouchLockingAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<BooleanSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 41u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            BooleanSubscriptionState.Error(
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
            "Enableonetouchlocking attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: Boolean? =
            if (tlvReader.isNextTag(AnonymousTag)) {
              tlvReader.getBoolean(AnonymousTag)
            } else {
              null
            }

          decodedValue?.let { emit(BooleanSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(BooleanSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readEnableInsideStatusLEDAttribute(): Boolean? {
    val ATTRIBUTE_ID: UInt = 42u

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

    requireNotNull(attributeData) { "Enableinsidestatusled attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Boolean? =
      if (tlvReader.isNextTag(AnonymousTag)) {
        tlvReader.getBoolean(AnonymousTag)
      } else {
        null
      }

    return decodedValue
  }

  suspend fun writeEnableInsideStatusLEDAttribute(
    value: Boolean,
    timedWriteTimeout: Duration? = null
  ) {
    val ATTRIBUTE_ID: UInt = 42u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
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

  suspend fun subscribeEnableInsideStatusLEDAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<BooleanSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 42u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            BooleanSubscriptionState.Error(
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
            "Enableinsidestatusled attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: Boolean? =
            if (tlvReader.isNextTag(AnonymousTag)) {
              tlvReader.getBoolean(AnonymousTag)
            } else {
              null
            }

          decodedValue?.let { emit(BooleanSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(BooleanSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readEnablePrivacyModeButtonAttribute(): Boolean? {
    val ATTRIBUTE_ID: UInt = 43u

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

    requireNotNull(attributeData) { "Enableprivacymodebutton attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Boolean? =
      if (tlvReader.isNextTag(AnonymousTag)) {
        tlvReader.getBoolean(AnonymousTag)
      } else {
        null
      }

    return decodedValue
  }

  suspend fun writeEnablePrivacyModeButtonAttribute(
    value: Boolean,
    timedWriteTimeout: Duration? = null
  ) {
    val ATTRIBUTE_ID: UInt = 43u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
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

  suspend fun subscribeEnablePrivacyModeButtonAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<BooleanSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 43u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            BooleanSubscriptionState.Error(
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
            "Enableprivacymodebutton attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: Boolean? =
            if (tlvReader.isNextTag(AnonymousTag)) {
              tlvReader.getBoolean(AnonymousTag)
            } else {
              null
            }

          decodedValue?.let { emit(BooleanSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(BooleanSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readLocalProgrammingFeaturesAttribute(): UByte? {
    val ATTRIBUTE_ID: UInt = 44u

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

    requireNotNull(attributeData) { "Localprogrammingfeatures attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UByte? =
      if (tlvReader.isNextTag(AnonymousTag)) {
        tlvReader.getUByte(AnonymousTag)
      } else {
        null
      }

    return decodedValue
  }

  suspend fun writeLocalProgrammingFeaturesAttribute(
    value: UByte,
    timedWriteTimeout: Duration? = null
  ) {
    val ATTRIBUTE_ID: UInt = 44u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
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

  suspend fun subscribeLocalProgrammingFeaturesAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<UByteSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 44u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
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

          requireNotNull(attributeData) {
            "Localprogrammingfeatures attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UByte? =
            if (tlvReader.isNextTag(AnonymousTag)) {
              tlvReader.getUByte(AnonymousTag)
            } else {
              null
            }

          decodedValue?.let { emit(UByteSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(UByteSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readWrongCodeEntryLimitAttribute(): UByte? {
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

    requireNotNull(attributeData) { "Wrongcodeentrylimit attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UByte? =
      if (tlvReader.isNextTag(AnonymousTag)) {
        tlvReader.getUByte(AnonymousTag)
      } else {
        null
      }

    return decodedValue
  }

  suspend fun writeWrongCodeEntryLimitAttribute(value: UByte, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 48u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
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

  suspend fun subscribeWrongCodeEntryLimitAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<UByteSubscriptionState> {
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
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
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

          requireNotNull(attributeData) {
            "Wrongcodeentrylimit attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UByte? =
            if (tlvReader.isNextTag(AnonymousTag)) {
              tlvReader.getUByte(AnonymousTag)
            } else {
              null
            }

          decodedValue?.let { emit(UByteSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(UByteSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readUserCodeTemporaryDisableTimeAttribute(): UByte? {
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

    requireNotNull(attributeData) { "Usercodetemporarydisabletime attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UByte? =
      if (tlvReader.isNextTag(AnonymousTag)) {
        tlvReader.getUByte(AnonymousTag)
      } else {
        null
      }

    return decodedValue
  }

  suspend fun writeUserCodeTemporaryDisableTimeAttribute(
    value: UByte,
    timedWriteTimeout: Duration? = null
  ) {
    val ATTRIBUTE_ID: UInt = 49u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
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

  suspend fun subscribeUserCodeTemporaryDisableTimeAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<UByteSubscriptionState> {
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
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
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

          requireNotNull(attributeData) {
            "Usercodetemporarydisabletime attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UByte? =
            if (tlvReader.isNextTag(AnonymousTag)) {
              tlvReader.getUByte(AnonymousTag)
            } else {
              null
            }

          decodedValue?.let { emit(UByteSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(UByteSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readSendPINOverTheAirAttribute(): Boolean? {
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

    requireNotNull(attributeData) { "Sendpinovertheair attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Boolean? =
      if (tlvReader.isNextTag(AnonymousTag)) {
        tlvReader.getBoolean(AnonymousTag)
      } else {
        null
      }

    return decodedValue
  }

  suspend fun writeSendPINOverTheAirAttribute(value: Boolean, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 50u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
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

  suspend fun subscribeSendPINOverTheAirAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<BooleanSubscriptionState> {
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
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            BooleanSubscriptionState.Error(
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
            "Sendpinovertheair attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: Boolean? =
            if (tlvReader.isNextTag(AnonymousTag)) {
              tlvReader.getBoolean(AnonymousTag)
            } else {
              null
            }

          decodedValue?.let { emit(BooleanSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(BooleanSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readRequirePINforRemoteOperationAttribute(): Boolean? {
    val ATTRIBUTE_ID: UInt = 51u

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

    requireNotNull(attributeData) { "Requirepinforremoteoperation attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Boolean? =
      if (tlvReader.isNextTag(AnonymousTag)) {
        tlvReader.getBoolean(AnonymousTag)
      } else {
        null
      }

    return decodedValue
  }

  suspend fun writeRequirePINforRemoteOperationAttribute(
    value: Boolean,
    timedWriteTimeout: Duration? = null
  ) {
    val ATTRIBUTE_ID: UInt = 51u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
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

  suspend fun subscribeRequirePINforRemoteOperationAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<BooleanSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 51u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            BooleanSubscriptionState.Error(
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
            "Requirepinforremoteoperation attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: Boolean? =
            if (tlvReader.isNextTag(AnonymousTag)) {
              tlvReader.getBoolean(AnonymousTag)
            } else {
              null
            }

          decodedValue?.let { emit(BooleanSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(BooleanSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readExpiringUserTimeoutAttribute(): UShort? {
    val ATTRIBUTE_ID: UInt = 53u

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

    requireNotNull(attributeData) { "Expiringusertimeout attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UShort? =
      if (tlvReader.isNextTag(AnonymousTag)) {
        tlvReader.getUShort(AnonymousTag)
      } else {
        null
      }

    return decodedValue
  }

  suspend fun writeExpiringUserTimeoutAttribute(
    value: UShort,
    timedWriteTimeout: Duration? = null
  ) {
    val ATTRIBUTE_ID: UInt = 53u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
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

  suspend fun subscribeExpiringUserTimeoutAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<UShortSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 53u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
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
            "Expiringusertimeout attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UShort? =
            if (tlvReader.isNextTag(AnonymousTag)) {
              tlvReader.getUShort(AnonymousTag)
            } else {
              null
            }

          decodedValue?.let { emit(UShortSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(UShortSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readAliroReaderVerificationKeyAttribute(): AliroReaderVerificationKeyAttribute {
    val ATTRIBUTE_ID: UInt = 128u

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

    requireNotNull(attributeData) { "Aliroreaderverificationkey attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: ByteArray? =
      if (!tlvReader.isNull()) {
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getByteArray(AnonymousTag)
        } else {
          null
        }
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return AliroReaderVerificationKeyAttribute(decodedValue)
  }

  suspend fun subscribeAliroReaderVerificationKeyAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<AliroReaderVerificationKeyAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 128u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            AliroReaderVerificationKeyAttributeSubscriptionState.Error(
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
            "Aliroreaderverificationkey attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: ByteArray? =
            if (!tlvReader.isNull()) {
              if (tlvReader.isNextTag(AnonymousTag)) {
                tlvReader.getByteArray(AnonymousTag)
              } else {
                null
              }
            } else {
              tlvReader.getNull(AnonymousTag)
              null
            }

          decodedValue?.let {
            emit(AliroReaderVerificationKeyAttributeSubscriptionState.Success(it))
          }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(AliroReaderVerificationKeyAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readAliroReaderGroupIdentifierAttribute(): AliroReaderGroupIdentifierAttribute {
    val ATTRIBUTE_ID: UInt = 129u

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

    requireNotNull(attributeData) { "Aliroreadergroupidentifier attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: ByteArray? =
      if (!tlvReader.isNull()) {
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getByteArray(AnonymousTag)
        } else {
          null
        }
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return AliroReaderGroupIdentifierAttribute(decodedValue)
  }

  suspend fun subscribeAliroReaderGroupIdentifierAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<AliroReaderGroupIdentifierAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 129u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            AliroReaderGroupIdentifierAttributeSubscriptionState.Error(
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
            "Aliroreadergroupidentifier attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: ByteArray? =
            if (!tlvReader.isNull()) {
              if (tlvReader.isNextTag(AnonymousTag)) {
                tlvReader.getByteArray(AnonymousTag)
              } else {
                null
              }
            } else {
              tlvReader.getNull(AnonymousTag)
              null
            }

          decodedValue?.let {
            emit(AliroReaderGroupIdentifierAttributeSubscriptionState.Success(it))
          }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(AliroReaderGroupIdentifierAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readAliroReaderGroupSubIdentifierAttribute(): ByteArray? {
    val ATTRIBUTE_ID: UInt = 130u

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

    requireNotNull(attributeData) {
      "Aliroreadergroupsubidentifier attribute not found in response"
    }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: ByteArray? =
      if (tlvReader.isNextTag(AnonymousTag)) {
        tlvReader.getByteArray(AnonymousTag)
      } else {
        null
      }

    return decodedValue
  }

  suspend fun subscribeAliroReaderGroupSubIdentifierAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<ByteArraySubscriptionState> {
    val ATTRIBUTE_ID: UInt = 130u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            ByteArraySubscriptionState.Error(
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
            "Aliroreadergroupsubidentifier attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: ByteArray? =
            if (tlvReader.isNextTag(AnonymousTag)) {
              tlvReader.getByteArray(AnonymousTag)
            } else {
              null
            }

          decodedValue?.let { emit(ByteArraySubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(ByteArraySubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readAliroExpeditedTransactionSupportedProtocolVersionsAttribute():
    AliroExpeditedTransactionSupportedProtocolVersionsAttribute {
    val ATTRIBUTE_ID: UInt = 131u

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

    requireNotNull(attributeData) {
      "Aliroexpeditedtransactionsupportedprotocolversions attribute not found in response"
    }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: List<ByteArray>? =
      if (tlvReader.isNextTag(AnonymousTag)) {
        buildList<ByteArray> {
          tlvReader.enterArray(AnonymousTag)
          while (!tlvReader.isEndOfContainer()) {
            add(tlvReader.getByteArray(AnonymousTag))
          }
          tlvReader.exitContainer()
        }
      } else {
        null
      }

    return AliroExpeditedTransactionSupportedProtocolVersionsAttribute(decodedValue)
  }

  suspend fun subscribeAliroExpeditedTransactionSupportedProtocolVersionsAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<AliroExpeditedTransactionSupportedProtocolVersionsAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 131u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            AliroExpeditedTransactionSupportedProtocolVersionsAttributeSubscriptionState.Error(
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
            "Aliroexpeditedtransactionsupportedprotocolversions attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: List<ByteArray>? =
            if (tlvReader.isNextTag(AnonymousTag)) {
              buildList<ByteArray> {
                tlvReader.enterArray(AnonymousTag)
                while (!tlvReader.isEndOfContainer()) {
                  add(tlvReader.getByteArray(AnonymousTag))
                }
                tlvReader.exitContainer()
              }
            } else {
              null
            }

          decodedValue?.let {
            emit(
              AliroExpeditedTransactionSupportedProtocolVersionsAttributeSubscriptionState.Success(
                it
              )
            )
          }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(
            AliroExpeditedTransactionSupportedProtocolVersionsAttributeSubscriptionState
              .SubscriptionEstablished
          )
        }
      }
    }
  }

  suspend fun readAliroGroupResolvingKeyAttribute(): AliroGroupResolvingKeyAttribute {
    val ATTRIBUTE_ID: UInt = 132u

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

    requireNotNull(attributeData) { "Alirogroupresolvingkey attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: ByteArray? =
      if (!tlvReader.isNull()) {
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getByteArray(AnonymousTag)
        } else {
          null
        }
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return AliroGroupResolvingKeyAttribute(decodedValue)
  }

  suspend fun subscribeAliroGroupResolvingKeyAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<AliroGroupResolvingKeyAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 132u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            AliroGroupResolvingKeyAttributeSubscriptionState.Error(
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
            "Alirogroupresolvingkey attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: ByteArray? =
            if (!tlvReader.isNull()) {
              if (tlvReader.isNextTag(AnonymousTag)) {
                tlvReader.getByteArray(AnonymousTag)
              } else {
                null
              }
            } else {
              tlvReader.getNull(AnonymousTag)
              null
            }

          decodedValue?.let { emit(AliroGroupResolvingKeyAttributeSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(AliroGroupResolvingKeyAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readAliroSupportedBLEUWBProtocolVersionsAttribute():
    AliroSupportedBLEUWBProtocolVersionsAttribute {
    val ATTRIBUTE_ID: UInt = 133u

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

    requireNotNull(attributeData) {
      "Alirosupportedbleuwbprotocolversions attribute not found in response"
    }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: List<ByteArray>? =
      if (tlvReader.isNextTag(AnonymousTag)) {
        buildList<ByteArray> {
          tlvReader.enterArray(AnonymousTag)
          while (!tlvReader.isEndOfContainer()) {
            add(tlvReader.getByteArray(AnonymousTag))
          }
          tlvReader.exitContainer()
        }
      } else {
        null
      }

    return AliroSupportedBLEUWBProtocolVersionsAttribute(decodedValue)
  }

  suspend fun subscribeAliroSupportedBLEUWBProtocolVersionsAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<AliroSupportedBLEUWBProtocolVersionsAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 133u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            AliroSupportedBLEUWBProtocolVersionsAttributeSubscriptionState.Error(
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
            "Alirosupportedbleuwbprotocolversions attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: List<ByteArray>? =
            if (tlvReader.isNextTag(AnonymousTag)) {
              buildList<ByteArray> {
                tlvReader.enterArray(AnonymousTag)
                while (!tlvReader.isEndOfContainer()) {
                  add(tlvReader.getByteArray(AnonymousTag))
                }
                tlvReader.exitContainer()
              }
            } else {
              null
            }

          decodedValue?.let {
            emit(AliroSupportedBLEUWBProtocolVersionsAttributeSubscriptionState.Success(it))
          }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(
            AliroSupportedBLEUWBProtocolVersionsAttributeSubscriptionState.SubscriptionEstablished
          )
        }
      }
    }
  }

  suspend fun readAliroBLEAdvertisingVersionAttribute(): UByte? {
    val ATTRIBUTE_ID: UInt = 134u

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

    requireNotNull(attributeData) { "Alirobleadvertisingversion attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UByte? =
      if (tlvReader.isNextTag(AnonymousTag)) {
        tlvReader.getUByte(AnonymousTag)
      } else {
        null
      }

    return decodedValue
  }

  suspend fun subscribeAliroBLEAdvertisingVersionAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<UByteSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 134u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
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

          requireNotNull(attributeData) {
            "Alirobleadvertisingversion attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UByte? =
            if (tlvReader.isNextTag(AnonymousTag)) {
              tlvReader.getUByte(AnonymousTag)
            } else {
              null
            }

          decodedValue?.let { emit(UByteSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(UByteSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readNumberOfAliroCredentialIssuerKeysSupportedAttribute(): UShort? {
    val ATTRIBUTE_ID: UInt = 135u

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

    requireNotNull(attributeData) {
      "Numberofalirocredentialissuerkeyssupported attribute not found in response"
    }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UShort? =
      if (tlvReader.isNextTag(AnonymousTag)) {
        tlvReader.getUShort(AnonymousTag)
      } else {
        null
      }

    return decodedValue
  }

  suspend fun subscribeNumberOfAliroCredentialIssuerKeysSupportedAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<UShortSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 135u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
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
            "Numberofalirocredentialissuerkeyssupported attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UShort? =
            if (tlvReader.isNextTag(AnonymousTag)) {
              tlvReader.getUShort(AnonymousTag)
            } else {
              null
            }

          decodedValue?.let { emit(UShortSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(UShortSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readNumberOfAliroEndpointKeysSupportedAttribute(): UShort? {
    val ATTRIBUTE_ID: UInt = 136u

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

    requireNotNull(attributeData) {
      "Numberofaliroendpointkeyssupported attribute not found in response"
    }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UShort? =
      if (tlvReader.isNextTag(AnonymousTag)) {
        tlvReader.getUShort(AnonymousTag)
      } else {
        null
      }

    return decodedValue
  }

  suspend fun subscribeNumberOfAliroEndpointKeysSupportedAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<UShortSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 136u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
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
            "Numberofaliroendpointkeyssupported attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UShort? =
            if (tlvReader.isNextTag(AnonymousTag)) {
              tlvReader.getUShort(AnonymousTag)
            } else {
              null
            }

          decodedValue?.let { emit(UShortSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(UShortSubscriptionState.SubscriptionEstablished)
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
    maxInterval: Int
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
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
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
    maxInterval: Int
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
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
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
    maxInterval: Int
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
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
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
    maxInterval: Int
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
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
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
    maxInterval: Int
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
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
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
    maxInterval: Int
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
        maxInterval = Duration.ofSeconds(maxInterval.toLong())
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
    private val logger = Logger.getLogger(DoorLockCluster::class.java.name)
    const val CLUSTER_ID: UInt = 257u
  }
}
