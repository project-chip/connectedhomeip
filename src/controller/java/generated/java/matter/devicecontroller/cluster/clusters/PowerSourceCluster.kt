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

class PowerSourceCluster(private val controller: MatterController, private val endpointId: UShort) {
  class WiredAssessedInputVoltageAttribute(val value: UInt?)

  class WiredAssessedInputFrequencyAttribute(val value: UShort?)

  class WiredAssessedCurrentAttribute(val value: UInt?)

  class ActiveWiredFaultsAttribute(val value: List<UInt>?)

  class BatVoltageAttribute(val value: UInt?)

  class BatPercentRemainingAttribute(val value: UByte?)

  class BatTimeRemainingAttribute(val value: UInt?)

  class ActiveBatFaultsAttribute(val value: List<UInt>?)

  class BatTimeToFullChargeAttribute(val value: UInt?)

  class BatChargingCurrentAttribute(val value: UInt?)

  class ActiveBatChargeFaultsAttribute(val value: List<UInt>?)

  class EndpointListAttribute(val value: List<UShort>)

  class GeneratedCommandListAttribute(val value: List<UInt>)

  class AcceptedCommandListAttribute(val value: List<UInt>)

  class EventListAttribute(val value: List<UInt>)

  class AttributeListAttribute(val value: List<UInt>)

  suspend fun readStatusAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeStatusAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readOrderAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeOrderAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readDescriptionAttribute(): CharString {
    // Implementation needs to be added here
  }

  suspend fun subscribeDescriptionAttribute(minInterval: Int, maxInterval: Int): CharString {
    // Implementation needs to be added here
  }

  suspend fun readWiredAssessedInputVoltageAttribute(): WiredAssessedInputVoltageAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeWiredAssessedInputVoltageAttribute(
    minInterval: Int,
    maxInterval: Int
  ): WiredAssessedInputVoltageAttribute {
    // Implementation needs to be added here
  }

  suspend fun readWiredAssessedInputFrequencyAttribute(): WiredAssessedInputFrequencyAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeWiredAssessedInputFrequencyAttribute(
    minInterval: Int,
    maxInterval: Int
  ): WiredAssessedInputFrequencyAttribute {
    // Implementation needs to be added here
  }

  suspend fun readWiredCurrentTypeAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeWiredCurrentTypeAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readWiredAssessedCurrentAttribute(): WiredAssessedCurrentAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeWiredAssessedCurrentAttribute(
    minInterval: Int,
    maxInterval: Int
  ): WiredAssessedCurrentAttribute {
    // Implementation needs to be added here
  }

  suspend fun readWiredNominalVoltageAttribute(): UInt {
    // Implementation needs to be added here
  }

  suspend fun subscribeWiredNominalVoltageAttribute(minInterval: Int, maxInterval: Int): UInt {
    // Implementation needs to be added here
  }

  suspend fun readWiredMaximumCurrentAttribute(): UInt {
    // Implementation needs to be added here
  }

  suspend fun subscribeWiredMaximumCurrentAttribute(minInterval: Int, maxInterval: Int): UInt {
    // Implementation needs to be added here
  }

  suspend fun readWiredPresentAttribute(): Boolean {
    // Implementation needs to be added here
  }

  suspend fun subscribeWiredPresentAttribute(minInterval: Int, maxInterval: Int): Boolean {
    // Implementation needs to be added here
  }

  suspend fun readActiveWiredFaultsAttribute(): ActiveWiredFaultsAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeActiveWiredFaultsAttribute(
    minInterval: Int,
    maxInterval: Int
  ): ActiveWiredFaultsAttribute {
    // Implementation needs to be added here
  }

  suspend fun readBatVoltageAttribute(): BatVoltageAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeBatVoltageAttribute(
    minInterval: Int,
    maxInterval: Int
  ): BatVoltageAttribute {
    // Implementation needs to be added here
  }

