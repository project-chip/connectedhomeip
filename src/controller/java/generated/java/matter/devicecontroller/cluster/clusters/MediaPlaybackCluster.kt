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

import matter.controller.MatterController
import matter.devicecontroller.cluster.structs.*

class MediaPlaybackCluster(
  private val controller: MatterController,
  private val endpointId: UShort
) {
  class PlaybackResponse(val status: UInt, val data: String?)

  class StartTimeAttribute(val value: ULong?)

  class DurationAttribute(val value: ULong?)

  class SampledPositionAttribute(val value: MediaPlaybackClusterPlaybackPositionStruct?)

  class SeekRangeEndAttribute(val value: ULong?)

  class SeekRangeStartAttribute(val value: ULong?)

  class GeneratedCommandListAttribute(val value: List<UInt>)

  class AcceptedCommandListAttribute(val value: List<UInt>)

  class EventListAttribute(val value: List<UInt>)

  class AttributeListAttribute(val value: List<UInt>)

  suspend fun play(timedInvokeTimeoutMs: Int? = null): PlaybackResponse {
    val commandId = 0L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun pause(timedInvokeTimeoutMs: Int? = null): PlaybackResponse {
    val commandId = 1L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun stop(timedInvokeTimeoutMs: Int? = null): PlaybackResponse {
    val commandId = 2L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun startOver(timedInvokeTimeoutMs: Int? = null): PlaybackResponse {
    val commandId = 3L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun previous(timedInvokeTimeoutMs: Int? = null): PlaybackResponse {
    val commandId = 4L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun next(timedInvokeTimeoutMs: Int? = null): PlaybackResponse {
    val commandId = 5L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun rewind(timedInvokeTimeoutMs: Int? = null): PlaybackResponse {
    val commandId = 6L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun fastForward(timedInvokeTimeoutMs: Int? = null): PlaybackResponse {
    val commandId = 7L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun skipForward(
    deltaPositionMilliseconds: ULong,
    timedInvokeTimeoutMs: Int? = null
  ): PlaybackResponse {
    val commandId = 8L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun skipBackward(
    deltaPositionMilliseconds: ULong,
    timedInvokeTimeoutMs: Int? = null
  ): PlaybackResponse {
    val commandId = 9L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun seek(position: ULong, timedInvokeTimeoutMs: Int? = null): PlaybackResponse {
    val commandId = 11L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun readCurrentStateAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeCurrentStateAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readStartTimeAttribute(): StartTimeAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeStartTimeAttribute(minInterval: Int, maxInterval: Int): StartTimeAttribute {
    // Implementation needs to be added here
  }

  suspend fun readDurationAttribute(): DurationAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeDurationAttribute(minInterval: Int, maxInterval: Int): DurationAttribute {
    // Implementation needs to be added here
  }

  suspend fun readSampledPositionAttribute(): SampledPositionAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeSampledPositionAttribute(
    minInterval: Int,
    maxInterval: Int
  ): SampledPositionAttribute {
    // Implementation needs to be added here
  }

  suspend fun readPlaybackSpeedAttribute(): Float {
    // Implementation needs to be added here
  }

  suspend fun subscribePlaybackSpeedAttribute(minInterval: Int, maxInterval: Int): Float {
    // Implementation needs to be added here
  }

  suspend fun readSeekRangeEndAttribute(): SeekRangeEndAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeSeekRangeEndAttribute(
    minInterval: Int,
    maxInterval: Int
  ): SeekRangeEndAttribute {
    // Implementation needs to be added here
  }

  suspend fun readSeekRangeStartAttribute(): SeekRangeStartAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeSeekRangeStartAttribute(
    minInterval: Int,
    maxInterval: Int
  ): SeekRangeStartAttribute {
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
    const val CLUSTER_ID: UInt = 1286u
  }
}
