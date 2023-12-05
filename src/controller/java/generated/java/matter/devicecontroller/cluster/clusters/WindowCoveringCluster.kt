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

class WindowCoveringCluster(
  private val controller: MatterController,
  private val endpointId: UShort
) {
  class CurrentPositionLiftAttribute(val value: UShort?)

  class CurrentPositionTiltAttribute(val value: UShort?)

  class CurrentPositionLiftPercentageAttribute(val value: UByte?)

  class CurrentPositionTiltPercentageAttribute(val value: UByte?)

  class TargetPositionLiftPercent100thsAttribute(val value: UShort?)

  class TargetPositionTiltPercent100thsAttribute(val value: UShort?)

  class CurrentPositionLiftPercent100thsAttribute(val value: UShort?)

  class CurrentPositionTiltPercent100thsAttribute(val value: UShort?)

  class GeneratedCommandListAttribute(val value: List<UInt>)

  class AcceptedCommandListAttribute(val value: List<UInt>)

  class EventListAttribute(val value: List<UInt>)

  class AttributeListAttribute(val value: List<UInt>)

  suspend fun upOrOpen(timedInvokeTimeoutMs: Int? = null) {
    val commandId = 0L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun downOrClose(timedInvokeTimeoutMs: Int? = null) {
    val commandId = 1L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun stopMotion(timedInvokeTimeoutMs: Int? = null) {
    val commandId = 2L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun goToLiftValue(liftValue: UShort, timedInvokeTimeoutMs: Int? = null) {
    val commandId = 4L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun goToLiftPercentage(
    liftPercent100thsValue: UShort,
    timedInvokeTimeoutMs: Int? = null
  ) {
    val commandId = 5L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun goToTiltValue(tiltValue: UShort, timedInvokeTimeoutMs: Int? = null) {
    val commandId = 7L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun goToTiltPercentage(
    tiltPercent100thsValue: UShort,
    timedInvokeTimeoutMs: Int? = null
  ) {
    val commandId = 8L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun readTypeAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeTypeAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readPhysicalClosedLimitLiftAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribePhysicalClosedLimitLiftAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UShort {
    // Implementation needs to be added here
  }

  suspend fun readPhysicalClosedLimitTiltAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribePhysicalClosedLimitTiltAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UShort {
    // Implementation needs to be added here
  }

  suspend fun readCurrentPositionLiftAttribute(): CurrentPositionLiftAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeCurrentPositionLiftAttribute(
    minInterval: Int,
    maxInterval: Int
  ): CurrentPositionLiftAttribute {
    // Implementation needs to be added here
  }

  suspend fun readCurrentPositionTiltAttribute(): CurrentPositionTiltAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeCurrentPositionTiltAttribute(
    minInterval: Int,
    maxInterval: Int
  ): CurrentPositionTiltAttribute {
    // Implementation needs to be added here
  }

  suspend fun readNumberOfActuationsLiftAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeNumberOfActuationsLiftAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readNumberOfActuationsTiltAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeNumberOfActuationsTiltAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readConfigStatusAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeConfigStatusAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readCurrentPositionLiftPercentageAttribute(): CurrentPositionLiftPercentageAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeCurrentPositionLiftPercentageAttribute(
    minInterval: Int,
    maxInterval: Int
  ): CurrentPositionLiftPercentageAttribute {
    // Implementation needs to be added here
  }

  suspend fun readCurrentPositionTiltPercentageAttribute(): CurrentPositionTiltPercentageAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeCurrentPositionTiltPercentageAttribute(
    minInterval: Int,
    maxInterval: Int
  ): CurrentPositionTiltPercentageAttribute {
    // Implementation needs to be added here
  }

  suspend fun readOperationalStatusAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeOperationalStatusAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readTargetPositionLiftPercent100thsAttribute():
    TargetPositionLiftPercent100thsAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeTargetPositionLiftPercent100thsAttribute(
    minInterval: Int,
    maxInterval: Int
  ): TargetPositionLiftPercent100thsAttribute {
    // Implementation needs to be added here
  }

  suspend fun readTargetPositionTiltPercent100thsAttribute():
    TargetPositionTiltPercent100thsAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeTargetPositionTiltPercent100thsAttribute(
    minInterval: Int,
    maxInterval: Int
  ): TargetPositionTiltPercent100thsAttribute {
    // Implementation needs to be added here
  }

  suspend fun readEndProductTypeAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeEndProductTypeAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readCurrentPositionLiftPercent100thsAttribute():
    CurrentPositionLiftPercent100thsAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeCurrentPositionLiftPercent100thsAttribute(
    minInterval: Int,
    maxInterval: Int
  ): CurrentPositionLiftPercent100thsAttribute {
    // Implementation needs to be added here
  }

  suspend fun readCurrentPositionTiltPercent100thsAttribute():
    CurrentPositionTiltPercent100thsAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeCurrentPositionTiltPercent100thsAttribute(
    minInterval: Int,
    maxInterval: Int
  ): CurrentPositionTiltPercent100thsAttribute {
    // Implementation needs to be added here
  }

  suspend fun readInstalledOpenLimitLiftAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeInstalledOpenLimitLiftAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readInstalledClosedLimitLiftAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeInstalledClosedLimitLiftAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UShort {
    // Implementation needs to be added here
  }

  suspend fun readInstalledOpenLimitTiltAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeInstalledOpenLimitTiltAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readInstalledClosedLimitTiltAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeInstalledClosedLimitTiltAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UShort {
    // Implementation needs to be added here
  }

  suspend fun readModeAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun writeModeAttribute(value: UInt, timedWriteTimeoutMs: Int? = null) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
  }

  suspend fun subscribeModeAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readSafetyStatusAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeSafetyStatusAttribute(minInterval: Int, maxInterval: Int): UShort {
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
    const val CLUSTER_ID: UInt = 258u
  }
}
