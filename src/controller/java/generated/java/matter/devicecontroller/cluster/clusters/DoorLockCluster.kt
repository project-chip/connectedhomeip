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

import java.util.ArrayList

class DoorLockCluster(private val endpointId: UShort) {
  class GetWeekDayScheduleResponse(
    val weekDayIndex: UByte,
    val userIndex: UShort,
    val status: UInt,
    val daysMask: UInt?,
    val startHour: UByte?,
    val startMinute: UByte?,
    val endHour: UByte?,
    val endMinute: UByte?
  )

  class GetYearDayScheduleResponse(
    val yearDayIndex: UByte,
    val userIndex: UShort,
    val status: UInt,
    val localStartTime: UInt?,
    val localEndTime: UInt?
  )

  class GetHolidayScheduleResponse(
    val holidayIndex: UByte,
    val status: UInt,
    val localStartTime: UInt?,
    val localEndTime: UInt?,
    val operatingMode: UInt?
  )

  class GetUserResponse(
    val userIndex: UShort,
    val userName: String?,
    val userUniqueID: UInt?,
    val userStatus: UInt?,
    val userType: UInt?,
    val credentialRule: UInt?,
    val credentials: ArrayList<ChipStructs.DoorLockClusterCredentialStruct>?,
    val creatorFabricIndex: UByte?,
    val lastModifiedFabricIndex: UByte?,
    val nextUserIndex: UShort?
  )

