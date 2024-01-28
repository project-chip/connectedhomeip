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
package chip.devicecontroller.cluster.structs

import chip.devicecontroller.cluster.*
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvParsingException
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

import java.util.Optional

class DeviceEnergyManagementClusterSlotStruct (
    val minDuration: ULong,
    val maxDuration: ULong,
    val defaultDuration: ULong,
    val elapsedSlotTime: ULong,
    val remainingSlotTime: ULong,
    val slotIsPauseable: Optional<Boolean>,
    val minPauseDuration: Optional<ULong>,
    val maxPauseDuration: Optional<ULong>,
    val manufacturerESAState: Optional<UInt>,
    val nominalPower: Optional<Long>,
    val minPower: Optional<Long>,
    val maxPower: Optional<Long>,
    val nominalEnergy: Optional<Long>,
    val costs: Optional<List<DeviceEnergyManagementClusterCostStruct>>,
    val minPowerAdjustment: Optional<Long>,
    val maxPowerAdjustment: Optional<Long>,
    val minDurationAdjustment: Optional<ULong>,
    val maxDurationAdjustment: Optional<ULong>) {
  override fun toString(): String  = buildString {
    append("DeviceEnergyManagementClusterSlotStruct {\n")
    append("\tminDuration : $minDuration\n")
    append("\tmaxDuration : $maxDuration\n")
    append("\tdefaultDuration : $defaultDuration\n")
    append("\telapsedSlotTime : $elapsedSlotTime\n")
    append("\tremainingSlotTime : $remainingSlotTime\n")
    append("\tslotIsPauseable : $slotIsPauseable\n")
    append("\tminPauseDuration : $minPauseDuration\n")
    append("\tmaxPauseDuration : $maxPauseDuration\n")
    append("\tmanufacturerESAState : $manufacturerESAState\n")
    append("\tnominalPower : $nominalPower\n")
    append("\tminPower : $minPower\n")
    append("\tmaxPower : $maxPower\n")
    append("\tnominalEnergy : $nominalEnergy\n")
    append("\tcosts : $costs\n")
    append("\tminPowerAdjustment : $minPowerAdjustment\n")
    append("\tmaxPowerAdjustment : $maxPowerAdjustment\n")
    append("\tminDurationAdjustment : $minDurationAdjustment\n")
    append("\tmaxDurationAdjustment : $maxDurationAdjustment\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_MIN_DURATION), minDuration)
      put(ContextSpecificTag(TAG_MAX_DURATION), maxDuration)
      put(ContextSpecificTag(TAG_DEFAULT_DURATION), defaultDuration)
      put(ContextSpecificTag(TAG_ELAPSED_SLOT_TIME), elapsedSlotTime)
      put(ContextSpecificTag(TAG_REMAINING_SLOT_TIME), remainingSlotTime)
      if (slotIsPauseable.isPresent) {
      val optslotIsPauseable = slotIsPauseable.get()
      put(ContextSpecificTag(TAG_SLOT_IS_PAUSEABLE), optslotIsPauseable)
    }
      if (minPauseDuration.isPresent) {
      val optminPauseDuration = minPauseDuration.get()
      put(ContextSpecificTag(TAG_MIN_PAUSE_DURATION), optminPauseDuration)
    }
      if (maxPauseDuration.isPresent) {
      val optmaxPauseDuration = maxPauseDuration.get()
      put(ContextSpecificTag(TAG_MAX_PAUSE_DURATION), optmaxPauseDuration)
    }
      if (manufacturerESAState.isPresent) {
      val optmanufacturerESAState = manufacturerESAState.get()
      put(ContextSpecificTag(TAG_MANUFACTURER_E_S_A_STATE), optmanufacturerESAState)
    }
      if (nominalPower.isPresent) {
      val optnominalPower = nominalPower.get()
      put(ContextSpecificTag(TAG_NOMINAL_POWER), optnominalPower)
    }
      if (minPower.isPresent) {
      val optminPower = minPower.get()
      put(ContextSpecificTag(TAG_MIN_POWER), optminPower)
    }
      if (maxPower.isPresent) {
      val optmaxPower = maxPower.get()
      put(ContextSpecificTag(TAG_MAX_POWER), optmaxPower)
    }
      if (nominalEnergy.isPresent) {
      val optnominalEnergy = nominalEnergy.get()
      put(ContextSpecificTag(TAG_NOMINAL_ENERGY), optnominalEnergy)
    }
      if (costs.isPresent) {
      val optcosts = costs.get()
      startArray(ContextSpecificTag(TAG_COSTS))
      for (item in optcosts.iterator()) {
        item.toTlv(AnonymousTag, this)
      }
      endArray()
    }
      if (minPowerAdjustment.isPresent) {
      val optminPowerAdjustment = minPowerAdjustment.get()
      put(ContextSpecificTag(TAG_MIN_POWER_ADJUSTMENT), optminPowerAdjustment)
    }
      if (maxPowerAdjustment.isPresent) {
      val optmaxPowerAdjustment = maxPowerAdjustment.get()
      put(ContextSpecificTag(TAG_MAX_POWER_ADJUSTMENT), optmaxPowerAdjustment)
    }
      if (minDurationAdjustment.isPresent) {
      val optminDurationAdjustment = minDurationAdjustment.get()
      put(ContextSpecificTag(TAG_MIN_DURATION_ADJUSTMENT), optminDurationAdjustment)
    }
      if (maxDurationAdjustment.isPresent) {
      val optmaxDurationAdjustment = maxDurationAdjustment.get()
      put(ContextSpecificTag(TAG_MAX_DURATION_ADJUSTMENT), optmaxDurationAdjustment)
    }
      endStructure()
    }
  }

  companion object {
    private const val TAG_MIN_DURATION = 0
    private const val TAG_MAX_DURATION = 1
    private const val TAG_DEFAULT_DURATION = 2
    private const val TAG_ELAPSED_SLOT_TIME = 3
    private const val TAG_REMAINING_SLOT_TIME = 4
    private const val TAG_SLOT_IS_PAUSEABLE = 5
    private const val TAG_MIN_PAUSE_DURATION = 6
    private const val TAG_MAX_PAUSE_DURATION = 7
    private const val TAG_MANUFACTURER_E_S_A_STATE = 8
    private const val TAG_NOMINAL_POWER = 9
    private const val TAG_MIN_POWER = 10
    private const val TAG_MAX_POWER = 11
    private const val TAG_NOMINAL_ENERGY = 12
    private const val TAG_COSTS = 13
    private const val TAG_MIN_POWER_ADJUSTMENT = 14
    private const val TAG_MAX_POWER_ADJUSTMENT = 15
    private const val TAG_MIN_DURATION_ADJUSTMENT = 16
    private const val TAG_MAX_DURATION_ADJUSTMENT = 17

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader) : DeviceEnergyManagementClusterSlotStruct {
      tlvReader.enterStructure(tlvTag)
      val minDuration = tlvReader.getULong(ContextSpecificTag(TAG_MIN_DURATION))
      val maxDuration = tlvReader.getULong(ContextSpecificTag(TAG_MAX_DURATION))
      val defaultDuration = tlvReader.getULong(ContextSpecificTag(TAG_DEFAULT_DURATION))
      val elapsedSlotTime = tlvReader.getULong(ContextSpecificTag(TAG_ELAPSED_SLOT_TIME))
      val remainingSlotTime = tlvReader.getULong(ContextSpecificTag(TAG_REMAINING_SLOT_TIME))
      val slotIsPauseable = if (tlvReader.isNextTag(ContextSpecificTag(TAG_SLOT_IS_PAUSEABLE))) {
      Optional.of(tlvReader.getBoolean(ContextSpecificTag(TAG_SLOT_IS_PAUSEABLE)))
    } else {
      Optional.empty()
    }
      val minPauseDuration = if (tlvReader.isNextTag(ContextSpecificTag(TAG_MIN_PAUSE_DURATION))) {
      Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_MIN_PAUSE_DURATION)))
    } else {
      Optional.empty()
    }
      val maxPauseDuration = if (tlvReader.isNextTag(ContextSpecificTag(TAG_MAX_PAUSE_DURATION))) {
      Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_MAX_PAUSE_DURATION)))
    } else {
      Optional.empty()
    }
      val manufacturerESAState = if (tlvReader.isNextTag(ContextSpecificTag(TAG_MANUFACTURER_E_S_A_STATE))) {
      Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_MANUFACTURER_E_S_A_STATE)))
    } else {
      Optional.empty()
    }
      val nominalPower = if (tlvReader.isNextTag(ContextSpecificTag(TAG_NOMINAL_POWER))) {
      Optional.of(tlvReader.getLong(ContextSpecificTag(TAG_NOMINAL_POWER)))
    } else {
      Optional.empty()
    }
      val minPower = if (tlvReader.isNextTag(ContextSpecificTag(TAG_MIN_POWER))) {
      Optional.of(tlvReader.getLong(ContextSpecificTag(TAG_MIN_POWER)))
    } else {
      Optional.empty()
    }
      val maxPower = if (tlvReader.isNextTag(ContextSpecificTag(TAG_MAX_POWER))) {
      Optional.of(tlvReader.getLong(ContextSpecificTag(TAG_MAX_POWER)))
    } else {
      Optional.empty()
    }
      val nominalEnergy = if (tlvReader.isNextTag(ContextSpecificTag(TAG_NOMINAL_ENERGY))) {
      Optional.of(tlvReader.getLong(ContextSpecificTag(TAG_NOMINAL_ENERGY)))
    } else {
      Optional.empty()
    }
      val costs = if (tlvReader.isNextTag(ContextSpecificTag(TAG_COSTS))) {
      Optional.of(buildList<DeviceEnergyManagementClusterCostStruct> {
      tlvReader.enterArray(ContextSpecificTag(TAG_COSTS))
      while(!tlvReader.isEndOfContainer()) {
        add(DeviceEnergyManagementClusterCostStruct.fromTlv(AnonymousTag, tlvReader))
      }
      tlvReader.exitContainer()
    })
    } else {
      Optional.empty()
    }
      val minPowerAdjustment = if (tlvReader.isNextTag(ContextSpecificTag(TAG_MIN_POWER_ADJUSTMENT))) {
      Optional.of(tlvReader.getLong(ContextSpecificTag(TAG_MIN_POWER_ADJUSTMENT)))
    } else {
      Optional.empty()
    }
      val maxPowerAdjustment = if (tlvReader.isNextTag(ContextSpecificTag(TAG_MAX_POWER_ADJUSTMENT))) {
      Optional.of(tlvReader.getLong(ContextSpecificTag(TAG_MAX_POWER_ADJUSTMENT)))
    } else {
      Optional.empty()
    }
      val minDurationAdjustment = if (tlvReader.isNextTag(ContextSpecificTag(TAG_MIN_DURATION_ADJUSTMENT))) {
      Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_MIN_DURATION_ADJUSTMENT)))
    } else {
      Optional.empty()
    }
      val maxDurationAdjustment = if (tlvReader.isNextTag(ContextSpecificTag(TAG_MAX_DURATION_ADJUSTMENT))) {
      Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_MAX_DURATION_ADJUSTMENT)))
    } else {
      Optional.empty()
    }
      
      tlvReader.exitContainer()

      return DeviceEnergyManagementClusterSlotStruct(minDuration, maxDuration, defaultDuration, elapsedSlotTime, remainingSlotTime, slotIsPauseable, minPauseDuration, maxPauseDuration, manufacturerESAState, nominalPower, minPower, maxPower, nominalEnergy, costs, minPowerAdjustment, maxPowerAdjustment, minDurationAdjustment, maxDurationAdjustment)
    }
  }
}