  suspend fun readBatPercentRemainingAttribute(): BatPercentRemainingAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeBatPercentRemainingAttribute(
    minInterval: Int,
    maxInterval: Int
  ): BatPercentRemainingAttribute {
    // Implementation needs to be added here
  }

  suspend fun readBatTimeRemainingAttribute(): BatTimeRemainingAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeBatTimeRemainingAttribute(
    minInterval: Int,
    maxInterval: Int
  ): BatTimeRemainingAttribute {
    // Implementation needs to be added here
  }

  suspend fun readBatChargeLevelAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeBatChargeLevelAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readBatReplacementNeededAttribute(): Boolean {
    // Implementation needs to be added here
  }

  suspend fun subscribeBatReplacementNeededAttribute(minInterval: Int, maxInterval: Int): Boolean {
    // Implementation needs to be added here
  }

  suspend fun readBatReplaceabilityAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeBatReplaceabilityAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readBatPresentAttribute(): Boolean {
    // Implementation needs to be added here
  }

  suspend fun subscribeBatPresentAttribute(minInterval: Int, maxInterval: Int): Boolean {
    // Implementation needs to be added here
  }

  suspend fun readActiveBatFaultsAttribute(): ActiveBatFaultsAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeActiveBatFaultsAttribute(
    minInterval: Int,
    maxInterval: Int
  ): ActiveBatFaultsAttribute {
    // Implementation needs to be added here
  }

  suspend fun readBatReplacementDescriptionAttribute(): CharString {
    // Implementation needs to be added here
  }

  suspend fun subscribeBatReplacementDescriptionAttribute(
    minInterval: Int,
    maxInterval: Int
  ): CharString {
    // Implementation needs to be added here
  }

  suspend fun readBatCommonDesignationAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeBatCommonDesignationAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readBatANSIDesignationAttribute(): CharString {
    // Implementation needs to be added here
  }

  suspend fun subscribeBatANSIDesignationAttribute(minInterval: Int, maxInterval: Int): CharString {
    // Implementation needs to be added here
  }

  suspend fun readBatIECDesignationAttribute(): CharString {
    // Implementation needs to be added here
  }

  suspend fun subscribeBatIECDesignationAttribute(minInterval: Int, maxInterval: Int): CharString {
    // Implementation needs to be added here
  }

  suspend fun readBatApprovedChemistryAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeBatApprovedChemistryAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readBatCapacityAttribute(): UInt {
    // Implementation needs to be added here
  }

  suspend fun subscribeBatCapacityAttribute(minInterval: Int, maxInterval: Int): UInt {
    // Implementation needs to be added here
  }

  suspend fun readBatQuantityAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeBatQuantityAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readBatChargeStateAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeBatChargeStateAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readBatTimeToFullChargeAttribute(): BatTimeToFullChargeAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeBatTimeToFullChargeAttribute(
    minInterval: Int,
    maxInterval: Int
  ): BatTimeToFullChargeAttribute {
    // Implementation needs to be added here
  }

  suspend fun readBatFunctionalWhileChargingAttribute(): Boolean {
    // Implementation needs to be added here
  }

  suspend fun subscribeBatFunctionalWhileChargingAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Boolean {
    // Implementation needs to be added here
  }

  suspend fun readBatChargingCurrentAttribute(): BatChargingCurrentAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeBatChargingCurrentAttribute(
    minInterval: Int,
    maxInterval: Int
  ): BatChargingCurrentAttribute {
    // Implementation needs to be added here
  }

  suspend fun readActiveBatChargeFaultsAttribute(): ActiveBatChargeFaultsAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeActiveBatChargeFaultsAttribute(
    minInterval: Int,
    maxInterval: Int
  ): ActiveBatChargeFaultsAttribute {
    // Implementation needs to be added here
  }

  suspend fun readEndpointListAttribute(): EndpointListAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeEndpointListAttribute(
    minInterval: Int,
    maxInterval: Int
  ): EndpointListAttribute {
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
    const val CLUSTER_ID: UInt = 47u
  }
}
