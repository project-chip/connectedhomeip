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

class ElectricalEnergyMeasurementCluster(
  private val controller: MatterController,
  private val endpointId: UShort
) {
  class AccuracyAttribute(val value: ElectricalEnergyMeasurementClusterMeasurementAccuracyStruct)

  class CumulativeEnergyImportedAttribute(
    val value: ElectricalEnergyMeasurementClusterEnergyMeasurementStruct?
  )

  class CumulativeEnergyExportedAttribute(
    val value: ElectricalEnergyMeasurementClusterEnergyMeasurementStruct?
  )

  class PeriodicEnergyImportedAttribute(
    val value: ElectricalEnergyMeasurementClusterEnergyMeasurementStruct?
  )

  class PeriodicEnergyExportedAttribute(
    val value: ElectricalEnergyMeasurementClusterEnergyMeasurementStruct?
  )

  class GeneratedCommandListAttribute(val value: List<UInt>)

  class AcceptedCommandListAttribute(val value: List<UInt>)

  class EventListAttribute(val value: List<UInt>)

  class AttributeListAttribute(val value: List<UInt>)

  suspend fun readMeasuredAttribute(): Boolean {
    // Implementation needs to be added here
  }

  suspend fun subscribeMeasuredAttribute(minInterval: Int, maxInterval: Int): Boolean {
    // Implementation needs to be added here
  }

  suspend fun readAccuracyAttribute(): AccuracyAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeAccuracyAttribute(minInterval: Int, maxInterval: Int): AccuracyAttribute {
    // Implementation needs to be added here
  }

  suspend fun readCumulativeEnergyImportedAttribute(): CumulativeEnergyImportedAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeCumulativeEnergyImportedAttribute(
    minInterval: Int,
    maxInterval: Int
  ): CumulativeEnergyImportedAttribute {
    // Implementation needs to be added here
  }

  suspend fun readCumulativeEnergyExportedAttribute(): CumulativeEnergyExportedAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeCumulativeEnergyExportedAttribute(
    minInterval: Int,
    maxInterval: Int
  ): CumulativeEnergyExportedAttribute {
    // Implementation needs to be added here
  }

  suspend fun readPeriodicEnergyImportedAttribute(): PeriodicEnergyImportedAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribePeriodicEnergyImportedAttribute(
    minInterval: Int,
    maxInterval: Int
  ): PeriodicEnergyImportedAttribute {
    // Implementation needs to be added here
  }

  suspend fun readPeriodicEnergyExportedAttribute(): PeriodicEnergyExportedAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribePeriodicEnergyExportedAttribute(
    minInterval: Int,
    maxInterval: Int
  ): PeriodicEnergyExportedAttribute {
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
    const val CLUSTER_ID: UInt = 145u
  }
}
