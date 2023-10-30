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

class ElectricalMeasurementCluster(private val endpointId: UShort) {
  class GeneratedCommandListAttribute(val value: ArrayList<UInt>)

  class AcceptedCommandListAttribute(val value: ArrayList<UInt>)

  class EventListAttribute(val value: ArrayList<UInt>)

  class AttributeListAttribute(val value: ArrayList<UInt>)

  suspend fun getProfileInfoCommand() {
    // Implementation needs to be added here
  }

  suspend fun getProfileInfoCommand(timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun getMeasurementProfileCommand(
    attributeId: UShort,
    startTime: UInt,
    numberOfIntervals: UInt
  ) {
    // Implementation needs to be added here
  }

  suspend fun getMeasurementProfileCommand(
    attributeId: UShort,
    startTime: UInt,
    numberOfIntervals: UInt,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  suspend fun readMeasurementTypeAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeMeasurementTypeAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readDcVoltageAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeDcVoltageAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readDcVoltageMinAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeDcVoltageMinAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readDcVoltageMaxAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeDcVoltageMaxAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readDcCurrentAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeDcCurrentAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readDcCurrentMinAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeDcCurrentMinAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readDcCurrentMaxAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeDcCurrentMaxAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readDcPowerAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeDcPowerAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readDcPowerMinAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeDcPowerMinAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readDcPowerMaxAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeDcPowerMaxAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readDcVoltageMultiplierAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeDcVoltageMultiplierAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readDcVoltageDivisorAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeDcVoltageDivisorAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readDcCurrentMultiplierAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeDcCurrentMultiplierAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readDcCurrentDivisorAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeDcCurrentDivisorAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readDcPowerMultiplierAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeDcPowerMultiplierAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readDcPowerDivisorAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeDcPowerDivisorAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readAcFrequencyAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeAcFrequencyAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readAcFrequencyMinAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeAcFrequencyMinAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readAcFrequencyMaxAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeAcFrequencyMaxAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readNeutralCurrentAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeNeutralCurrentAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readTotalActivePowerAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeTotalActivePowerAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readTotalReactivePowerAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeTotalReactivePowerAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readTotalApparentPowerAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeTotalApparentPowerAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readMeasured1stHarmonicCurrentAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeMeasured1stHarmonicCurrentAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readMeasured3rdHarmonicCurrentAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeMeasured3rdHarmonicCurrentAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readMeasured5thHarmonicCurrentAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeMeasured5thHarmonicCurrentAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readMeasured7thHarmonicCurrentAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeMeasured7thHarmonicCurrentAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readMeasured9thHarmonicCurrentAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeMeasured9thHarmonicCurrentAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readMeasured11thHarmonicCurrentAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeMeasured11thHarmonicCurrentAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readMeasuredPhase1stHarmonicCurrentAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeMeasuredPhase1stHarmonicCurrentAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readMeasuredPhase3rdHarmonicCurrentAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeMeasuredPhase3rdHarmonicCurrentAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readMeasuredPhase5thHarmonicCurrentAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeMeasuredPhase5thHarmonicCurrentAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readMeasuredPhase7thHarmonicCurrentAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeMeasuredPhase7thHarmonicCurrentAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readMeasuredPhase9thHarmonicCurrentAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeMeasuredPhase9thHarmonicCurrentAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readMeasuredPhase11thHarmonicCurrentAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeMeasuredPhase11thHarmonicCurrentAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readAcFrequencyMultiplierAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeAcFrequencyMultiplierAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readAcFrequencyDivisorAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeAcFrequencyDivisorAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readPowerMultiplierAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribePowerMultiplierAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readPowerDivisorAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribePowerDivisorAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readHarmonicCurrentMultiplierAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeHarmonicCurrentMultiplierAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readPhaseHarmonicCurrentMultiplierAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribePhaseHarmonicCurrentMultiplierAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readInstantaneousVoltageAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeInstantaneousVoltageAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readInstantaneousLineCurrentAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeInstantaneousLineCurrentAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readInstantaneousActiveCurrentAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeInstantaneousActiveCurrentAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readInstantaneousReactiveCurrentAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeInstantaneousReactiveCurrentAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readInstantaneousPowerAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeInstantaneousPowerAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readRmsVoltageAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsVoltageAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readRmsVoltageMinAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsVoltageMinAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readRmsVoltageMaxAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsVoltageMaxAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readRmsCurrentAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsCurrentAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readRmsCurrentMinAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsCurrentMinAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readRmsCurrentMaxAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsCurrentMaxAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readActivePowerAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeActivePowerAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readActivePowerMinAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeActivePowerMinAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readActivePowerMaxAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeActivePowerMaxAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readReactivePowerAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeReactivePowerAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readApparentPowerAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeApparentPowerAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readPowerFactorAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribePowerFactorAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readAverageRmsVoltageMeasurementPeriodAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun writeAverageRmsVoltageMeasurementPeriodAttribute(value: UShort) {
    // Implementation needs to be added here
  }

  suspend fun writeAverageRmsVoltageMeasurementPeriodAttribute(
    value: UShort,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  suspend fun subscribeAverageRmsVoltageMeasurementPeriodAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readAverageRmsUnderVoltageCounterAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun writeAverageRmsUnderVoltageCounterAttribute(value: UShort) {
    // Implementation needs to be added here
  }

  suspend fun writeAverageRmsUnderVoltageCounterAttribute(value: UShort, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeAverageRmsUnderVoltageCounterAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readRmsExtremeOverVoltagePeriodAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun writeRmsExtremeOverVoltagePeriodAttribute(value: UShort) {
    // Implementation needs to be added here
  }

  suspend fun writeRmsExtremeOverVoltagePeriodAttribute(value: UShort, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsExtremeOverVoltagePeriodAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readRmsExtremeUnderVoltagePeriodAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun writeRmsExtremeUnderVoltagePeriodAttribute(value: UShort) {
    // Implementation needs to be added here
  }

  suspend fun writeRmsExtremeUnderVoltagePeriodAttribute(value: UShort, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsExtremeUnderVoltagePeriodAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readRmsVoltageSagPeriodAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun writeRmsVoltageSagPeriodAttribute(value: UShort) {
    // Implementation needs to be added here
  }

  suspend fun writeRmsVoltageSagPeriodAttribute(value: UShort, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsVoltageSagPeriodAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readRmsVoltageSwellPeriodAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun writeRmsVoltageSwellPeriodAttribute(value: UShort) {
    // Implementation needs to be added here
  }

  suspend fun writeRmsVoltageSwellPeriodAttribute(value: UShort, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsVoltageSwellPeriodAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readAcVoltageMultiplierAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeAcVoltageMultiplierAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readAcVoltageDivisorAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeAcVoltageDivisorAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readAcCurrentMultiplierAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeAcCurrentMultiplierAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readAcCurrentDivisorAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeAcCurrentDivisorAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readAcPowerMultiplierAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeAcPowerMultiplierAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readAcPowerDivisorAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeAcPowerDivisorAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readOverloadAlarmsMaskAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun writeOverloadAlarmsMaskAttribute(value: UInt) {
    // Implementation needs to be added here
  }

  suspend fun writeOverloadAlarmsMaskAttribute(value: UInt, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeOverloadAlarmsMaskAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readVoltageOverloadAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeVoltageOverloadAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readCurrentOverloadAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeCurrentOverloadAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readAcOverloadAlarmsMaskAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun writeAcOverloadAlarmsMaskAttribute(value: UInt) {
    // Implementation needs to be added here
  }

  suspend fun writeAcOverloadAlarmsMaskAttribute(value: UInt, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeAcOverloadAlarmsMaskAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readAcVoltageOverloadAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeAcVoltageOverloadAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readAcCurrentOverloadAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeAcCurrentOverloadAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readAcActivePowerOverloadAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeAcActivePowerOverloadAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readAcReactivePowerOverloadAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeAcReactivePowerOverloadAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readAverageRmsOverVoltageAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeAverageRmsOverVoltageAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readAverageRmsUnderVoltageAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeAverageRmsUnderVoltageAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readRmsExtremeOverVoltageAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsExtremeOverVoltageAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readRmsExtremeUnderVoltageAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsExtremeUnderVoltageAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readRmsVoltageSagAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsVoltageSagAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readRmsVoltageSwellAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsVoltageSwellAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readLineCurrentPhaseBAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeLineCurrentPhaseBAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readActiveCurrentPhaseBAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeActiveCurrentPhaseBAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readReactiveCurrentPhaseBAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeReactiveCurrentPhaseBAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readRmsVoltagePhaseBAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsVoltagePhaseBAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readRmsVoltageMinPhaseBAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsVoltageMinPhaseBAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readRmsVoltageMaxPhaseBAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsVoltageMaxPhaseBAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readRmsCurrentPhaseBAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsCurrentPhaseBAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readRmsCurrentMinPhaseBAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsCurrentMinPhaseBAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readRmsCurrentMaxPhaseBAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsCurrentMaxPhaseBAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readActivePowerPhaseBAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeActivePowerPhaseBAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readActivePowerMinPhaseBAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeActivePowerMinPhaseBAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readActivePowerMaxPhaseBAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeActivePowerMaxPhaseBAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readReactivePowerPhaseBAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeReactivePowerPhaseBAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readApparentPowerPhaseBAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeApparentPowerPhaseBAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readPowerFactorPhaseBAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribePowerFactorPhaseBAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readAverageRmsVoltageMeasurementPeriodPhaseBAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeAverageRmsVoltageMeasurementPeriodPhaseBAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readAverageRmsOverVoltageCounterPhaseBAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeAverageRmsOverVoltageCounterPhaseBAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readAverageRmsUnderVoltageCounterPhaseBAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeAverageRmsUnderVoltageCounterPhaseBAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readRmsExtremeOverVoltagePeriodPhaseBAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsExtremeOverVoltagePeriodPhaseBAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readRmsExtremeUnderVoltagePeriodPhaseBAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsExtremeUnderVoltagePeriodPhaseBAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readRmsVoltageSagPeriodPhaseBAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsVoltageSagPeriodPhaseBAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readRmsVoltageSwellPeriodPhaseBAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsVoltageSwellPeriodPhaseBAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readLineCurrentPhaseCAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeLineCurrentPhaseCAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readActiveCurrentPhaseCAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeActiveCurrentPhaseCAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readReactiveCurrentPhaseCAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeReactiveCurrentPhaseCAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readRmsVoltagePhaseCAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsVoltagePhaseCAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readRmsVoltageMinPhaseCAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsVoltageMinPhaseCAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readRmsVoltageMaxPhaseCAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsVoltageMaxPhaseCAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readRmsCurrentPhaseCAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsCurrentPhaseCAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readRmsCurrentMinPhaseCAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsCurrentMinPhaseCAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readRmsCurrentMaxPhaseCAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsCurrentMaxPhaseCAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readActivePowerPhaseCAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeActivePowerPhaseCAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readActivePowerMinPhaseCAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeActivePowerMinPhaseCAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readActivePowerMaxPhaseCAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeActivePowerMaxPhaseCAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readReactivePowerPhaseCAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeReactivePowerPhaseCAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readApparentPowerPhaseCAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeApparentPowerPhaseCAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readPowerFactorPhaseCAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribePowerFactorPhaseCAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readAverageRmsVoltageMeasurementPeriodPhaseCAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeAverageRmsVoltageMeasurementPeriodPhaseCAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readAverageRmsOverVoltageCounterPhaseCAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeAverageRmsOverVoltageCounterPhaseCAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readAverageRmsUnderVoltageCounterPhaseCAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeAverageRmsUnderVoltageCounterPhaseCAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readRmsExtremeOverVoltagePeriodPhaseCAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsExtremeOverVoltagePeriodPhaseCAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readRmsExtremeUnderVoltagePeriodPhaseCAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsExtremeUnderVoltagePeriodPhaseCAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readRmsVoltageSagPeriodPhaseCAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsVoltageSagPeriodPhaseCAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readRmsVoltageSwellPeriodPhaseCAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeRmsVoltageSwellPeriodPhaseCAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
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
    const val CLUSTER_ID: UInt = 2820u
  }
}
