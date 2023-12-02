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

class Pm10ConcentrationMeasurementCluster(
  private val controller: MatterController,
  private val endpointId: UShort
) {
  class MeasuredValueAttribute(val value: Float?)

  class MinMeasuredValueAttribute(val value: Float?)

  class MaxMeasuredValueAttribute(val value: Float?)

  class PeakMeasuredValueAttribute(val value: Float?)

  class AverageMeasuredValueAttribute(val value: Float?)

  class GeneratedCommandListAttribute(val value: List<UInt>)

  class AcceptedCommandListAttribute(val value: List<UInt>)

  class EventListAttribute(val value: List<UInt>)

  class AttributeListAttribute(val value: List<UInt>)

  suspend fun readMeasuredValueAttribute(): MeasuredValueAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeMeasuredValueAttribute(
    minInterval: Int,
    maxInterval: Int
  ): MeasuredValueAttribute {
    // Implementation needs to be added here
  }

  suspend fun readMinMeasuredValueAttribute(): MinMeasuredValueAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeMinMeasuredValueAttribute(
    minInterval: Int,
    maxInterval: Int
  ): MinMeasuredValueAttribute {
    // Implementation needs to be added here
  }

  suspend fun readMaxMeasuredValueAttribute(): MaxMeasuredValueAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeMaxMeasuredValueAttribute(
    minInterval: Int,
    maxInterval: Int
  ): MaxMeasuredValueAttribute {
    // Implementation needs to be added here
  }

  suspend fun readPeakMeasuredValueAttribute(): PeakMeasuredValueAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribePeakMeasuredValueAttribute(
    minInterval: Int,
    maxInterval: Int
  ): PeakMeasuredValueAttribute {
    // Implementation needs to be added here
  }

  suspend fun readPeakMeasuredValueWindowAttribute(): UInt {
    // Implementation needs to be added here
  }

  suspend fun subscribePeakMeasuredValueWindowAttribute(minInterval: Int, maxInterval: Int): UInt {
    // Implementation needs to be added here
  }

  suspend fun readAverageMeasuredValueAttribute(): AverageMeasuredValueAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeAverageMeasuredValueAttribute(
    minInterval: Int,
    maxInterval: Int
  ): AverageMeasuredValueAttribute {
    // Implementation needs to be added here
  }

  suspend fun readAverageMeasuredValueWindowAttribute(): UInt {
    // Implementation needs to be added here
  }

  suspend fun subscribeAverageMeasuredValueWindowAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UInt {
    // Implementation needs to be added here
  }

  suspend fun readUncertaintyAttribute(): Float {
    // Implementation needs to be added here
  }

  suspend fun subscribeUncertaintyAttribute(minInterval: Int, maxInterval: Int): Float {
    // Implementation needs to be added here
  }

  suspend fun readMeasurementUnitAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeMeasurementUnitAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readMeasurementMediumAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeMeasurementMediumAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readLevelValueAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeLevelValueAttribute(minInterval: Int, maxInterval: Int): UByte {
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
    const val CLUSTER_ID: UInt = 1069u
  }
}
