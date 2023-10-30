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

class GeneralCommissioningCluster(private val endpointId: UShort) {
  class ArmFailSafeResponse(val errorCode: UInt, val debugText: String)

  class SetRegulatoryConfigResponse(val errorCode: UInt, val debugText: String)

  class CommissioningCompleteResponse(val errorCode: UInt, val debugText: String)

  class BasicCommissioningInfoAttribute(
    val value: ChipStructs.GeneralCommissioningClusterBasicCommissioningInfo
  )

  class GeneratedCommandListAttribute(val value: ArrayList<UInt>)

  class AcceptedCommandListAttribute(val value: ArrayList<UInt>)

  class EventListAttribute(val value: ArrayList<UInt>)

  class AttributeListAttribute(val value: ArrayList<UInt>)

  suspend fun armFailSafe(expiryLengthSeconds: UShort, breadcrumb: ULong): ArmFailSafeResponse {
    // Implementation needs to be added here
  }

  suspend fun armFailSafe(
    expiryLengthSeconds: UShort,
    breadcrumb: ULong,
    timedInvokeTimeoutMs: Int
  ): ArmFailSafeResponse {
    // Implementation needs to be added here
  }

  suspend fun setRegulatoryConfig(
    newRegulatoryConfig: UInt,
    countryCode: String,
    breadcrumb: ULong
  ): SetRegulatoryConfigResponse {
    // Implementation needs to be added here
  }

  suspend fun setRegulatoryConfig(
    newRegulatoryConfig: UInt,
    countryCode: String,
    breadcrumb: ULong,
    timedInvokeTimeoutMs: Int
  ): SetRegulatoryConfigResponse {
    // Implementation needs to be added here
  }

  suspend fun commissioningComplete(): CommissioningCompleteResponse {
    // Implementation needs to be added here
  }

  suspend fun commissioningComplete(timedInvokeTimeoutMs: Int): CommissioningCompleteResponse {
    // Implementation needs to be added here
  }

  suspend fun readBreadcrumbAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun writeBreadcrumbAttribute(value: ULong) {
    // Implementation needs to be added here
  }

  suspend fun writeBreadcrumbAttribute(value: ULong, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeBreadcrumbAttribute(minInterval: Int, maxInterval: Int): Long {
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

  suspend fun readRegulatoryConfigAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeRegulatoryConfigAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readLocationCapabilityAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeLocationCapabilityAttribute(minInterval: Int, maxInterval: Int): Integer {
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

  suspend fun readFeatureMapAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeFeatureMapAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readClusterRevisionAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeClusterRevisionAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  companion object {
    const val CLUSTER_ID: UInt = 48u
  }
}
