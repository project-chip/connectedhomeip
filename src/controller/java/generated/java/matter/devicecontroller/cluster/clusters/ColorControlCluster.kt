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

class ColorControlCluster(private val endpointId: UShort) {
  companion object {
    const val CLUSTER_ID: UInt = 768u
  }

  fun moveToHue(
    callback: DefaultClusterCallback,
    hue: Integer,
    direction: Integer,
    transitionTime: Integer,
    optionsMask: Integer,
    optionsOverride: Integer
  ) {
    // Implementation needs to be added here
  }

  fun moveToHue(
    callback: DefaultClusterCallback,
    hue: Integer,
    direction: Integer,
    transitionTime: Integer,
    optionsMask: Integer,
    optionsOverride: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun moveHue(
    callback: DefaultClusterCallback,
    moveMode: Integer,
    rate: Integer,
    optionsMask: Integer,
    optionsOverride: Integer
  ) {
    // Implementation needs to be added here
  }

  fun moveHue(
    callback: DefaultClusterCallback,
    moveMode: Integer,
    rate: Integer,
    optionsMask: Integer,
    optionsOverride: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun stepHue(
    callback: DefaultClusterCallback,
    stepMode: Integer,
    stepSize: Integer,
    transitionTime: Integer,
    optionsMask: Integer,
    optionsOverride: Integer
  ) {
    // Implementation needs to be added here
  }

  fun stepHue(
    callback: DefaultClusterCallback,
    stepMode: Integer,
    stepSize: Integer,
    transitionTime: Integer,
    optionsMask: Integer,
    optionsOverride: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun moveToSaturation(
    callback: DefaultClusterCallback,
    saturation: Integer,
    transitionTime: Integer,
    optionsMask: Integer,
    optionsOverride: Integer
  ) {
    // Implementation needs to be added here
  }

  fun moveToSaturation(
    callback: DefaultClusterCallback,
    saturation: Integer,
    transitionTime: Integer,
    optionsMask: Integer,
    optionsOverride: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun moveSaturation(
    callback: DefaultClusterCallback,
    moveMode: Integer,
    rate: Integer,
    optionsMask: Integer,
    optionsOverride: Integer
  ) {
    // Implementation needs to be added here
  }

  fun moveSaturation(
    callback: DefaultClusterCallback,
    moveMode: Integer,
    rate: Integer,
    optionsMask: Integer,
    optionsOverride: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun stepSaturation(
    callback: DefaultClusterCallback,
    stepMode: Integer,
    stepSize: Integer,
    transitionTime: Integer,
    optionsMask: Integer,
    optionsOverride: Integer
  ) {
    // Implementation needs to be added here
  }

  fun stepSaturation(
    callback: DefaultClusterCallback,
    stepMode: Integer,
    stepSize: Integer,
    transitionTime: Integer,
    optionsMask: Integer,
    optionsOverride: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun moveToHueAndSaturation(
    callback: DefaultClusterCallback,
    hue: Integer,
    saturation: Integer,
    transitionTime: Integer,
    optionsMask: Integer,
    optionsOverride: Integer
  ) {
    // Implementation needs to be added here
  }

  fun moveToHueAndSaturation(
    callback: DefaultClusterCallback,
    hue: Integer,
    saturation: Integer,
    transitionTime: Integer,
    optionsMask: Integer,
    optionsOverride: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun moveToColor(
    callback: DefaultClusterCallback,
    colorX: Integer,
    colorY: Integer,
    transitionTime: Integer,
    optionsMask: Integer,
    optionsOverride: Integer
  ) {
    // Implementation needs to be added here
  }

  fun moveToColor(
    callback: DefaultClusterCallback,
    colorX: Integer,
    colorY: Integer,
    transitionTime: Integer,
    optionsMask: Integer,
    optionsOverride: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun moveColor(
    callback: DefaultClusterCallback,
    rateX: Integer,
    rateY: Integer,
    optionsMask: Integer,
    optionsOverride: Integer
  ) {
    // Implementation needs to be added here
  }

  fun moveColor(
    callback: DefaultClusterCallback,
    rateX: Integer,
    rateY: Integer,
    optionsMask: Integer,
    optionsOverride: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun stepColor(
    callback: DefaultClusterCallback,
    stepX: Integer,
    stepY: Integer,
    transitionTime: Integer,
    optionsMask: Integer,
    optionsOverride: Integer
  ) {
    // Implementation needs to be added here
  }

  fun stepColor(
    callback: DefaultClusterCallback,
    stepX: Integer,
    stepY: Integer,
    transitionTime: Integer,
    optionsMask: Integer,
    optionsOverride: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun moveToColorTemperature(
    callback: DefaultClusterCallback,
    colorTemperatureMireds: Integer,
    transitionTime: Integer,
    optionsMask: Integer,
    optionsOverride: Integer
  ) {
    // Implementation needs to be added here
  }

  fun moveToColorTemperature(
    callback: DefaultClusterCallback,
    colorTemperatureMireds: Integer,
    transitionTime: Integer,
    optionsMask: Integer,
    optionsOverride: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun enhancedMoveToHue(
    callback: DefaultClusterCallback,
    enhancedHue: Integer,
    direction: Integer,
    transitionTime: Integer,
    optionsMask: Integer,
    optionsOverride: Integer
  ) {
    // Implementation needs to be added here
  }

  fun enhancedMoveToHue(
    callback: DefaultClusterCallback,
    enhancedHue: Integer,
    direction: Integer,
    transitionTime: Integer,
    optionsMask: Integer,
    optionsOverride: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun enhancedMoveHue(
    callback: DefaultClusterCallback,
    moveMode: Integer,
    rate: Integer,
    optionsMask: Integer,
    optionsOverride: Integer
  ) {
    // Implementation needs to be added here
  }

  fun enhancedMoveHue(
    callback: DefaultClusterCallback,
    moveMode: Integer,
    rate: Integer,
    optionsMask: Integer,
    optionsOverride: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun enhancedStepHue(
    callback: DefaultClusterCallback,
    stepMode: Integer,
    stepSize: Integer,
    transitionTime: Integer,
    optionsMask: Integer,
    optionsOverride: Integer
  ) {
    // Implementation needs to be added here
  }

  fun enhancedStepHue(
    callback: DefaultClusterCallback,
    stepMode: Integer,
    stepSize: Integer,
    transitionTime: Integer,
    optionsMask: Integer,
    optionsOverride: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun enhancedMoveToHueAndSaturation(
    callback: DefaultClusterCallback,
    enhancedHue: Integer,
    saturation: Integer,
    transitionTime: Integer,
    optionsMask: Integer,
    optionsOverride: Integer
  ) {
    // Implementation needs to be added here
  }

  fun enhancedMoveToHueAndSaturation(
    callback: DefaultClusterCallback,
    enhancedHue: Integer,
    saturation: Integer,
    transitionTime: Integer,
    optionsMask: Integer,
    optionsOverride: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun colorLoopSet(
    callback: DefaultClusterCallback,
    updateFlags: Integer,
    action: Integer,
    direction: Integer,
    time: Integer,
    startHue: Integer,
    optionsMask: Integer,
    optionsOverride: Integer
  ) {
    // Implementation needs to be added here
  }

  fun colorLoopSet(
    callback: DefaultClusterCallback,
    updateFlags: Integer,
    action: Integer,
    direction: Integer,
    time: Integer,
    startHue: Integer,
    optionsMask: Integer,
    optionsOverride: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun stopMoveStep(
    callback: DefaultClusterCallback,
    optionsMask: Integer,
    optionsOverride: Integer
  ) {
    // Implementation needs to be added here
  }

  fun stopMoveStep(
    callback: DefaultClusterCallback,
    optionsMask: Integer,
    optionsOverride: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun moveColorTemperature(
    callback: DefaultClusterCallback,
    moveMode: Integer,
    rate: Integer,
    colorTemperatureMinimumMireds: Integer,
    colorTemperatureMaximumMireds: Integer,
    optionsMask: Integer,
    optionsOverride: Integer
  ) {
    // Implementation needs to be added here
  }

  fun moveColorTemperature(
    callback: DefaultClusterCallback,
    moveMode: Integer,
    rate: Integer,
    colorTemperatureMinimumMireds: Integer,
    colorTemperatureMaximumMireds: Integer,
    optionsMask: Integer,
    optionsOverride: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun stepColorTemperature(
    callback: DefaultClusterCallback,
    stepMode: Integer,
    stepSize: Integer,
    transitionTime: Integer,
    colorTemperatureMinimumMireds: Integer,
    colorTemperatureMaximumMireds: Integer,
    optionsMask: Integer,
    optionsOverride: Integer
  ) {
    // Implementation needs to be added here
  }

  fun stepColorTemperature(
    callback: DefaultClusterCallback,
    stepMode: Integer,
    stepSize: Integer,
    transitionTime: Integer,
    colorTemperatureMinimumMireds: Integer,
    colorTemperatureMaximumMireds: Integer,
    optionsMask: Integer,
    optionsOverride: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  interface NumberOfPrimariesAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface Primary1IntensityAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface Primary2IntensityAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface Primary3IntensityAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface Primary4IntensityAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface Primary5IntensityAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface Primary6IntensityAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface ColorPointRIntensityAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface ColorPointGIntensityAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface ColorPointBIntensityAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface StartUpColorTemperatureMiredsAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface GeneratedCommandListAttributeCallback {
    fun onSuccess(value: ArrayList<Long>)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface AcceptedCommandListAttributeCallback {
    fun onSuccess(value: ArrayList<Long>)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface EventListAttributeCallback {
    fun onSuccess(value: ArrayList<Long>)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface AttributeListAttributeCallback {
    fun onSuccess(value: ArrayList<Long>)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  fun readCurrentHueAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeCurrentHueAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readCurrentSaturationAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeCurrentSaturationAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readRemainingTimeAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeRemainingTimeAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readCurrentXAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeCurrentXAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readCurrentYAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeCurrentYAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readDriftCompensationAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeDriftCompensationAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readCompensationTextAttribute(callback: CharStringAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeCompensationTextAttribute(
    callback: CharStringAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readColorTemperatureMiredsAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeColorTemperatureMiredsAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readColorModeAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeColorModeAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readOptionsAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeOptionsAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeOptionsAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeOptionsAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNumberOfPrimariesAttribute(callback: NumberOfPrimariesAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeNumberOfPrimariesAttribute(
    callback: NumberOfPrimariesAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readPrimary1XAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribePrimary1XAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readPrimary1YAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribePrimary1YAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readPrimary1IntensityAttribute(callback: Primary1IntensityAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribePrimary1IntensityAttribute(
    callback: Primary1IntensityAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readPrimary2XAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribePrimary2XAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readPrimary2YAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribePrimary2YAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readPrimary2IntensityAttribute(callback: Primary2IntensityAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribePrimary2IntensityAttribute(
    callback: Primary2IntensityAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readPrimary3XAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribePrimary3XAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readPrimary3YAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribePrimary3YAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readPrimary3IntensityAttribute(callback: Primary3IntensityAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribePrimary3IntensityAttribute(
    callback: Primary3IntensityAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readPrimary4XAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribePrimary4XAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readPrimary4YAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribePrimary4YAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readPrimary4IntensityAttribute(callback: Primary4IntensityAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribePrimary4IntensityAttribute(
    callback: Primary4IntensityAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readPrimary5XAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribePrimary5XAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readPrimary5YAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribePrimary5YAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readPrimary5IntensityAttribute(callback: Primary5IntensityAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribePrimary5IntensityAttribute(
    callback: Primary5IntensityAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readPrimary6XAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribePrimary6XAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readPrimary6YAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribePrimary6YAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readPrimary6IntensityAttribute(callback: Primary6IntensityAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribePrimary6IntensityAttribute(
    callback: Primary6IntensityAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readWhitePointXAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeWhitePointXAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeWhitePointXAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeWhitePointXAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readWhitePointYAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeWhitePointYAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeWhitePointYAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeWhitePointYAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readColorPointRXAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeColorPointRXAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeColorPointRXAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeColorPointRXAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readColorPointRYAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeColorPointRYAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeColorPointRYAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeColorPointRYAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readColorPointRIntensityAttribute(callback: ColorPointRIntensityAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeColorPointRIntensityAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeColorPointRIntensityAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeColorPointRIntensityAttribute(
    callback: ColorPointRIntensityAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readColorPointGXAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeColorPointGXAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeColorPointGXAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeColorPointGXAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readColorPointGYAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeColorPointGYAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeColorPointGYAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeColorPointGYAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readColorPointGIntensityAttribute(callback: ColorPointGIntensityAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeColorPointGIntensityAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeColorPointGIntensityAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeColorPointGIntensityAttribute(
    callback: ColorPointGIntensityAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readColorPointBXAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeColorPointBXAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeColorPointBXAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeColorPointBXAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readColorPointBYAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeColorPointBYAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeColorPointBYAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeColorPointBYAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readColorPointBIntensityAttribute(callback: ColorPointBIntensityAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeColorPointBIntensityAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeColorPointBIntensityAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeColorPointBIntensityAttribute(
    callback: ColorPointBIntensityAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readEnhancedCurrentHueAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeEnhancedCurrentHueAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readEnhancedColorModeAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeEnhancedColorModeAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readColorLoopActiveAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeColorLoopActiveAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readColorLoopDirectionAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeColorLoopDirectionAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readColorLoopTimeAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeColorLoopTimeAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readColorLoopStartEnhancedHueAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeColorLoopStartEnhancedHueAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readColorLoopStoredEnhancedHueAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeColorLoopStoredEnhancedHueAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readColorCapabilitiesAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeColorCapabilitiesAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readColorTempPhysicalMinMiredsAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeColorTempPhysicalMinMiredsAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readColorTempPhysicalMaxMiredsAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeColorTempPhysicalMaxMiredsAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readCoupleColorTempToLevelMinMiredsAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeCoupleColorTempToLevelMinMiredsAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readStartUpColorTemperatureMiredsAttribute(
    callback: StartUpColorTemperatureMiredsAttributeCallback
  ) {
    // Implementation needs to be added here
  }

  fun writeStartUpColorTemperatureMiredsAttribute(
    callback: DefaultClusterCallback,
    value: Integer
  ) {
    // Implementation needs to be added here
  }

  fun writeStartUpColorTemperatureMiredsAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeStartUpColorTemperatureMiredsAttribute(
    callback: StartUpColorTemperatureMiredsAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readGeneratedCommandListAttribute(callback: GeneratedCommandListAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeGeneratedCommandListAttribute(
    callback: GeneratedCommandListAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readAcceptedCommandListAttribute(callback: AcceptedCommandListAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeAcceptedCommandListAttribute(
    callback: AcceptedCommandListAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readEventListAttribute(callback: EventListAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeEventListAttribute(
    callback: EventListAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readAttributeListAttribute(callback: AttributeListAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeAttributeListAttribute(
    callback: AttributeListAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readFeatureMapAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeFeatureMapAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readClusterRevisionAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeClusterRevisionAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }
}