  class SetCredentialResponse(
    val status: UInt,
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

  class LockStateAttribute(val value: UInt?)

  class DoorStateAttribute(val value: UInt?)

  class GeneratedCommandListAttribute(val value: ArrayList<UInt>)

  class AcceptedCommandListAttribute(val value: ArrayList<UInt>)

  class EventListAttribute(val value: ArrayList<UInt>)

  class AttributeListAttribute(val value: ArrayList<UInt>)

  suspend fun lockDoor(PINCode: ByteArray?, timedInvokeTimeoutMs: Int? = null) {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun unlockDoor(PINCode: ByteArray?, timedInvokeTimeoutMs: Int? = null) {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun unlockWithTimeout(
    timeout: UShort,
    PINCode: ByteArray?,
    timedInvokeTimeoutMs: Int? = null
  ) {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun setWeekDaySchedule(
    weekDayIndex: UByte,
    userIndex: UShort,
    daysMask: UInt,
    startHour: UByte,
    startMinute: UByte,
    endHour: UByte,
    endMinute: UByte,
    timedInvokeTimeoutMs: Int? = null
  ) {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun getWeekDaySchedule(
    weekDayIndex: UByte,
    userIndex: UShort,
    timedInvokeTimeoutMs: Int? = null
  ): GetWeekDayScheduleResponse {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun clearWeekDaySchedule(
    weekDayIndex: UByte,
    userIndex: UShort,
    timedInvokeTimeoutMs: Int? = null
  ) {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun setYearDaySchedule(
    yearDayIndex: UByte,
    userIndex: UShort,
    localStartTime: UInt,
    localEndTime: UInt,
    timedInvokeTimeoutMs: Int? = null
  ) {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun getYearDaySchedule(
    yearDayIndex: UByte,
    userIndex: UShort,
    timedInvokeTimeoutMs: Int? = null
  ): GetYearDayScheduleResponse {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun clearYearDaySchedule(
    yearDayIndex: UByte,
    userIndex: UShort,
    timedInvokeTimeoutMs: Int? = null
  ) {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun setHolidaySchedule(
    holidayIndex: UByte,
    localStartTime: UInt,
    localEndTime: UInt,
    operatingMode: UInt,
    timedInvokeTimeoutMs: Int? = null
  ) {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun getHolidaySchedule(
    holidayIndex: UByte,
    timedInvokeTimeoutMs: Int? = null
  ): GetHolidayScheduleResponse {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun clearHolidaySchedule(holidayIndex: UByte, timedInvokeTimeoutMs: Int? = null) {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun setUser(
    operationType: UInt,
    userIndex: UShort,
    userName: String?,
    userUniqueID: UInt?,
    userStatus: UInt?,
    userType: UInt?,
    credentialRule: UInt?,
    timedInvokeTimeoutMs: Int? = null
  ) {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun getUser(userIndex: UShort, timedInvokeTimeoutMs: Int? = null): GetUserResponse {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun clearUser(userIndex: UShort, timedInvokeTimeoutMs: Int? = null) {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun setCredential(
    operationType: UInt,
    credential: ChipStructs.DoorLockClusterCredentialStruct,
    credentialData: ByteArray,
    userIndex: UShort?,
    userStatus: UInt?,
    userType: UInt?,
    timedInvokeTimeoutMs: Int? = null
  ): SetCredentialResponse {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun getCredentialStatus(
    credential: ChipStructs.DoorLockClusterCredentialStruct,
    timedInvokeTimeoutMs: Int? = null
  ): GetCredentialStatusResponse {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun clearCredential(
    credential: ChipStructs.DoorLockClusterCredentialStruct?,
    timedInvokeTimeoutMs: Int? = null
  ) {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun unboltDoor(PINCode: ByteArray?, timedInvokeTimeoutMs: Int? = null) {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun readLockStateAttribute(): LockStateAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeLockStateAttribute(minInterval: Int, maxInterval: Int): LockStateAttribute {
    // Implementation needs to be added here
  }

  suspend fun readLockTypeAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeLockTypeAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readActuatorEnabledAttribute(): Boolean {
    // Implementation needs to be added here
  }

  suspend fun subscribeActuatorEnabledAttribute(minInterval: Int, maxInterval: Int): Boolean {
    // Implementation needs to be added here
  }

  suspend fun readDoorStateAttribute(): DoorStateAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeDoorStateAttribute(minInterval: Int, maxInterval: Int): DoorStateAttribute {
    // Implementation needs to be added here
  }

  suspend fun readDoorOpenEventsAttribute(): UInt {
    // Implementation needs to be added here
  }

  suspend fun writeDoorOpenEventsAttribute(value: UInt) {
    // Implementation needs to be added here
  }

  suspend fun writeDoorOpenEventsAttribute(value: UInt, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeDoorOpenEventsAttribute(minInterval: Int, maxInterval: Int): UInt {
    // Implementation needs to be added here
  }

  suspend fun readDoorClosedEventsAttribute(): UInt {
    // Implementation needs to be added here
  }

  suspend fun writeDoorClosedEventsAttribute(value: UInt) {
    // Implementation needs to be added here
  }

  suspend fun writeDoorClosedEventsAttribute(value: UInt, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeDoorClosedEventsAttribute(minInterval: Int, maxInterval: Int): UInt {
    // Implementation needs to be added here
  }

  suspend fun readOpenPeriodAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun writeOpenPeriodAttribute(value: UShort) {
    // Implementation needs to be added here
  }

  suspend fun writeOpenPeriodAttribute(value: UShort, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeOpenPeriodAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readNumberOfTotalUsersSupportedAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeNumberOfTotalUsersSupportedAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UShort {
    // Implementation needs to be added here
  }

  suspend fun readNumberOfPINUsersSupportedAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeNumberOfPINUsersSupportedAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UShort {
    // Implementation needs to be added here
  }

  suspend fun readNumberOfRFIDUsersSupportedAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeNumberOfRFIDUsersSupportedAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UShort {
    // Implementation needs to be added here
  }

  suspend fun readNumberOfWeekDaySchedulesSupportedPerUserAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeNumberOfWeekDaySchedulesSupportedPerUserAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UByte {
    // Implementation needs to be added here
  }

  suspend fun readNumberOfYearDaySchedulesSupportedPerUserAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeNumberOfYearDaySchedulesSupportedPerUserAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UByte {
    // Implementation needs to be added here
  }

  suspend fun readNumberOfHolidaySchedulesSupportedAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeNumberOfHolidaySchedulesSupportedAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UByte {
    // Implementation needs to be added here
  }

  suspend fun readMaxPINCodeLengthAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeMaxPINCodeLengthAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readMinPINCodeLengthAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeMinPINCodeLengthAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readMaxRFIDCodeLengthAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeMaxRFIDCodeLengthAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readMinRFIDCodeLengthAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeMinRFIDCodeLengthAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readCredentialRulesSupportAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeCredentialRulesSupportAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readNumberOfCredentialsSupportedPerUserAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeNumberOfCredentialsSupportedPerUserAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UByte {
    // Implementation needs to be added here
  }

  suspend fun readLanguageAttribute(): CharString {
    // Implementation needs to be added here
  }

  suspend fun writeLanguageAttribute(value: String) {
    // Implementation needs to be added here
  }

  suspend fun writeLanguageAttribute(value: String, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeLanguageAttribute(minInterval: Int, maxInterval: Int): CharString {
    // Implementation needs to be added here
  }

  suspend fun readLEDSettingsAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun writeLEDSettingsAttribute(value: UByte) {
    // Implementation needs to be added here
  }

  suspend fun writeLEDSettingsAttribute(value: UByte, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeLEDSettingsAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readAutoRelockTimeAttribute(): UInt {
    // Implementation needs to be added here
  }

  suspend fun writeAutoRelockTimeAttribute(value: UInt) {
    // Implementation needs to be added here
  }

  suspend fun writeAutoRelockTimeAttribute(value: UInt, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeAutoRelockTimeAttribute(minInterval: Int, maxInterval: Int): UInt {
    // Implementation needs to be added here
  }

  suspend fun readSoundVolumeAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun writeSoundVolumeAttribute(value: UByte) {
    // Implementation needs to be added here
  }

  suspend fun writeSoundVolumeAttribute(value: UByte, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeSoundVolumeAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readOperatingModeAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun writeOperatingModeAttribute(value: UInt) {
    // Implementation needs to be added here
  }

  suspend fun writeOperatingModeAttribute(value: UInt, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeOperatingModeAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readSupportedOperatingModesAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeSupportedOperatingModesAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UShort {
    // Implementation needs to be added here
  }

  suspend fun readDefaultConfigurationRegisterAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeDefaultConfigurationRegisterAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UShort {
    // Implementation needs to be added here
  }

  suspend fun readEnableLocalProgrammingAttribute(): Boolean {
    // Implementation needs to be added here
  }

  suspend fun writeEnableLocalProgrammingAttribute(value: Boolean) {
    // Implementation needs to be added here
  }

  suspend fun writeEnableLocalProgrammingAttribute(value: Boolean, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeEnableLocalProgrammingAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Boolean {
    // Implementation needs to be added here
  }

  suspend fun readEnableOneTouchLockingAttribute(): Boolean {
    // Implementation needs to be added here
  }

  suspend fun writeEnableOneTouchLockingAttribute(value: Boolean) {
    // Implementation needs to be added here
  }

  suspend fun writeEnableOneTouchLockingAttribute(value: Boolean, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeEnableOneTouchLockingAttribute(minInterval: Int, maxInterval: Int): Boolean {
    // Implementation needs to be added here
  }

  suspend fun readEnableInsideStatusLEDAttribute(): Boolean {
    // Implementation needs to be added here
  }

  suspend fun writeEnableInsideStatusLEDAttribute(value: Boolean) {
    // Implementation needs to be added here
  }

  suspend fun writeEnableInsideStatusLEDAttribute(value: Boolean, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeEnableInsideStatusLEDAttribute(minInterval: Int, maxInterval: Int): Boolean {
    // Implementation needs to be added here
  }

  suspend fun readEnablePrivacyModeButtonAttribute(): Boolean {
    // Implementation needs to be added here
  }

  suspend fun writeEnablePrivacyModeButtonAttribute(value: Boolean) {
    // Implementation needs to be added here
  }

  suspend fun writeEnablePrivacyModeButtonAttribute(value: Boolean, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeEnablePrivacyModeButtonAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Boolean {
    // Implementation needs to be added here
  }

  suspend fun readLocalProgrammingFeaturesAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun writeLocalProgrammingFeaturesAttribute(value: UInt) {
    // Implementation needs to be added here
  }

  suspend fun writeLocalProgrammingFeaturesAttribute(value: UInt, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeLocalProgrammingFeaturesAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UByte {
    // Implementation needs to be added here
  }

  suspend fun readWrongCodeEntryLimitAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun writeWrongCodeEntryLimitAttribute(value: UByte) {
    // Implementation needs to be added here
  }

  suspend fun writeWrongCodeEntryLimitAttribute(value: UByte, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeWrongCodeEntryLimitAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readUserCodeTemporaryDisableTimeAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun writeUserCodeTemporaryDisableTimeAttribute(value: UByte) {
    // Implementation needs to be added here
  }

  suspend fun writeUserCodeTemporaryDisableTimeAttribute(value: UByte, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeUserCodeTemporaryDisableTimeAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UByte {
    // Implementation needs to be added here
  }

  suspend fun readSendPINOverTheAirAttribute(): Boolean {
    // Implementation needs to be added here
  }

  suspend fun writeSendPINOverTheAirAttribute(value: Boolean) {
    // Implementation needs to be added here
  }

  suspend fun writeSendPINOverTheAirAttribute(value: Boolean, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeSendPINOverTheAirAttribute(minInterval: Int, maxInterval: Int): Boolean {
    // Implementation needs to be added here
  }

  suspend fun readRequirePINforRemoteOperationAttribute(): Boolean {
    // Implementation needs to be added here
  }

  suspend fun writeRequirePINforRemoteOperationAttribute(value: Boolean) {
    // Implementation needs to be added here
  }

  suspend fun writeRequirePINforRemoteOperationAttribute(value: Boolean, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeRequirePINforRemoteOperationAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Boolean {
    // Implementation needs to be added here
  }

  suspend fun readExpiringUserTimeoutAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun writeExpiringUserTimeoutAttribute(value: UShort) {
    // Implementation needs to be added here
  }

  suspend fun writeExpiringUserTimeoutAttribute(value: UShort, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeExpiringUserTimeoutAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readGeneratedCommandListAttribute(): GeneratedCommandListAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeGeneratedCommandListAttribute(
    minInterval: Int,
    maxInterval: Int
  ): GeneratedCommandListAttribute {
    // Implementation needs to be added here
  }

  suspend fun readAcceptedCommandListAttribute(): AcceptedCommandListAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeAcceptedCommandListAttribute(
    minInterval: Int,
    maxInterval: Int
  ): AcceptedCommandListAttribute {
    // Implementation needs to be added here
  }

  suspend fun readEventListAttribute(): EventListAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeEventListAttribute(minInterval: Int, maxInterval: Int): EventListAttribute {
    // Implementation needs to be added here
  }

  suspend fun readAttributeListAttribute(): AttributeListAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeAttributeListAttribute(
    minInterval: Int,
    maxInterval: Int
  ): AttributeListAttribute {
    // Implementation needs to be added here
  }

  suspend fun readFeatureMapAttribute(): UInt {
    // Implementation needs to be added here
  }

  suspend fun subscribeFeatureMapAttribute(minInterval: Int, maxInterval: Int): UInt {
    // Implementation needs to be added here
  }

  suspend fun readClusterRevisionAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeClusterRevisionAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  companion object {
    const val CLUSTER_ID: UInt = 257u
  }
}
