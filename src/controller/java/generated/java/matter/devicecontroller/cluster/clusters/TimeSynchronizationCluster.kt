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

class TimeSynchronizationCluster(private val endpointId: UShort) {
  companion object {
    const val CLUSTER_ID: UInt = 56u
  }

  fun setUTCTime(
    callback: DefaultClusterCallback,
    UTCTime: Long,
    granularity: Integer,
    timeSource: Integer?
  ) {
    // Implementation needs to be added here
  }

  fun setUTCTime(
    callback: DefaultClusterCallback,
    UTCTime: Long,
    granularity: Integer,
    timeSource: Integer?,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun setTrustedTimeSource(
    callback: DefaultClusterCallback,
    trustedTimeSource: ChipStructs.TimeSynchronizationClusterFabricScopedTrustedTimeSourceStruct?
  ) {
    // Implementation needs to be added here
  }

  fun setTrustedTimeSource(
    callback: DefaultClusterCallback,
    trustedTimeSource: ChipStructs.TimeSynchronizationClusterFabricScopedTrustedTimeSourceStruct?,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun setTimeZone(
    callback: SetTimeZoneResponseCallback,
    timeZone: ArrayList<ChipStructs.TimeSynchronizationClusterTimeZoneStruct>
  ) {
    // Implementation needs to be added here
  }

  fun setTimeZone(
    callback: SetTimeZoneResponseCallback,
    timeZone: ArrayList<ChipStructs.TimeSynchronizationClusterTimeZoneStruct>,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun setDSTOffset(
    callback: DefaultClusterCallback,
    DSTOffset: ArrayList<ChipStructs.TimeSynchronizationClusterDSTOffsetStruct>
  ) {
    // Implementation needs to be added here
  }

  fun setDSTOffset(
    callback: DefaultClusterCallback,
    DSTOffset: ArrayList<ChipStructs.TimeSynchronizationClusterDSTOffsetStruct>,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun setDefaultNTP(callback: DefaultClusterCallback, defaultNTP: String?) {
    // Implementation needs to be added here
  }

  fun setDefaultNTP(
    callback: DefaultClusterCallback,
    defaultNTP: String?,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  interface SetTimeZoneResponseCallback {
    fun onSuccess(DSTOffsetRequired: Boolean)

    fun onError(error: Exception)
  }

  interface UTCTimeAttributeCallback {
    fun onSuccess(value: Long?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface TrustedTimeSourceAttributeCallback {
    fun onSuccess(value: ChipStructs.TimeSynchronizationClusterTrustedTimeSourceStruct?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface DefaultNTPAttributeCallback {
    fun onSuccess(value: String?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface TimeZoneAttributeCallback {
    fun onSuccess(value: ArrayList<ChipStructs.TimeSynchronizationClusterTimeZoneStruct>?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface DSTOffsetAttributeCallback {
    fun onSuccess(value: ArrayList<ChipStructs.TimeSynchronizationClusterDSTOffsetStruct>?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface LocalTimeAttributeCallback {
    fun onSuccess(value: Long?)

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

  fun readUTCTimeAttribute(callback: UTCTimeAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeUTCTimeAttribute(
    callback: UTCTimeAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readGranularityAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeGranularityAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readTimeSourceAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeTimeSourceAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readTrustedTimeSourceAttribute(callback: TrustedTimeSourceAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeTrustedTimeSourceAttribute(
    callback: TrustedTimeSourceAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readDefaultNTPAttribute(callback: DefaultNTPAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeDefaultNTPAttribute(
    callback: DefaultNTPAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readTimeZoneAttribute(callback: TimeZoneAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeTimeZoneAttribute(
    callback: TimeZoneAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readDSTOffsetAttribute(callback: DSTOffsetAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeDSTOffsetAttribute(
    callback: DSTOffsetAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readLocalTimeAttribute(callback: LocalTimeAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeLocalTimeAttribute(
    callback: LocalTimeAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readTimeZoneDatabaseAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeTimeZoneDatabaseAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNTPServerAvailableAttribute(callback: BooleanAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeNTPServerAvailableAttribute(
    callback: BooleanAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readTimeZoneListMaxSizeAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeTimeZoneListMaxSizeAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readDSTOffsetListMaxSizeAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeDSTOffsetListMaxSizeAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readSupportsDNSResolveAttribute(callback: BooleanAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeSupportsDNSResolveAttribute(
    callback: BooleanAttributeCallback,
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
