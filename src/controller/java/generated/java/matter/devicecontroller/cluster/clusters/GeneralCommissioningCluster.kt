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

class GeneralCommissioningCluster(
  private val controller: MatterController,
  private val endpointId: UShort
) {
  class ArmFailSafeResponse(val errorCode: UInt, val debugText: String)

  class SetRegulatoryConfigResponse(val errorCode: UInt, val debugText: String)

  class CommissioningCompleteResponse(val errorCode: UInt, val debugText: String)

  class BasicCommissioningInfoAttribute(
    val value: GeneralCommissioningClusterBasicCommissioningInfo
  )

  class GeneratedCommandListAttribute(val value: List<UInt>)

  class AcceptedCommandListAttribute(val value: List<UInt>)

  class EventListAttribute(val value: List<UInt>)

  class AttributeListAttribute(val value: List<UInt>)

  suspend fun armFailSafe(
    expiryLengthSeconds: UShort,
    breadcrumb: ULong,
    timedInvokeTimeoutMs: Int? = null
  ): ArmFailSafeResponse {
    val commandId = 0L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun setRegulatoryConfig(
    newRegulatoryConfig: UInt,
    countryCode: String,
    breadcrumb: ULong,
    timedInvokeTimeoutMs: Int? = null
  ): SetRegulatoryConfigResponse {
    val commandId = 2L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun commissioningComplete(
    timedInvokeTimeoutMs: Int? = null
  ): CommissioningCompleteResponse {
    val commandId = 4L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun readBreadcrumbAttribute(): ULong {
    // Implementation needs to be added here
  }

  suspend fun writeBreadcrumbAttribute(value: ULong, timedWriteTimeoutMs: Int? = null) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
  }

  suspend fun subscribeBreadcrumbAttribute(minInterval: Int, maxInterval: Int): ULong {
    // Implementation needs to be added here
  }

  suspend fun readBasicCommissioningInfoAttribute(): BasicCommissioningInfoAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeBasicCommissioningInfoAttribute(
    minInterval: Int,
    maxInterval: Int
  ): BasicCommissioningInfoAttribute {
    // Implementation needs to be added here
  }

  suspend fun readRegulatoryConfigAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeRegulatoryConfigAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readLocationCapabilityAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeLocationCapabilityAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readSupportsConcurrentConnectionAttribute(): Boolean {
    // Implementation needs to be added here
  }

  suspend fun subscribeSupportsConcurrentConnectionAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Boolean {
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
    const val CLUSTER_ID: UInt = 48u
  }
}
