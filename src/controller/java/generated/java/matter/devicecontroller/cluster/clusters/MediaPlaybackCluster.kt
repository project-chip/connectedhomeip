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

class MediaPlaybackCluster(private val endpointId: UShort) {
  companion object {
    const val CLUSTER_ID: UInt = 1286u
  }

  fun play(callback: PlaybackResponseCallback) {
    // Implementation needs to be added here
  }

  fun play(callback: PlaybackResponseCallback, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  fun pause(callback: PlaybackResponseCallback) {
    // Implementation needs to be added here
  }

  fun pause(callback: PlaybackResponseCallback, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  fun stop(callback: PlaybackResponseCallback) {
    // Implementation needs to be added here
  }

  fun stop(callback: PlaybackResponseCallback, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  fun startOver(callback: PlaybackResponseCallback) {
    // Implementation needs to be added here
  }

  fun startOver(callback: PlaybackResponseCallback, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  fun previous(callback: PlaybackResponseCallback) {
    // Implementation needs to be added here
  }

  fun previous(callback: PlaybackResponseCallback, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  fun next(callback: PlaybackResponseCallback) {
    // Implementation needs to be added here
  }

  fun next(callback: PlaybackResponseCallback, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  fun rewind(callback: PlaybackResponseCallback) {
    // Implementation needs to be added here
  }

  fun rewind(callback: PlaybackResponseCallback, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  fun fastForward(callback: PlaybackResponseCallback) {
    // Implementation needs to be added here
  }

  fun fastForward(callback: PlaybackResponseCallback, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  fun skipForward(callback: PlaybackResponseCallback, deltaPositionMilliseconds: Long) {
    // Implementation needs to be added here
  }

  fun skipForward(
    callback: PlaybackResponseCallback,
    deltaPositionMilliseconds: Long,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun skipBackward(callback: PlaybackResponseCallback, deltaPositionMilliseconds: Long) {
    // Implementation needs to be added here
  }

  fun skipBackward(
    callback: PlaybackResponseCallback,
    deltaPositionMilliseconds: Long,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun seek(callback: PlaybackResponseCallback, position: Long) {
    // Implementation needs to be added here
  }

  fun seek(callback: PlaybackResponseCallback, position: Long, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  interface PlaybackResponseCallback {
    fun onSuccess(status: Integer, data: String?)

    fun onError(error: Exception)
  }

  interface StartTimeAttributeCallback {
    fun onSuccess(value: Long?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface DurationAttributeCallback {
    fun onSuccess(value: Long?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface SampledPositionAttributeCallback {
    fun onSuccess(value: ChipStructs.MediaPlaybackClusterPlaybackPositionStruct?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface SeekRangeEndAttributeCallback {
    fun onSuccess(value: Long?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface SeekRangeStartAttributeCallback {
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

  fun readCurrentStateAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeCurrentStateAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readStartTimeAttribute(callback: StartTimeAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeStartTimeAttribute(
    callback: StartTimeAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readDurationAttribute(callback: DurationAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeDurationAttribute(
    callback: DurationAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readSampledPositionAttribute(callback: SampledPositionAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeSampledPositionAttribute(
    callback: SampledPositionAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readPlaybackSpeedAttribute(callback: FloatAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribePlaybackSpeedAttribute(
    callback: FloatAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readSeekRangeEndAttribute(callback: SeekRangeEndAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeSeekRangeEndAttribute(
    callback: SeekRangeEndAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readSeekRangeStartAttribute(callback: SeekRangeStartAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeSeekRangeStartAttribute(
    callback: SeekRangeStartAttributeCallback,
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
