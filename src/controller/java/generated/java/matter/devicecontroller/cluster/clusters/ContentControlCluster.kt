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

class ContentControlCluster(
  private val controller: MatterController,
  private val endpointId: UShort
) {
  class ResetPINResponse(val PINCode: String)

  class OnDemandRatingsAttribute(val value: List<ContentControlClusterRatingNameStruct>?)

  class ScheduledContentRatingsAttribute(val value: List<ContentControlClusterRatingNameStruct>?)

  class GeneratedCommandListAttribute(val value: List<UInt>)

  class AcceptedCommandListAttribute(val value: List<UInt>)

  class EventListAttribute(val value: List<UInt>)

  class AttributeListAttribute(val value: List<UInt>)

  suspend fun updatePIN(oldPIN: String?, newPIN: String, timedInvokeTimeoutMs: Int? = null) {
    val commandId = 0L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun resetPIN(timedInvokeTimeoutMs: Int? = null): ResetPINResponse {
    val commandId = 1L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun enable(timedInvokeTimeoutMs: Int? = null) {
    val commandId = 3L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun disable(timedInvokeTimeoutMs: Int? = null) {
    val commandId = 4L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun addBonusTime(PINCode: String?, bonusTime: UInt?, timedInvokeTimeoutMs: Int? = null) {
    val commandId = 5L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun setScreenDailyTime(screenTime: UInt, timedInvokeTimeoutMs: Int? = null) {
    val commandId = 6L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun blockUnratedContent(timedInvokeTimeoutMs: Int? = null) {
    val commandId = 7L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun unblockUnratedContent(timedInvokeTimeoutMs: Int? = null) {
    val commandId = 8L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun setOnDemandRatingThreshold(rating: String, timedInvokeTimeoutMs: Int? = null) {
    val commandId = 9L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun setScheduledContentRatingThreshold(
    rating: String,
    timedInvokeTimeoutMs: Int? = null
  ) {
    val commandId = 10L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun readEnabledAttribute(): Boolean {
    // Implementation needs to be added here
  }

  suspend fun subscribeEnabledAttribute(minInterval: Int, maxInterval: Int): Boolean {
    // Implementation needs to be added here
  }

  suspend fun readOnDemandRatingsAttribute(): OnDemandRatingsAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeOnDemandRatingsAttribute(
    minInterval: Int,
    maxInterval: Int
  ): OnDemandRatingsAttribute {
    // Implementation needs to be added here
  }

  suspend fun readOnDemandRatingThresholdAttribute(): CharString {
    // Implementation needs to be added here
  }

  suspend fun subscribeOnDemandRatingThresholdAttribute(
    minInterval: Int,
    maxInterval: Int
  ): CharString {
    // Implementation needs to be added here
  }

  suspend fun readScheduledContentRatingsAttribute(): ScheduledContentRatingsAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeScheduledContentRatingsAttribute(
    minInterval: Int,
    maxInterval: Int
  ): ScheduledContentRatingsAttribute {
    // Implementation needs to be added here
  }

  suspend fun readScheduledContentRatingThresholdAttribute(): CharString {
    // Implementation needs to be added here
  }

  suspend fun subscribeScheduledContentRatingThresholdAttribute(
    minInterval: Int,
    maxInterval: Int
  ): CharString {
    // Implementation needs to be added here
  }

  suspend fun readScreenDailyTimeAttribute(): UInt {
    // Implementation needs to be added here
  }

  suspend fun subscribeScreenDailyTimeAttribute(minInterval: Int, maxInterval: Int): UInt {
    // Implementation needs to be added here
  }

  suspend fun readRemainingScreenTimeAttribute(): UInt {
    // Implementation needs to be added here
  }

  suspend fun subscribeRemainingScreenTimeAttribute(minInterval: Int, maxInterval: Int): UInt {
    // Implementation needs to be added here
  }

  suspend fun readBlockUnratedAttribute(): Boolean {
    // Implementation needs to be added here
  }

  suspend fun subscribeBlockUnratedAttribute(minInterval: Int, maxInterval: Int): Boolean {
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
    const val CLUSTER_ID: UInt = 1295u
  }
}
