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

class EnergyPreferenceCluster(
  private val controller: MatterController,
  private val endpointId: UShort
) {
  class EnergyBalancesAttribute(val value: List<EnergyPreferenceClusterBalanceStruct>?)

  class EnergyPrioritiesAttribute(val value: List<UInt>?)

  class LowPowerModeSensitivitiesAttribute(val value: List<EnergyPreferenceClusterBalanceStruct>?)

  class GeneratedCommandListAttribute(val value: List<UInt>)

  class AcceptedCommandListAttribute(val value: List<UInt>)

  class EventListAttribute(val value: List<UInt>)

  class AttributeListAttribute(val value: List<UInt>)

  suspend fun readEnergyBalancesAttribute(): EnergyBalancesAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeEnergyBalancesAttribute(
    minInterval: Int,
    maxInterval: Int
  ): EnergyBalancesAttribute {
    // Implementation needs to be added here
  }

  suspend fun readCurrentEnergyBalanceAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun writeCurrentEnergyBalanceAttribute(value: UByte, timedWriteTimeoutMs: Int? = null) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
  }

  suspend fun subscribeCurrentEnergyBalanceAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readEnergyPrioritiesAttribute(): EnergyPrioritiesAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeEnergyPrioritiesAttribute(
    minInterval: Int,
    maxInterval: Int
  ): EnergyPrioritiesAttribute {
    // Implementation needs to be added here
  }

  suspend fun readLowPowerModeSensitivitiesAttribute(): LowPowerModeSensitivitiesAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeLowPowerModeSensitivitiesAttribute(
    minInterval: Int,
    maxInterval: Int
  ): LowPowerModeSensitivitiesAttribute {
    // Implementation needs to be added here
  }

  suspend fun readCurrentLowPowerModeSensitivityAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun writeCurrentLowPowerModeSensitivityAttribute(
    value: UByte,
    timedWriteTimeoutMs: Int? = null
  ) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
  }

  suspend fun subscribeCurrentLowPowerModeSensitivityAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UByte {
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
    const val CLUSTER_ID: UInt = 155u
  }
}
