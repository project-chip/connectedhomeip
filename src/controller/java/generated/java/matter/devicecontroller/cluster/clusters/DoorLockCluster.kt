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

package matter.devicecontroller.cluster.clusters

import java.time.Duration
import java.util.logging.Level
import java.util.logging.Logger
import matter.controller.*
import matter.controller.model.AttributePath
import matter.controller.model.CommandPath
import matter.devicecontroller.cluster.structs.*
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

  class DoorStateAttribute(val value: UByte?)

  class GeneratedCommandListAttribute(val value: List<UInt>)

  class AcceptedCommandListAttribute(val value: List<UInt>)

  class EventListAttribute(val value: List<UInt>)

  class AttributeListAttribute(val value: List<UInt>)

  suspend fun lockDoor(PINCode: ByteArray?, timedInvokeTimeoutMs: Int) {
    val commandId: UInt = 0u
    val timeoutMs: Duration = Duration.ofMillis(timedInvokeTimeoutMs.toLong())

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_P_I_N_CODE_REQ: Int = 0
    PINCode?.let { tlvWriter.put(ContextSpecificTag(TAG_P_I_N_CODE_REQ), PINCode) }
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timeoutMs
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.INFO, "Invoke command succeeded: ${response}")
  }

  suspend fun unlockDoor(PINCode: ByteArray?, timedInvokeTimeoutMs: Int) {
    val commandId: UInt = 1u
    val timeoutMs: Duration = Duration.ofMillis(timedInvokeTimeoutMs.toLong())

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_P_I_N_CODE_REQ: Int = 0
    PINCode?.let { tlvWriter.put(ContextSpecificTag(TAG_P_I_N_CODE_REQ), PINCode) }
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timeoutMs
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.INFO, "Invoke command succeeded: ${response}")
  }

  suspend fun unlockWithTimeout(timeout: UShort, PINCode: ByteArray?, timedInvokeTimeoutMs: Int) {
    val commandId: UInt = 3u
    val timeoutMs: Duration = Duration.ofMillis(timedInvokeTimeoutMs.toLong())

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
        timedRequest = timeoutMs
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.INFO, "Invoke command succeeded: ${response}")
  }

  suspend fun setWeekDaySchedule(
    weekDayIndex: UByte,
    userIndex: UShort,
    daysMask: UByte,
    startHour: UByte,
    startMinute: UByte,
    endHour: UByte,
    endMinute: UByte,
    timedInvokeTimeoutMs: Int? = null
  ) {
    val commandId: UInt = 11u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

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
        timedRequest = timeoutMs
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.INFO, "Invoke command succeeded: ${response}")
  }

  suspend fun getWeekDaySchedule(
    weekDayIndex: UByte,
    userIndex: UShort,
    timedInvokeTimeoutMs: Int? = null
  ): GetWeekDayScheduleResponse {
    val commandId: UInt = 12u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

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
        timedRequest = timeoutMs
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.INFO, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_WEEK_DAY_INDEX: Int = 0
    val weekDayIndex_decoded = tlvReader.getUByte(ContextSpecificTag(TAG_WEEK_DAY_INDEX))

    val TAG_USER_INDEX: Int = 1
    val userIndex_decoded = tlvReader.getUShort(ContextSpecificTag(TAG_USER_INDEX))

    val TAG_STATUS: Int = 2
    val status_decoded = tlvReader.getUByte(ContextSpecificTag(TAG_STATUS))

    val TAG_DAYS_MASK: Int = 3
    val daysMask_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_DAYS_MASK))) {
        tlvReader.getUByte(ContextSpecificTag(TAG_DAYS_MASK))
      } else {
        null
      }

    val TAG_START_HOUR: Int = 4
    val startHour_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_START_HOUR))) {
        tlvReader.getUByte(ContextSpecificTag(TAG_START_HOUR))
      } else {
        null
      }

    val TAG_START_MINUTE: Int = 5
    val startMinute_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_START_MINUTE))) {
        tlvReader.getUByte(ContextSpecificTag(TAG_START_MINUTE))
      } else {
        null
      }

    val TAG_END_HOUR: Int = 6
    val endHour_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_END_HOUR))) {
        tlvReader.getUByte(ContextSpecificTag(TAG_END_HOUR))
      } else {
        null
      }

    val TAG_END_MINUTE: Int = 7
    val endMinute_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_END_MINUTE))) {
        tlvReader.getUByte(ContextSpecificTag(TAG_END_MINUTE))
      } else {
        null
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
    timedInvokeTimeoutMs: Int? = null
  ) {
    val commandId: UInt = 13u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

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
        timedRequest = timeoutMs
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.INFO, "Invoke command succeeded: ${response}")
  }

  suspend fun setYearDaySchedule(
    yearDayIndex: UByte,
    userIndex: UShort,
    localStartTime: UInt,
    localEndTime: UInt,
    timedInvokeTimeoutMs: Int? = null
  ) {
    val commandId: UInt = 14u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

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
        timedRequest = timeoutMs
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.INFO, "Invoke command succeeded: ${response}")
  }

  suspend fun getYearDaySchedule(
    yearDayIndex: UByte,
    userIndex: UShort,
    timedInvokeTimeoutMs: Int? = null
  ): GetYearDayScheduleResponse {
    val commandId: UInt = 15u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

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
        timedRequest = timeoutMs
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.INFO, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_YEAR_DAY_INDEX: Int = 0
    val yearDayIndex_decoded = tlvReader.getUByte(ContextSpecificTag(TAG_YEAR_DAY_INDEX))

    val TAG_USER_INDEX: Int = 1
    val userIndex_decoded = tlvReader.getUShort(ContextSpecificTag(TAG_USER_INDEX))

    val TAG_STATUS: Int = 2
    val status_decoded = tlvReader.getUByte(ContextSpecificTag(TAG_STATUS))

    val TAG_LOCAL_START_TIME: Int = 3
    val localStartTime_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_LOCAL_START_TIME))) {
        tlvReader.getUInt(ContextSpecificTag(TAG_LOCAL_START_TIME))
      } else {
        null
      }

    val TAG_LOCAL_END_TIME: Int = 4
    val localEndTime_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_LOCAL_END_TIME))) {
        tlvReader.getUInt(ContextSpecificTag(TAG_LOCAL_END_TIME))
      } else {
        null
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
    timedInvokeTimeoutMs: Int? = null
  ) {
    val commandId: UInt = 16u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

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
        timedRequest = timeoutMs
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.INFO, "Invoke command succeeded: ${response}")
  }

  suspend fun setHolidaySchedule(
    holidayIndex: UByte,
    localStartTime: UInt,
    localEndTime: UInt,
    operatingMode: UByte,
    timedInvokeTimeoutMs: Int? = null
  ) {
    val commandId: UInt = 17u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

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
        timedRequest = timeoutMs
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.INFO, "Invoke command succeeded: ${response}")
  }

  suspend fun getHolidaySchedule(
    holidayIndex: UByte,
    timedInvokeTimeoutMs: Int? = null
  ): GetHolidayScheduleResponse {
    val commandId: UInt = 18u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_HOLIDAY_INDEX_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_HOLIDAY_INDEX_REQ), holidayIndex)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timeoutMs
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.INFO, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_HOLIDAY_INDEX: Int = 0
    val holidayIndex_decoded = tlvReader.getUByte(ContextSpecificTag(TAG_HOLIDAY_INDEX))

    val TAG_STATUS: Int = 1
    val status_decoded = tlvReader.getUByte(ContextSpecificTag(TAG_STATUS))

    val TAG_LOCAL_START_TIME: Int = 2
    val localStartTime_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_LOCAL_START_TIME))) {
        tlvReader.getUInt(ContextSpecificTag(TAG_LOCAL_START_TIME))
      } else {
        null
      }

    val TAG_LOCAL_END_TIME: Int = 3
    val localEndTime_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_LOCAL_END_TIME))) {
        tlvReader.getUInt(ContextSpecificTag(TAG_LOCAL_END_TIME))
      } else {
        null
      }

    val TAG_OPERATING_MODE: Int = 4
    val operatingMode_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_OPERATING_MODE))) {
        tlvReader.getUByte(ContextSpecificTag(TAG_OPERATING_MODE))
      } else {
        null
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

  suspend fun clearHolidaySchedule(holidayIndex: UByte, timedInvokeTimeoutMs: Int? = null) {
    val commandId: UInt = 19u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_HOLIDAY_INDEX_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_HOLIDAY_INDEX_REQ), holidayIndex)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timeoutMs
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.INFO, "Invoke command succeeded: ${response}")
  }

  suspend fun setUser(
    operationType: UByte,
    userIndex: UShort,
    userName: String?,
    userUniqueID: UInt?,
    userStatus: UByte?,
    userType: UByte?,
    credentialRule: UByte?,
    timedInvokeTimeoutMs: Int
  ) {
    val commandId: UInt = 26u
    val timeoutMs: Duration = Duration.ofMillis(timedInvokeTimeoutMs.toLong())

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
        timedRequest = timeoutMs
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.INFO, "Invoke command succeeded: ${response}")
  }

  suspend fun getUser(userIndex: UShort, timedInvokeTimeoutMs: Int? = null): GetUserResponse {
    val commandId: UInt = 27u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_USER_INDEX_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_USER_INDEX_REQ), userIndex)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timeoutMs
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.INFO, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_USER_INDEX: Int = 0
    val userIndex_decoded = tlvReader.getUShort(ContextSpecificTag(TAG_USER_INDEX))

    val TAG_USER_NAME: Int = 1
    val userName_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_USER_NAME))) {
        if (!tlvReader.isNull()) {
          tlvReader.getString(ContextSpecificTag(TAG_USER_NAME))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_USER_NAME))
          null
        }
      } else {
        null
      }

    val TAG_USER_UNIQUE_I_D: Int = 2
    val userUniqueID_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_USER_UNIQUE_I_D))) {
        if (!tlvReader.isNull()) {
          tlvReader.getUInt(ContextSpecificTag(TAG_USER_UNIQUE_I_D))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_USER_UNIQUE_I_D))
          null
        }
      } else {
        null
      }

    val TAG_USER_STATUS: Int = 3
    val userStatus_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_USER_STATUS))) {
        if (!tlvReader.isNull()) {
          tlvReader.getUByte(ContextSpecificTag(TAG_USER_STATUS))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_USER_STATUS))
          null
        }
      } else {
        null
      }

    val TAG_USER_TYPE: Int = 4
    val userType_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_USER_TYPE))) {
        if (!tlvReader.isNull()) {
          tlvReader.getUByte(ContextSpecificTag(TAG_USER_TYPE))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_USER_TYPE))
          null
        }
      } else {
        null
      }

    val TAG_CREDENTIAL_RULE: Int = 5
    val credentialRule_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_CREDENTIAL_RULE))) {
        if (!tlvReader.isNull()) {
          tlvReader.getUByte(ContextSpecificTag(TAG_CREDENTIAL_RULE))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_CREDENTIAL_RULE))
          null
        }
      } else {
        null
      }

    val TAG_CREDENTIALS: Int = 6
    val credentials_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_CREDENTIALS))) {
        if (!tlvReader.isNull()) {
          buildList<DoorLockClusterCredentialStruct> {
            tlvReader.enterArray(ContextSpecificTag(TAG_CREDENTIALS))
            while (!tlvReader.isEndOfContainer()) {
              add(DoorLockClusterCredentialStruct.fromTlv(AnonymousTag, tlvReader))
            }
            tlvReader.exitContainer()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_CREDENTIALS))
          null
        }
      } else {
        null
      }

    val TAG_CREATOR_FABRIC_INDEX: Int = 7
    val creatorFabricIndex_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_CREATOR_FABRIC_INDEX))) {
        if (!tlvReader.isNull()) {
          tlvReader.getUByte(ContextSpecificTag(TAG_CREATOR_FABRIC_INDEX))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_CREATOR_FABRIC_INDEX))
          null
        }
      } else {
        null
      }

    val TAG_LAST_MODIFIED_FABRIC_INDEX: Int = 8
    val lastModifiedFabricIndex_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_LAST_MODIFIED_FABRIC_INDEX))) {
        if (!tlvReader.isNull()) {
          tlvReader.getUByte(ContextSpecificTag(TAG_LAST_MODIFIED_FABRIC_INDEX))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_LAST_MODIFIED_FABRIC_INDEX))
          null
        }
      } else {
        null
      }

    val TAG_NEXT_USER_INDEX: Int = 9
    val nextUserIndex_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_NEXT_USER_INDEX))) {
        if (!tlvReader.isNull()) {
          tlvReader.getUShort(ContextSpecificTag(TAG_NEXT_USER_INDEX))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_NEXT_USER_INDEX))
          null
        }
      } else {
        null
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

  suspend fun clearUser(userIndex: UShort, timedInvokeTimeoutMs: Int) {
    val commandId: UInt = 29u
    val timeoutMs: Duration = Duration.ofMillis(timedInvokeTimeoutMs.toLong())

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_USER_INDEX_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_USER_INDEX_REQ), userIndex)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timeoutMs
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.INFO, "Invoke command succeeded: ${response}")
  }

  suspend fun setCredential(
    operationType: UByte,
    credential: DoorLockClusterCredentialStruct,
    credentialData: ByteArray,
    userIndex: UShort?,
    userStatus: UByte?,
    userType: UByte?,
    timedInvokeTimeoutMs: Int
  ): SetCredentialResponse {
    val commandId: UInt = 34u
    val timeoutMs: Duration = Duration.ofMillis(timedInvokeTimeoutMs.toLong())

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
        timedRequest = timeoutMs
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.INFO, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_STATUS: Int = 0
    val status_decoded = tlvReader.getUByte(ContextSpecificTag(TAG_STATUS))

    val TAG_USER_INDEX: Int = 1
    val userIndex_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_USER_INDEX))) {
        if (!tlvReader.isNull()) {
          tlvReader.getUShort(ContextSpecificTag(TAG_USER_INDEX))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_USER_INDEX))
          null
        }
      } else {
        null
      }

    val TAG_NEXT_CREDENTIAL_INDEX: Int = 2
    val nextCredentialIndex_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_NEXT_CREDENTIAL_INDEX))) {
        if (!tlvReader.isNull()) {
          tlvReader.getUShort(ContextSpecificTag(TAG_NEXT_CREDENTIAL_INDEX))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_NEXT_CREDENTIAL_INDEX))
          null
        }
      } else {
        null
      }
    tlvReader.exitContainer()

    return SetCredentialResponse(status_decoded, userIndex_decoded, nextCredentialIndex_decoded)
  }

  suspend fun getCredentialStatus(
    credential: DoorLockClusterCredentialStruct,
    timedInvokeTimeoutMs: Int? = null
  ): GetCredentialStatusResponse {
    val commandId: UInt = 36u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_CREDENTIAL_REQ: Int = 0
    credential.toTlv(ContextSpecificTag(TAG_CREDENTIAL_REQ), tlvWriter)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timeoutMs
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.INFO, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_CREDENTIAL_EXISTS: Int = 0
    val credentialExists_decoded = tlvReader.getBoolean(ContextSpecificTag(TAG_CREDENTIAL_EXISTS))

    val TAG_USER_INDEX: Int = 1
    val userIndex_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_USER_INDEX))) {
        if (!tlvReader.isNull()) {
          tlvReader.getUShort(ContextSpecificTag(TAG_USER_INDEX))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_USER_INDEX))
          null
        }
      } else {
        null
      }

    val TAG_CREATOR_FABRIC_INDEX: Int = 2
    val creatorFabricIndex_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_CREATOR_FABRIC_INDEX))) {
        if (!tlvReader.isNull()) {
          tlvReader.getUByte(ContextSpecificTag(TAG_CREATOR_FABRIC_INDEX))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_CREATOR_FABRIC_INDEX))
          null
        }
      } else {
        null
      }

    val TAG_LAST_MODIFIED_FABRIC_INDEX: Int = 3
    val lastModifiedFabricIndex_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_LAST_MODIFIED_FABRIC_INDEX))) {
        if (!tlvReader.isNull()) {
          tlvReader.getUByte(ContextSpecificTag(TAG_LAST_MODIFIED_FABRIC_INDEX))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_LAST_MODIFIED_FABRIC_INDEX))
          null
        }
      } else {
        null
      }

    val TAG_NEXT_CREDENTIAL_INDEX: Int = 4
    val nextCredentialIndex_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_NEXT_CREDENTIAL_INDEX))) {
        if (!tlvReader.isNull()) {
          tlvReader.getUShort(ContextSpecificTag(TAG_NEXT_CREDENTIAL_INDEX))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_NEXT_CREDENTIAL_INDEX))
          null
        }
      } else {
        null
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
    timedInvokeTimeoutMs: Int
  ) {
    val commandId: UInt = 38u
    val timeoutMs: Duration = Duration.ofMillis(timedInvokeTimeoutMs.toLong())

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_CREDENTIAL_REQ: Int = 0
    credential?.let { credential.toTlv(ContextSpecificTag(TAG_CREDENTIAL_REQ), tlvWriter) }
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timeoutMs
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.INFO, "Invoke command succeeded: ${response}")
  }

  suspend fun unboltDoor(PINCode: ByteArray?, timedInvokeTimeoutMs: Int) {
    val commandId: UInt = 39u
    val timeoutMs: Duration = Duration.ofMillis(timedInvokeTimeoutMs.toLong())

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_P_I_N_CODE_REQ: Int = 0
    PINCode?.let { tlvWriter.put(ContextSpecificTag(TAG_P_I_N_CODE_REQ), PINCode) }
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timeoutMs
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.INFO, "Invoke command succeeded: ${response}")
  }

  suspend fun readLockStateAttribute(): LockStateAttribute {
    val ATTRIBUTE_ID_LOCKSTATE: UInt = 0u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_LOCKSTATE
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_LOCKSTATE
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readLockTypeAttribute(): UByte {
    val ATTRIBUTE_ID_LOCKTYPE: UInt = 1u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_LOCKTYPE
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_LOCKTYPE
        }

      requireNotNull(attributeData) { "Locktype attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: UByte = tlvReader.getUByte(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readActuatorEnabledAttribute(): Boolean {
    val ATTRIBUTE_ID_ACTUATORENABLED: UInt = 2u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_ACTUATORENABLED
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_ACTUATORENABLED
        }

      requireNotNull(attributeData) { "Actuatorenabled attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Boolean = tlvReader.getBoolean(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readDoorStateAttribute(): DoorStateAttribute {
    val ATTRIBUTE_ID_DOORSTATE: UInt = 3u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_DOORSTATE
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_DOORSTATE
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readDoorOpenEventsAttribute(): UInt? {
    val ATTRIBUTE_ID_DOOROPENEVENTS: UInt = 4u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_DOOROPENEVENTS
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_DOOROPENEVENTS
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeDoorOpenEventsAttribute(value: UInt, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_DOOROPENEVENTS: UInt = 4u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_DOOROPENEVENTS
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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

  suspend fun readDoorClosedEventsAttribute(): UInt? {
    val ATTRIBUTE_ID_DOORCLOSEDEVENTS: UInt = 5u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_DOORCLOSEDEVENTS
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_DOORCLOSEDEVENTS
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeDoorClosedEventsAttribute(value: UInt, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_DOORCLOSEDEVENTS: UInt = 5u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_DOORCLOSEDEVENTS
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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

  suspend fun readOpenPeriodAttribute(): UShort? {
    val ATTRIBUTE_ID_OPENPERIOD: UInt = 6u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_OPENPERIOD
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_OPENPERIOD
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeOpenPeriodAttribute(value: UShort, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_OPENPERIOD: UInt = 6u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_OPENPERIOD
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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

  suspend fun readNumberOfTotalUsersSupportedAttribute(): UShort? {
    val ATTRIBUTE_ID_NUMBEROFTOTALUSERSSUPPORTED: UInt = 17u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_NUMBEROFTOTALUSERSSUPPORTED
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_NUMBEROFTOTALUSERSSUPPORTED
        }

      requireNotNull(attributeData) {
        "Numberoftotaluserssupported attribute not found in response"
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readNumberOfPINUsersSupportedAttribute(): UShort? {
    val ATTRIBUTE_ID_NUMBEROFPINUSERSSUPPORTED: UInt = 18u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_NUMBEROFPINUSERSSUPPORTED
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_NUMBEROFPINUSERSSUPPORTED
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readNumberOfRFIDUsersSupportedAttribute(): UShort? {
    val ATTRIBUTE_ID_NUMBEROFRFIDUSERSSUPPORTED: UInt = 19u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_NUMBEROFRFIDUSERSSUPPORTED
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_NUMBEROFRFIDUSERSSUPPORTED
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readNumberOfWeekDaySchedulesSupportedPerUserAttribute(): UByte? {
    val ATTRIBUTE_ID_NUMBEROFWEEKDAYSCHEDULESSUPPORTEDPERUSER: UInt = 20u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_NUMBEROFWEEKDAYSCHEDULESSUPPORTEDPERUSER
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_NUMBEROFWEEKDAYSCHEDULESSUPPORTEDPERUSER
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readNumberOfYearDaySchedulesSupportedPerUserAttribute(): UByte? {
    val ATTRIBUTE_ID_NUMBEROFYEARDAYSCHEDULESSUPPORTEDPERUSER: UInt = 21u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_NUMBEROFYEARDAYSCHEDULESSUPPORTEDPERUSER
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_NUMBEROFYEARDAYSCHEDULESSUPPORTEDPERUSER
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readNumberOfHolidaySchedulesSupportedAttribute(): UByte? {
    val ATTRIBUTE_ID_NUMBEROFHOLIDAYSCHEDULESSUPPORTED: UInt = 22u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_NUMBEROFHOLIDAYSCHEDULESSUPPORTED
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_NUMBEROFHOLIDAYSCHEDULESSUPPORTED
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readMaxPINCodeLengthAttribute(): UByte? {
    val ATTRIBUTE_ID_MAXPINCODELENGTH: UInt = 23u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_MAXPINCODELENGTH
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_MAXPINCODELENGTH
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readMinPINCodeLengthAttribute(): UByte? {
    val ATTRIBUTE_ID_MINPINCODELENGTH: UInt = 24u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_MINPINCODELENGTH
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_MINPINCODELENGTH
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readMaxRFIDCodeLengthAttribute(): UByte? {
    val ATTRIBUTE_ID_MAXRFIDCODELENGTH: UInt = 25u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_MAXRFIDCODELENGTH
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_MAXRFIDCODELENGTH
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readMinRFIDCodeLengthAttribute(): UByte? {
    val ATTRIBUTE_ID_MINRFIDCODELENGTH: UInt = 26u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_MINRFIDCODELENGTH
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_MINRFIDCODELENGTH
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readCredentialRulesSupportAttribute(): UByte? {
    val ATTRIBUTE_ID_CREDENTIALRULESSUPPORT: UInt = 27u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_CREDENTIALRULESSUPPORT
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_CREDENTIALRULESSUPPORT
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readNumberOfCredentialsSupportedPerUserAttribute(): UByte? {
    val ATTRIBUTE_ID_NUMBEROFCREDENTIALSSUPPORTEDPERUSER: UInt = 28u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_NUMBEROFCREDENTIALSSUPPORTEDPERUSER
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_NUMBEROFCREDENTIALSSUPPORTEDPERUSER
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readLanguageAttribute(): String? {
    val ATTRIBUTE_ID_LANGUAGE: UInt = 33u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_LANGUAGE
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_LANGUAGE
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeLanguageAttribute(value: String, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_LANGUAGE: UInt = 33u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_LANGUAGE
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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

  suspend fun readLEDSettingsAttribute(): UByte? {
    val ATTRIBUTE_ID_LEDSETTINGS: UInt = 34u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_LEDSETTINGS
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_LEDSETTINGS
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeLEDSettingsAttribute(value: UByte, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_LEDSETTINGS: UInt = 34u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_LEDSETTINGS
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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

  suspend fun readAutoRelockTimeAttribute(): UInt {
    val ATTRIBUTE_ID_AUTORELOCKTIME: UInt = 35u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_AUTORELOCKTIME
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_AUTORELOCKTIME
        }

      requireNotNull(attributeData) { "Autorelocktime attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: UInt = tlvReader.getUInt(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeAutoRelockTimeAttribute(value: UInt, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_AUTORELOCKTIME: UInt = 35u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_AUTORELOCKTIME
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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

  suspend fun readSoundVolumeAttribute(): UByte? {
    val ATTRIBUTE_ID_SOUNDVOLUME: UInt = 36u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_SOUNDVOLUME
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_SOUNDVOLUME
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeSoundVolumeAttribute(value: UByte, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_SOUNDVOLUME: UInt = 36u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_SOUNDVOLUME
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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

  suspend fun readOperatingModeAttribute(): UByte {
    val ATTRIBUTE_ID_OPERATINGMODE: UInt = 37u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_OPERATINGMODE
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_OPERATINGMODE
        }

      requireNotNull(attributeData) { "Operatingmode attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: UByte = tlvReader.getUByte(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeOperatingModeAttribute(value: UByte, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_OPERATINGMODE: UInt = 37u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_OPERATINGMODE
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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

  suspend fun readSupportedOperatingModesAttribute(): UShort {
    val ATTRIBUTE_ID_SUPPORTEDOPERATINGMODES: UInt = 38u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_SUPPORTEDOPERATINGMODES
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_SUPPORTEDOPERATINGMODES
        }

      requireNotNull(attributeData) { "Supportedoperatingmodes attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: UShort = tlvReader.getUShort(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readDefaultConfigurationRegisterAttribute(): UShort? {
    val ATTRIBUTE_ID_DEFAULTCONFIGURATIONREGISTER: UInt = 39u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_DEFAULTCONFIGURATIONREGISTER
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_DEFAULTCONFIGURATIONREGISTER
        }

      requireNotNull(attributeData) {
        "Defaultconfigurationregister attribute not found in response"
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readEnableLocalProgrammingAttribute(): Boolean? {
    val ATTRIBUTE_ID_ENABLELOCALPROGRAMMING: UInt = 40u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_ENABLELOCALPROGRAMMING
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_ENABLELOCALPROGRAMMING
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeEnableLocalProgrammingAttribute(
    value: Boolean,
    timedWriteTimeoutMs: Int? = null
  ) {
    val ATTRIBUTE_ID_ENABLELOCALPROGRAMMING: UInt = 40u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_ENABLELOCALPROGRAMMING
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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

  suspend fun readEnableOneTouchLockingAttribute(): Boolean? {
    val ATTRIBUTE_ID_ENABLEONETOUCHLOCKING: UInt = 41u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_ENABLEONETOUCHLOCKING
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_ENABLEONETOUCHLOCKING
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeEnableOneTouchLockingAttribute(
    value: Boolean,
    timedWriteTimeoutMs: Int? = null
  ) {
    val ATTRIBUTE_ID_ENABLEONETOUCHLOCKING: UInt = 41u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_ENABLEONETOUCHLOCKING
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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

  suspend fun readEnableInsideStatusLEDAttribute(): Boolean? {
    val ATTRIBUTE_ID_ENABLEINSIDESTATUSLED: UInt = 42u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_ENABLEINSIDESTATUSLED
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_ENABLEINSIDESTATUSLED
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeEnableInsideStatusLEDAttribute(
    value: Boolean,
    timedWriteTimeoutMs: Int? = null
  ) {
    val ATTRIBUTE_ID_ENABLEINSIDESTATUSLED: UInt = 42u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_ENABLEINSIDESTATUSLED
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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

  suspend fun readEnablePrivacyModeButtonAttribute(): Boolean? {
    val ATTRIBUTE_ID_ENABLEPRIVACYMODEBUTTON: UInt = 43u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_ENABLEPRIVACYMODEBUTTON
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_ENABLEPRIVACYMODEBUTTON
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeEnablePrivacyModeButtonAttribute(
    value: Boolean,
    timedWriteTimeoutMs: Int? = null
  ) {
    val ATTRIBUTE_ID_ENABLEPRIVACYMODEBUTTON: UInt = 43u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_ENABLEPRIVACYMODEBUTTON
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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

  suspend fun readLocalProgrammingFeaturesAttribute(): UByte? {
    val ATTRIBUTE_ID_LOCALPROGRAMMINGFEATURES: UInt = 44u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_LOCALPROGRAMMINGFEATURES
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_LOCALPROGRAMMINGFEATURES
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeLocalProgrammingFeaturesAttribute(
    value: UByte,
    timedWriteTimeoutMs: Int? = null
  ) {
    val ATTRIBUTE_ID_LOCALPROGRAMMINGFEATURES: UInt = 44u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_LOCALPROGRAMMINGFEATURES
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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

  suspend fun readWrongCodeEntryLimitAttribute(): UByte? {
    val ATTRIBUTE_ID_WRONGCODEENTRYLIMIT: UInt = 48u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_WRONGCODEENTRYLIMIT
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_WRONGCODEENTRYLIMIT
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeWrongCodeEntryLimitAttribute(value: UByte, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_WRONGCODEENTRYLIMIT: UInt = 48u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_WRONGCODEENTRYLIMIT
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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

  suspend fun readUserCodeTemporaryDisableTimeAttribute(): UByte? {
    val ATTRIBUTE_ID_USERCODETEMPORARYDISABLETIME: UInt = 49u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_USERCODETEMPORARYDISABLETIME
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_USERCODETEMPORARYDISABLETIME
        }

      requireNotNull(attributeData) {
        "Usercodetemporarydisabletime attribute not found in response"
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeUserCodeTemporaryDisableTimeAttribute(
    value: UByte,
    timedWriteTimeoutMs: Int? = null
  ) {
    val ATTRIBUTE_ID_USERCODETEMPORARYDISABLETIME: UInt = 49u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_USERCODETEMPORARYDISABLETIME
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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

  suspend fun readSendPINOverTheAirAttribute(): Boolean? {
    val ATTRIBUTE_ID_SENDPINOVERTHEAIR: UInt = 50u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_SENDPINOVERTHEAIR
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_SENDPINOVERTHEAIR
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeSendPINOverTheAirAttribute(value: Boolean, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_SENDPINOVERTHEAIR: UInt = 50u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_SENDPINOVERTHEAIR
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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

  suspend fun readRequirePINforRemoteOperationAttribute(): Boolean? {
    val ATTRIBUTE_ID_REQUIREPINFORREMOTEOPERATION: UInt = 51u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_REQUIREPINFORREMOTEOPERATION
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_REQUIREPINFORREMOTEOPERATION
        }

      requireNotNull(attributeData) {
        "Requirepinforremoteoperation attribute not found in response"
      }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Boolean? =
        if (tlvReader.isNextTag(AnonymousTag)) {
          tlvReader.getBoolean(AnonymousTag)
        } else {
          null
        }

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeRequirePINforRemoteOperationAttribute(
    value: Boolean,
    timedWriteTimeoutMs: Int? = null
  ) {
    val ATTRIBUTE_ID_REQUIREPINFORREMOTEOPERATION: UInt = 51u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_REQUIREPINFORREMOTEOPERATION
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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

  suspend fun readExpiringUserTimeoutAttribute(): UShort? {
    val ATTRIBUTE_ID_EXPIRINGUSERTIMEOUT: UInt = 53u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_EXPIRINGUSERTIMEOUT
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_EXPIRINGUSERTIMEOUT
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeExpiringUserTimeoutAttribute(value: UShort, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_EXPIRINGUSERTIMEOUT: UInt = 53u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_EXPIRINGUSERTIMEOUT
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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

  suspend fun readGeneratedCommandListAttribute(): GeneratedCommandListAttribute {
    val ATTRIBUTE_ID_GENERATEDCOMMANDLIST: UInt = 65528u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_GENERATEDCOMMANDLIST
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_GENERATEDCOMMANDLIST
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readAcceptedCommandListAttribute(): AcceptedCommandListAttribute {
    val ATTRIBUTE_ID_ACCEPTEDCOMMANDLIST: UInt = 65529u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_ACCEPTEDCOMMANDLIST
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_ACCEPTEDCOMMANDLIST
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readEventListAttribute(): EventListAttribute {
    val ATTRIBUTE_ID_EVENTLIST: UInt = 65530u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_EVENTLIST
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_EVENTLIST
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readAttributeListAttribute(): AttributeListAttribute {
    val ATTRIBUTE_ID_ATTRIBUTELIST: UInt = 65531u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_ATTRIBUTELIST
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_ATTRIBUTELIST
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readFeatureMapAttribute(): UInt {
    val ATTRIBUTE_ID_FEATUREMAP: UInt = 65532u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_FEATUREMAP
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_FEATUREMAP
        }

      requireNotNull(attributeData) { "Featuremap attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: UInt = tlvReader.getUInt(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readClusterRevisionAttribute(): UShort {
    val ATTRIBUTE_ID_CLUSTERREVISION: UInt = 65533u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_CLUSTERREVISION
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_CLUSTERREVISION
        }

      requireNotNull(attributeData) { "Clusterrevision attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: UShort = tlvReader.getUShort(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  companion object {
    private val logger = Logger.getLogger(DoorLockCluster::class.java.name)
    const val CLUSTER_ID: UInt = 257u
  }
}
