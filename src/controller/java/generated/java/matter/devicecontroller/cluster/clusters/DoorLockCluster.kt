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
  companion object {
    const val CLUSTER_ID: UInt = 257u
  }

  fun lockDoor(callback: DefaultClusterCallback, PINCode: ByteArray?, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  fun unlockDoor(callback: DefaultClusterCallback, PINCode: ByteArray?, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  fun unlockWithTimeout(
    callback: DefaultClusterCallback,
    timeout: Integer,
    PINCode: ByteArray?,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun setWeekDaySchedule(
    callback: DefaultClusterCallback,
    weekDayIndex: Integer,
    userIndex: Integer,
    daysMask: Integer,
    startHour: Integer,
    startMinute: Integer,
    endHour: Integer,
    endMinute: Integer
  ) {
    // Implementation needs to be added here
  }

  fun setWeekDaySchedule(
    callback: DefaultClusterCallback,
    weekDayIndex: Integer,
    userIndex: Integer,
    daysMask: Integer,
    startHour: Integer,
    startMinute: Integer,
    endHour: Integer,
    endMinute: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun getWeekDaySchedule(
    callback: GetWeekDayScheduleResponseCallback,
    weekDayIndex: Integer,
    userIndex: Integer
  ) {
    // Implementation needs to be added here
  }

  fun getWeekDaySchedule(
    callback: GetWeekDayScheduleResponseCallback,
    weekDayIndex: Integer,
    userIndex: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun clearWeekDaySchedule(
    callback: DefaultClusterCallback,
    weekDayIndex: Integer,
    userIndex: Integer
  ) {
    // Implementation needs to be added here
  }

  fun clearWeekDaySchedule(
    callback: DefaultClusterCallback,
    weekDayIndex: Integer,
    userIndex: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun setYearDaySchedule(
    callback: DefaultClusterCallback,
    yearDayIndex: Integer,
    userIndex: Integer,
    localStartTime: Long,
    localEndTime: Long
  ) {
    // Implementation needs to be added here
  }

  fun setYearDaySchedule(
    callback: DefaultClusterCallback,
    yearDayIndex: Integer,
    userIndex: Integer,
    localStartTime: Long,
    localEndTime: Long,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun getYearDaySchedule(
    callback: GetYearDayScheduleResponseCallback,
    yearDayIndex: Integer,
    userIndex: Integer
  ) {
    // Implementation needs to be added here
  }

  fun getYearDaySchedule(
    callback: GetYearDayScheduleResponseCallback,
    yearDayIndex: Integer,
    userIndex: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun clearYearDaySchedule(
    callback: DefaultClusterCallback,
    yearDayIndex: Integer,
    userIndex: Integer
  ) {
    // Implementation needs to be added here
  }

  fun clearYearDaySchedule(
    callback: DefaultClusterCallback,
    yearDayIndex: Integer,
    userIndex: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun setHolidaySchedule(
    callback: DefaultClusterCallback,
    holidayIndex: Integer,
    localStartTime: Long,
    localEndTime: Long,
    operatingMode: Integer
  ) {
    // Implementation needs to be added here
  }

  fun setHolidaySchedule(
    callback: DefaultClusterCallback,
    holidayIndex: Integer,
    localStartTime: Long,
    localEndTime: Long,
    operatingMode: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun getHolidaySchedule(callback: GetHolidayScheduleResponseCallback, holidayIndex: Integer) {
    // Implementation needs to be added here
  }

  fun getHolidaySchedule(
    callback: GetHolidayScheduleResponseCallback,
    holidayIndex: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun clearHolidaySchedule(callback: DefaultClusterCallback, holidayIndex: Integer) {
    // Implementation needs to be added here
  }

  fun clearHolidaySchedule(
    callback: DefaultClusterCallback,
    holidayIndex: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun setUser(
    callback: DefaultClusterCallback,
    operationType: Integer,
    userIndex: Integer,
    userName: String?,
    userUniqueID: Long?,
    userStatus: Integer?,
    userType: Integer?,
    credentialRule: Integer?,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun getUser(callback: GetUserResponseCallback, userIndex: Integer) {
    // Implementation needs to be added here
  }

  fun getUser(callback: GetUserResponseCallback, userIndex: Integer, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  fun clearUser(callback: DefaultClusterCallback, userIndex: Integer, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  fun setCredential(
    callback: SetCredentialResponseCallback,
    operationType: Integer,
    credential: ChipStructs.DoorLockClusterCredentialStruct,
    credentialData: ByteArray,
    userIndex: Integer?,
    userStatus: Integer?,
    userType: Integer?,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun getCredentialStatus(
    callback: GetCredentialStatusResponseCallback,
    credential: ChipStructs.DoorLockClusterCredentialStruct
  ) {
    // Implementation needs to be added here
  }

  fun getCredentialStatus(
    callback: GetCredentialStatusResponseCallback,
    credential: ChipStructs.DoorLockClusterCredentialStruct,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun clearCredential(
    callback: DefaultClusterCallback,
    credential: ChipStructs.DoorLockClusterCredentialStruct?,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun unboltDoor(callback: DefaultClusterCallback, PINCode: ByteArray?, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  interface GetWeekDayScheduleResponseCallback {
    fun onSuccess(
      weekDayIndex: Integer,
      userIndex: Integer,
      status: Integer,
      daysMask: Integer?,
      startHour: Integer?,
      startMinute: Integer?,
      endHour: Integer?,
      endMinute: Integer?
    )

    fun onError(error: Exception)
  }

  interface GetYearDayScheduleResponseCallback {
    fun onSuccess(
      yearDayIndex: Integer,
      userIndex: Integer,
      status: Integer,
      localStartTime: Long?,
      localEndTime: Long?
    )

    fun onError(error: Exception)
  }

  interface GetHolidayScheduleResponseCallback {
    fun onSuccess(
      holidayIndex: Integer,
      status: Integer,
      localStartTime: Long?,
      localEndTime: Long?,
      operatingMode: Integer?
    )

    fun onError(error: Exception)
  }

  interface GetUserResponseCallback {
    fun onSuccess(
      userIndex: Integer,
      userName: String?,
      userUniqueID: Long?,
      userStatus: Integer?,
      userType: Integer?,
      credentialRule: Integer?,
      credentials: ArrayList<ChipStructs.DoorLockClusterCredentialStruct>?,
      creatorFabricIndex: Integer?,
      lastModifiedFabricIndex: Integer?,
      nextUserIndex: Integer?
    )

    fun onError(error: Exception)
  }

  interface SetCredentialResponseCallback {
    fun onSuccess(status: Integer, userIndex: Integer?, nextCredentialIndex: Integer?)

    fun onError(error: Exception)
  }

  interface GetCredentialStatusResponseCallback {
    fun onSuccess(
      credentialExists: Boolean,
      userIndex: Integer?,
      creatorFabricIndex: Integer?,
      lastModifiedFabricIndex: Integer?,
      nextCredentialIndex: Integer?
    )

    fun onError(error: Exception)
  }

  interface LockStateAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface DoorStateAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface GeneratedCommandListAttributeCallback {
    fun onSuccess(value: ArrayList<Long>)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface AcceptedCommandListAttributeCallback {
    fun onSuccess(value: ArrayList<Long>)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface EventListAttributeCallback {
    fun onSuccess(value: ArrayList<Long>)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface AttributeListAttributeCallback {
    fun onSuccess(value: ArrayList<Long>)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  fun readLockStateAttribute(callback: LockStateAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeLockStateAttribute(
    callback: LockStateAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readLockTypeAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeLockTypeAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readActuatorEnabledAttribute(callback: BooleanAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeActuatorEnabledAttribute(
    callback: BooleanAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readDoorStateAttribute(callback: DoorStateAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeDoorStateAttribute(
    callback: DoorStateAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readDoorOpenEventsAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeDoorOpenEventsAttribute(callback: DefaultClusterCallback, value: Long) {
    // Implementation needs to be added here
  }

  fun writeDoorOpenEventsAttribute(
    callback: DefaultClusterCallback,
    value: Long,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeDoorOpenEventsAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readDoorClosedEventsAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeDoorClosedEventsAttribute(callback: DefaultClusterCallback, value: Long) {
    // Implementation needs to be added here
  }

  fun writeDoorClosedEventsAttribute(
    callback: DefaultClusterCallback,
    value: Long,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeDoorClosedEventsAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readOpenPeriodAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeOpenPeriodAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeOpenPeriodAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeOpenPeriodAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNumberOfTotalUsersSupportedAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeNumberOfTotalUsersSupportedAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNumberOfPINUsersSupportedAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeNumberOfPINUsersSupportedAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNumberOfRFIDUsersSupportedAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeNumberOfRFIDUsersSupportedAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNumberOfWeekDaySchedulesSupportedPerUserAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeNumberOfWeekDaySchedulesSupportedPerUserAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNumberOfYearDaySchedulesSupportedPerUserAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeNumberOfYearDaySchedulesSupportedPerUserAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNumberOfHolidaySchedulesSupportedAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeNumberOfHolidaySchedulesSupportedAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readMaxPINCodeLengthAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeMaxPINCodeLengthAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readMinPINCodeLengthAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeMinPINCodeLengthAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readMaxRFIDCodeLengthAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeMaxRFIDCodeLengthAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readMinRFIDCodeLengthAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeMinRFIDCodeLengthAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readCredentialRulesSupportAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeCredentialRulesSupportAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNumberOfCredentialsSupportedPerUserAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeNumberOfCredentialsSupportedPerUserAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readLanguageAttribute(callback: CharStringAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeLanguageAttribute(callback: DefaultClusterCallback, value: String) {
    // Implementation needs to be added here
  }

  fun writeLanguageAttribute(
    callback: DefaultClusterCallback,
    value: String,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeLanguageAttribute(
    callback: CharStringAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readLEDSettingsAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeLEDSettingsAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeLEDSettingsAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeLEDSettingsAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readAutoRelockTimeAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeAutoRelockTimeAttribute(callback: DefaultClusterCallback, value: Long) {
    // Implementation needs to be added here
  }

  fun writeAutoRelockTimeAttribute(
    callback: DefaultClusterCallback,
    value: Long,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeAutoRelockTimeAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readSoundVolumeAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeSoundVolumeAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeSoundVolumeAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeSoundVolumeAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readOperatingModeAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeOperatingModeAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeOperatingModeAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeOperatingModeAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readSupportedOperatingModesAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeSupportedOperatingModesAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readDefaultConfigurationRegisterAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeDefaultConfigurationRegisterAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readEnableLocalProgrammingAttribute(callback: BooleanAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeEnableLocalProgrammingAttribute(callback: DefaultClusterCallback, value: Boolean) {
    // Implementation needs to be added here
  }

  fun writeEnableLocalProgrammingAttribute(
    callback: DefaultClusterCallback,
    value: Boolean,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeEnableLocalProgrammingAttribute(
    callback: BooleanAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readEnableOneTouchLockingAttribute(callback: BooleanAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeEnableOneTouchLockingAttribute(callback: DefaultClusterCallback, value: Boolean) {
    // Implementation needs to be added here
  }

  fun writeEnableOneTouchLockingAttribute(
    callback: DefaultClusterCallback,
    value: Boolean,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeEnableOneTouchLockingAttribute(
    callback: BooleanAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readEnableInsideStatusLEDAttribute(callback: BooleanAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeEnableInsideStatusLEDAttribute(callback: DefaultClusterCallback, value: Boolean) {
    // Implementation needs to be added here
  }

  fun writeEnableInsideStatusLEDAttribute(
    callback: DefaultClusterCallback,
    value: Boolean,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeEnableInsideStatusLEDAttribute(
    callback: BooleanAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readEnablePrivacyModeButtonAttribute(callback: BooleanAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeEnablePrivacyModeButtonAttribute(callback: DefaultClusterCallback, value: Boolean) {
    // Implementation needs to be added here
  }

  fun writeEnablePrivacyModeButtonAttribute(
    callback: DefaultClusterCallback,
    value: Boolean,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeEnablePrivacyModeButtonAttribute(
    callback: BooleanAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readLocalProgrammingFeaturesAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeLocalProgrammingFeaturesAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeLocalProgrammingFeaturesAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeLocalProgrammingFeaturesAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readWrongCodeEntryLimitAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeWrongCodeEntryLimitAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeWrongCodeEntryLimitAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeWrongCodeEntryLimitAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readUserCodeTemporaryDisableTimeAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeUserCodeTemporaryDisableTimeAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeUserCodeTemporaryDisableTimeAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeUserCodeTemporaryDisableTimeAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readSendPINOverTheAirAttribute(callback: BooleanAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeSendPINOverTheAirAttribute(callback: DefaultClusterCallback, value: Boolean) {
    // Implementation needs to be added here
  }

  fun writeSendPINOverTheAirAttribute(
    callback: DefaultClusterCallback,
    value: Boolean,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeSendPINOverTheAirAttribute(
    callback: BooleanAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readRequirePINforRemoteOperationAttribute(callback: BooleanAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeRequirePINforRemoteOperationAttribute(callback: DefaultClusterCallback, value: Boolean) {
    // Implementation needs to be added here
  }

  fun writeRequirePINforRemoteOperationAttribute(
    callback: DefaultClusterCallback,
    value: Boolean,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeRequirePINforRemoteOperationAttribute(
    callback: BooleanAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readExpiringUserTimeoutAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeExpiringUserTimeoutAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeExpiringUserTimeoutAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeExpiringUserTimeoutAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readGeneratedCommandListAttribute(callback: GeneratedCommandListAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeGeneratedCommandListAttribute(
    callback: GeneratedCommandListAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readAcceptedCommandListAttribute(callback: AcceptedCommandListAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeAcceptedCommandListAttribute(
    callback: AcceptedCommandListAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readEventListAttribute(callback: EventListAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeEventListAttribute(
    callback: EventListAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readAttributeListAttribute(callback: AttributeListAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeAttributeListAttribute(
    callback: AttributeListAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readFeatureMapAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeFeatureMapAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readClusterRevisionAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeClusterRevisionAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }
}
