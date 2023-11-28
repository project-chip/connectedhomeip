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

class ElectricalMeasurementCluster(
  private val controller: MatterController,
  private val endpointId: UShort
) {
  class GeneratedCommandListAttribute(val value: List<UInt>)

  class AcceptedCommandListAttribute(val value: List<UInt>)

  class EventListAttribute(val value: List<UInt>)

  class AttributeListAttribute(val value: List<UInt>)

  suspend fun getProfileInfoCommand(timedInvokeTimeoutMs: Int? = null) {
    val commandId = 0L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun getMeasurementProfileCommand(
    attributeId: UShort,
    startTime: UInt,
    numberOfIntervals: UInt,
    timedInvokeTimeoutMs: Int? = null
  ) {
    val commandId = 1L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun readMeasurementTypeAttribute(): UInt {
    // Implementation needs to be added here
  }

  suspend fun subscribeMeasurementTypeAttribute(minInterval: Int, maxInterval: Int): UInt {
    // Implementation needs to be added here
  }

  suspend fun readDcVoltageAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeDcVoltageAttribute(minInterval: Int, maxInterval: Int): Short {
    // Implementation needs to be added here
  }

  suspend fun readDcVoltageMinAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeDcVoltageMinAttribute(minInterval: Int, maxInterval: Int): Short {
    // Implementation needs to be added here
  }

  suspend fun readDcVoltageMaxAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeDcVoltageMaxAttribute(minInterval: Int, maxInterval: Int): Short {
    // Implementation needs to be added here
  }

  suspend fun readDcCurrentAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeDcCurrentAttribute(minInterval: Int, maxInterval: Int): Short {
    // Implementation needs to be added here
  }

  suspend fun readDcCurrentMinAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeDcCurrentMinAttribute(minInterval: Int, maxInterval: Int): Short {
    // Implementation needs to be added here
  }

  suspend fun readDcCurrentMaxAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeDcCurrentMaxAttribute(minInterval: Int, maxInterval: Int): Short {
    // Implementation needs to be added here
  }

  suspend fun readDcPowerAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeDcPowerAttribute(minInterval: Int, maxInterval: Int): Short {
    // Implementation needs to be added here
  }

  suspend fun readDcPowerMinAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeDcPowerMinAttribute(minInterval: Int, maxInterval: Int): Short {
    // Implementation needs to be added here
  }

  suspend fun readDcPowerMaxAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeDcPowerMaxAttribute(minInterval: Int, maxInterval: Int): Short {
    // Implementation needs to be added here
  }

  suspend fun readDcVoltageMultiplierAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeDcVoltageMultiplierAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readDcVoltageDivisorAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeDcVoltageDivisorAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readDcCurrentMultiplierAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeDcCurrentMultiplierAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readDcCurrentDivisorAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeDcCurrentDivisorAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readDcPowerMultiplierAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeDcPowerMultiplierAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readDcPowerDivisorAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeDcPowerDivisorAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readAcFrequencyAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeAcFrequencyAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readAcFrequencyMinAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeAcFrequencyMinAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readAcFrequencyMaxAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeAcFrequencyMaxAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readNeutralCurrentAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeNeutralCurrentAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readTotalActivePowerAttribute(): Int {
    // Implementation needs to be added here
  }

  suspend fun subscribeTotalActivePowerAttribute(minInterval: Int, maxInterval: Int): Int {
    // Implementation needs to be added here
  }

  suspend fun readTotalReactivePowerAttribute(): Int {
    // Implementation needs to be added here
  }

  suspend fun subscribeTotalReactivePowerAttribute(minInterval: Int, maxInterval: Int): Int {
    // Implementation needs to be added here
  }

  suspend fun readTotalApparentPowerAttribute(): UInt {
    // Implementation needs to be added here
  }

  suspend fun subscribeTotalApparentPowerAttribute(minInterval: Int, maxInterval: Int): UInt {
    // Implementation needs to be added here
  }

  suspend fun readMeasured1stHarmonicCurrentAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeMeasured1stHarmonicCurrentAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Short {
    // Implementation needs to be added here
  }

  suspend fun readMeasured3rdHarmonicCurrentAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeMeasured3rdHarmonicCurrentAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Short {
    // Implementation needs to be added here
  }

  suspend fun readMeasured5thHarmonicCurrentAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeMeasured5thHarmonicCurrentAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Short {
    // Implementation needs to be added here
  }

  suspend fun readMeasured7thHarmonicCurrentAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeMeasured7thHarmonicCurrentAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Short {
    // Implementation needs to be added here
  }

  suspend fun readMeasured9thHarmonicCurrentAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeMeasured9thHarmonicCurrentAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Short {
    // Implementation needs to be added here
  }

  suspend fun readMeasured11thHarmonicCurrentAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeMeasured11thHarmonicCurrentAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Short {
    // Implementation needs to be added here
  }

  suspend fun readMeasuredPhase1stHarmonicCurrentAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeMeasuredPhase1stHarmonicCurrentAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Short {
    // Implementation needs to be added here
  }

  suspend fun readMeasuredPhase3rdHarmonicCurrentAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeMeasuredPhase3rdHarmonicCurrentAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Short {
    // Implementation needs to be added here
  }

  suspend fun readMeasuredPhase5thHarmonicCurrentAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeMeasuredPhase5thHarmonicCurrentAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Short {
    // Implementation needs to be added here
  }

  suspend fun readMeasuredPhase7thHarmonicCurrentAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeMeasuredPhase7thHarmonicCurrentAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Short {
    // Implementation needs to be added here
  }

  suspend fun readMeasuredPhase9thHarmonicCurrentAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeMeasuredPhase9thHarmonicCurrentAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Short {
    // Implementation needs to be added here
  }

  suspend fun readMeasuredPhase11thHarmonicCurrentAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeMeasuredPhase11thHarmonicCurrentAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Short {
    // Implementation needs to be added here
  }

  suspend fun readAcFrequencyMultiplierAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeAcFrequencyMultiplierAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readAcFrequencyDivisorAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeAcFrequencyDivisorAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readPowerMultiplierAttribute(): UInt {
    // Implementation needs to be added here
  }

  suspend fun subscribePowerMultiplierAttribute(minInterval: Int, maxInterval: Int): UInt {
    // Implementation needs to be added here
  }

  suspend fun readPowerDivisorAttribute(): UInt {
    // Implementation needs to be added here
  }

  suspend fun subscribePowerDivisorAttribute(minInterval: Int, maxInterval: Int): UInt {
    // Implementation needs to be added here
  }

  suspend fun readHarmonicCurrentMultiplierAttribute(): Byte {
    // Implementation needs to be added here
  }

  suspend fun subscribeHarmonicCurrentMultiplierAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Byte {
    // Implementation needs to be added here
  }

  suspend fun readPhaseHarmonicCurrentMultiplierAttribute(): Byte {
    // Implementation needs to be added here
  }

  suspend fun subscribePhaseHarmonicCurrentMultiplierAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Byte {
    // Implementation needs to be added here
  }

  suspend fun readInstantaneousVoltageAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeInstantaneousVoltageAttribute(minInterval: Int, maxInterval: Int): Short {
    // Implementation needs to be added here
  }

  suspend fun readInstantaneousLineCurrentAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeInstantaneousLineCurrentAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UShort {
    // Implementation needs to be added here
  }

  suspend fun readInstantaneousActiveCurrentAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeInstantaneousActiveCurrentAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Short {
    // Implementation needs to be added here
  }

  suspend fun readInstantaneousReactiveCurrentAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeInstantaneousReactiveCurrentAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Short {
    // Implementation needs to be added here
  }

  suspend fun readInstantaneousPowerAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeInstantaneousPowerAttribute(minInterval: Int, maxInterval: Int): Short {
    // Implementation needs to be added here
  }

  suspend fun readRmsVoltageAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsVoltageAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readRmsVoltageMinAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsVoltageMinAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readRmsVoltageMaxAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsVoltageMaxAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readRmsCurrentAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsCurrentAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readRmsCurrentMinAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsCurrentMinAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readRmsCurrentMaxAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsCurrentMaxAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readActivePowerAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeActivePowerAttribute(minInterval: Int, maxInterval: Int): Short {
    // Implementation needs to be added here
  }

  suspend fun readActivePowerMinAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeActivePowerMinAttribute(minInterval: Int, maxInterval: Int): Short {
    // Implementation needs to be added here
  }

  suspend fun readActivePowerMaxAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeActivePowerMaxAttribute(minInterval: Int, maxInterval: Int): Short {
    // Implementation needs to be added here
  }

  suspend fun readReactivePowerAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeReactivePowerAttribute(minInterval: Int, maxInterval: Int): Short {
    // Implementation needs to be added here
  }

  suspend fun readApparentPowerAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeApparentPowerAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readPowerFactorAttribute(): Byte {
    // Implementation needs to be added here
  }

  suspend fun subscribePowerFactorAttribute(minInterval: Int, maxInterval: Int): Byte {
    // Implementation needs to be added here
  }

  suspend fun readAverageRmsVoltageMeasurementPeriodAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun writeAverageRmsVoltageMeasurementPeriodAttribute(
    value: UShort,
    timedWriteTimeoutMs: Int? = null
  ) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
  }

  suspend fun subscribeAverageRmsVoltageMeasurementPeriodAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UShort {
    // Implementation needs to be added here
  }

  suspend fun readAverageRmsUnderVoltageCounterAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun writeAverageRmsUnderVoltageCounterAttribute(
    value: UShort,
    timedWriteTimeoutMs: Int? = null
  ) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
  }

  suspend fun subscribeAverageRmsUnderVoltageCounterAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UShort {
    // Implementation needs to be added here
  }

  suspend fun readRmsExtremeOverVoltagePeriodAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun writeRmsExtremeOverVoltagePeriodAttribute(
    value: UShort,
    timedWriteTimeoutMs: Int? = null
  ) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
  }

  suspend fun subscribeRmsExtremeOverVoltagePeriodAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UShort {
    // Implementation needs to be added here
  }

  suspend fun readRmsExtremeUnderVoltagePeriodAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun writeRmsExtremeUnderVoltagePeriodAttribute(
    value: UShort,
    timedWriteTimeoutMs: Int? = null
  ) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
  }

  suspend fun subscribeRmsExtremeUnderVoltagePeriodAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UShort {
    // Implementation needs to be added here
  }

  suspend fun readRmsVoltageSagPeriodAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun writeRmsVoltageSagPeriodAttribute(value: UShort, timedWriteTimeoutMs: Int? = null) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
  }

  suspend fun subscribeRmsVoltageSagPeriodAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readRmsVoltageSwellPeriodAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun writeRmsVoltageSwellPeriodAttribute(value: UShort, timedWriteTimeoutMs: Int? = null) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
  }

  suspend fun subscribeRmsVoltageSwellPeriodAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readAcVoltageMultiplierAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeAcVoltageMultiplierAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readAcVoltageDivisorAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeAcVoltageDivisorAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readAcCurrentMultiplierAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeAcCurrentMultiplierAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readAcCurrentDivisorAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeAcCurrentDivisorAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readAcPowerMultiplierAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeAcPowerMultiplierAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readAcPowerDivisorAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeAcPowerDivisorAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readOverloadAlarmsMaskAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun writeOverloadAlarmsMaskAttribute(value: UInt, timedWriteTimeoutMs: Int? = null) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
  }

  suspend fun subscribeOverloadAlarmsMaskAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readVoltageOverloadAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeVoltageOverloadAttribute(minInterval: Int, maxInterval: Int): Short {
    // Implementation needs to be added here
  }

  suspend fun readCurrentOverloadAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeCurrentOverloadAttribute(minInterval: Int, maxInterval: Int): Short {
    // Implementation needs to be added here
  }

  suspend fun readAcOverloadAlarmsMaskAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun writeAcOverloadAlarmsMaskAttribute(value: UInt, timedWriteTimeoutMs: Int? = null) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
  }

  suspend fun subscribeAcOverloadAlarmsMaskAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readAcVoltageOverloadAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeAcVoltageOverloadAttribute(minInterval: Int, maxInterval: Int): Short {
    // Implementation needs to be added here
  }

  suspend fun readAcCurrentOverloadAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeAcCurrentOverloadAttribute(minInterval: Int, maxInterval: Int): Short {
    // Implementation needs to be added here
  }

  suspend fun readAcActivePowerOverloadAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeAcActivePowerOverloadAttribute(minInterval: Int, maxInterval: Int): Short {
    // Implementation needs to be added here
  }

  suspend fun readAcReactivePowerOverloadAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeAcReactivePowerOverloadAttribute(minInterval: Int, maxInterval: Int): Short {
    // Implementation needs to be added here
  }

  suspend fun readAverageRmsOverVoltageAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeAverageRmsOverVoltageAttribute(minInterval: Int, maxInterval: Int): Short {
    // Implementation needs to be added here
  }

  suspend fun readAverageRmsUnderVoltageAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeAverageRmsUnderVoltageAttribute(minInterval: Int, maxInterval: Int): Short {
    // Implementation needs to be added here
  }

  suspend fun readRmsExtremeOverVoltageAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsExtremeOverVoltageAttribute(minInterval: Int, maxInterval: Int): Short {
    // Implementation needs to be added here
  }

  suspend fun readRmsExtremeUnderVoltageAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsExtremeUnderVoltageAttribute(minInterval: Int, maxInterval: Int): Short {
    // Implementation needs to be added here
  }

  suspend fun readRmsVoltageSagAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsVoltageSagAttribute(minInterval: Int, maxInterval: Int): Short {
    // Implementation needs to be added here
  }

  suspend fun readRmsVoltageSwellAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsVoltageSwellAttribute(minInterval: Int, maxInterval: Int): Short {
    // Implementation needs to be added here
  }

  suspend fun readLineCurrentPhaseBAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeLineCurrentPhaseBAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readActiveCurrentPhaseBAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeActiveCurrentPhaseBAttribute(minInterval: Int, maxInterval: Int): Short {
    // Implementation needs to be added here
  }

  suspend fun readReactiveCurrentPhaseBAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeReactiveCurrentPhaseBAttribute(minInterval: Int, maxInterval: Int): Short {
    // Implementation needs to be added here
  }

  suspend fun readRmsVoltagePhaseBAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsVoltagePhaseBAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readRmsVoltageMinPhaseBAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsVoltageMinPhaseBAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readRmsVoltageMaxPhaseBAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsVoltageMaxPhaseBAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readRmsCurrentPhaseBAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsCurrentPhaseBAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readRmsCurrentMinPhaseBAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsCurrentMinPhaseBAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readRmsCurrentMaxPhaseBAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsCurrentMaxPhaseBAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readActivePowerPhaseBAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeActivePowerPhaseBAttribute(minInterval: Int, maxInterval: Int): Short {
    // Implementation needs to be added here
  }

  suspend fun readActivePowerMinPhaseBAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeActivePowerMinPhaseBAttribute(minInterval: Int, maxInterval: Int): Short {
    // Implementation needs to be added here
  }

  suspend fun readActivePowerMaxPhaseBAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeActivePowerMaxPhaseBAttribute(minInterval: Int, maxInterval: Int): Short {
    // Implementation needs to be added here
  }

  suspend fun readReactivePowerPhaseBAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeReactivePowerPhaseBAttribute(minInterval: Int, maxInterval: Int): Short {
    // Implementation needs to be added here
  }

  suspend fun readApparentPowerPhaseBAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeApparentPowerPhaseBAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readPowerFactorPhaseBAttribute(): Byte {
    // Implementation needs to be added here
  }

  suspend fun subscribePowerFactorPhaseBAttribute(minInterval: Int, maxInterval: Int): Byte {
    // Implementation needs to be added here
  }

  suspend fun readAverageRmsVoltageMeasurementPeriodPhaseBAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeAverageRmsVoltageMeasurementPeriodPhaseBAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UShort {
    // Implementation needs to be added here
  }

  suspend fun readAverageRmsOverVoltageCounterPhaseBAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeAverageRmsOverVoltageCounterPhaseBAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UShort {
    // Implementation needs to be added here
  }

  suspend fun readAverageRmsUnderVoltageCounterPhaseBAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeAverageRmsUnderVoltageCounterPhaseBAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UShort {
    // Implementation needs to be added here
  }

  suspend fun readRmsExtremeOverVoltagePeriodPhaseBAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsExtremeOverVoltagePeriodPhaseBAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UShort {
    // Implementation needs to be added here
  }

  suspend fun readRmsExtremeUnderVoltagePeriodPhaseBAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsExtremeUnderVoltagePeriodPhaseBAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UShort {
    // Implementation needs to be added here
  }

  suspend fun readRmsVoltageSagPeriodPhaseBAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsVoltageSagPeriodPhaseBAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UShort {
    // Implementation needs to be added here
  }

  suspend fun readRmsVoltageSwellPeriodPhaseBAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsVoltageSwellPeriodPhaseBAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UShort {
    // Implementation needs to be added here
  }

  suspend fun readLineCurrentPhaseCAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeLineCurrentPhaseCAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readActiveCurrentPhaseCAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeActiveCurrentPhaseCAttribute(minInterval: Int, maxInterval: Int): Short {
    // Implementation needs to be added here
  }

  suspend fun readReactiveCurrentPhaseCAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeReactiveCurrentPhaseCAttribute(minInterval: Int, maxInterval: Int): Short {
    // Implementation needs to be added here
  }

  suspend fun readRmsVoltagePhaseCAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsVoltagePhaseCAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readRmsVoltageMinPhaseCAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsVoltageMinPhaseCAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readRmsVoltageMaxPhaseCAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsVoltageMaxPhaseCAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readRmsCurrentPhaseCAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsCurrentPhaseCAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readRmsCurrentMinPhaseCAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsCurrentMinPhaseCAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readRmsCurrentMaxPhaseCAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsCurrentMaxPhaseCAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readActivePowerPhaseCAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeActivePowerPhaseCAttribute(minInterval: Int, maxInterval: Int): Short {
    // Implementation needs to be added here
  }

  suspend fun readActivePowerMinPhaseCAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeActivePowerMinPhaseCAttribute(minInterval: Int, maxInterval: Int): Short {
    // Implementation needs to be added here
  }

  suspend fun readActivePowerMaxPhaseCAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeActivePowerMaxPhaseCAttribute(minInterval: Int, maxInterval: Int): Short {
    // Implementation needs to be added here
  }

  suspend fun readReactivePowerPhaseCAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeReactivePowerPhaseCAttribute(minInterval: Int, maxInterval: Int): Short {
    // Implementation needs to be added here
  }

  suspend fun readApparentPowerPhaseCAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeApparentPowerPhaseCAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readPowerFactorPhaseCAttribute(): Byte {
    // Implementation needs to be added here
  }

  suspend fun subscribePowerFactorPhaseCAttribute(minInterval: Int, maxInterval: Int): Byte {
    // Implementation needs to be added here
  }

  suspend fun readAverageRmsVoltageMeasurementPeriodPhaseCAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeAverageRmsVoltageMeasurementPeriodPhaseCAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UShort {
    // Implementation needs to be added here
  }

  suspend fun readAverageRmsOverVoltageCounterPhaseCAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeAverageRmsOverVoltageCounterPhaseCAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UShort {
    // Implementation needs to be added here
  }

  suspend fun readAverageRmsUnderVoltageCounterPhaseCAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeAverageRmsUnderVoltageCounterPhaseCAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UShort {
    // Implementation needs to be added here
  }

  suspend fun readRmsExtremeOverVoltagePeriodPhaseCAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsExtremeOverVoltagePeriodPhaseCAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UShort {
    // Implementation needs to be added here
  }

  suspend fun readRmsExtremeUnderVoltagePeriodPhaseCAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsExtremeUnderVoltagePeriodPhaseCAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UShort {
    // Implementation needs to be added here
  }

  suspend fun readRmsVoltageSagPeriodPhaseCAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsVoltageSagPeriodPhaseCAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UShort {
    // Implementation needs to be added here
  }

  suspend fun readRmsVoltageSwellPeriodPhaseCAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsVoltageSwellPeriodPhaseCAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UShort {
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
    const val CLUSTER_ID: UInt = 2820u
  }
}
