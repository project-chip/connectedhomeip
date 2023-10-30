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

class WindowCoveringCluster(private val endpointId: UShort) {
  class CurrentPositionLiftAttribute(val value: UShort?)

  class CurrentPositionTiltAttribute(val value: UShort?)

  class CurrentPositionLiftPercentageAttribute(val value: UByte?)

  class CurrentPositionTiltPercentageAttribute(val value: UByte?)

  class TargetPositionLiftPercent100thsAttribute(val value: UShort?)

  class TargetPositionTiltPercent100thsAttribute(val value: UShort?)

  class CurrentPositionLiftPercent100thsAttribute(val value: UShort?)

  class CurrentPositionTiltPercent100thsAttribute(val value: UShort?)

  class GeneratedCommandListAttribute(val value: ArrayList<UInt>)

  class AcceptedCommandListAttribute(val value: ArrayList<UInt>)

  class EventListAttribute(val value: ArrayList<UInt>)

  class AttributeListAttribute(val value: ArrayList<UInt>)

  suspend fun upOrOpen() {
    // Implementation needs to be added here
  }

  suspend fun upOrOpen(timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun downOrClose() {
    // Implementation needs to be added here
  }

  suspend fun downOrClose(timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun stopMotion() {
    // Implementation needs to be added here
  }

  suspend fun stopMotion(timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun goToLiftValue(liftValue: UShort) {
    // Implementation needs to be added here
  }

  suspend fun goToLiftValue(liftValue: UShort, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun goToLiftPercentage(liftPercent100thsValue: UShort) {
    // Implementation needs to be added here
  }

  suspend fun goToLiftPercentage(liftPercent100thsValue: UShort, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun goToTiltValue(tiltValue: UShort) {
    // Implementation needs to be added here
  }

  suspend fun goToTiltValue(tiltValue: UShort, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun goToTiltPercentage(tiltPercent100thsValue: UShort) {
    // Implementation needs to be added here
  }

  suspend fun goToTiltPercentage(tiltPercent100thsValue: UShort, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun readTypeAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeTypeAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readPhysicalClosedLimitLiftAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribePhysicalClosedLimitLiftAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readPhysicalClosedLimitTiltAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribePhysicalClosedLimitTiltAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
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

  suspend fun readNumberOfActuationsLiftAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeNumberOfActuationsLiftAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readNumberOfActuationsTiltAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeNumberOfActuationsTiltAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readConfigStatusAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeConfigStatusAttribute(minInterval: Int, maxInterval: Int): Integer {
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

  suspend fun readOperationalStatusAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeOperationalStatusAttribute(minInterval: Int, maxInterval: Int): Integer {
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

  suspend fun readEndProductTypeAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeEndProductTypeAttribute(minInterval: Int, maxInterval: Int): Integer {
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

  suspend fun readInstalledOpenLimitLiftAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeInstalledOpenLimitLiftAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readInstalledClosedLimitLiftAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeInstalledClosedLimitLiftAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readInstalledOpenLimitTiltAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeInstalledOpenLimitTiltAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readInstalledClosedLimitTiltAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeInstalledClosedLimitTiltAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readModeAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun writeModeAttribute(value: UInt) {
    // Implementation needs to be added here
  }

  suspend fun writeModeAttribute(value: UInt, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeModeAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readSafetyStatusAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeSafetyStatusAttribute(minInterval: Int, maxInterval: Int): Integer {
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
    const val CLUSTER_ID: UInt = 258u
  }
}
