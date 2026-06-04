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
import java.util.Optional
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class ElectricalProtectionAlarmClusterResidualCurrentFaultRatingsStruct(
  val currentSensitivity: Optional<Long>,
  val tripMechanism: Optional<UInt>,
  val voltageDependent: Optional<Boolean>,
  val groundFaultClass: Optional<UInt>,
  val waveform: Optional<UInt>,
  val trippingCharacteristic: Optional<UInt>,
  val ultimateMaxCurrent: Optional<Long>,
  val serviceMaxCurrent: Optional<Long>,
) {
  override fun toString(): String = buildString {
    append("ElectricalProtectionAlarmClusterResidualCurrentFaultRatingsStruct {\n")
    append("\tcurrentSensitivity : $currentSensitivity\n")
    append("\ttripMechanism : $tripMechanism\n")
    append("\tvoltageDependent : $voltageDependent\n")
    append("\tgroundFaultClass : $groundFaultClass\n")
    append("\twaveform : $waveform\n")
    append("\ttrippingCharacteristic : $trippingCharacteristic\n")
    append("\tultimateMaxCurrent : $ultimateMaxCurrent\n")
    append("\tserviceMaxCurrent : $serviceMaxCurrent\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (currentSensitivity.isPresent) {
        val optcurrentSensitivity = currentSensitivity.get()
        put(ContextSpecificTag(TAG_CURRENT_SENSITIVITY), optcurrentSensitivity)
      }
      if (tripMechanism.isPresent) {
        val opttripMechanism = tripMechanism.get()
        put(ContextSpecificTag(TAG_TRIP_MECHANISM), opttripMechanism)
      }
      if (voltageDependent.isPresent) {
        val optvoltageDependent = voltageDependent.get()
        put(ContextSpecificTag(TAG_VOLTAGE_DEPENDENT), optvoltageDependent)
      }
      if (groundFaultClass.isPresent) {
        val optgroundFaultClass = groundFaultClass.get()
        put(ContextSpecificTag(TAG_GROUND_FAULT_CLASS), optgroundFaultClass)
      }
      if (waveform.isPresent) {
        val optwaveform = waveform.get()
        put(ContextSpecificTag(TAG_WAVEFORM), optwaveform)
      }
      if (trippingCharacteristic.isPresent) {
        val opttrippingCharacteristic = trippingCharacteristic.get()
        put(ContextSpecificTag(TAG_TRIPPING_CHARACTERISTIC), opttrippingCharacteristic)
      }
      if (ultimateMaxCurrent.isPresent) {
        val optultimateMaxCurrent = ultimateMaxCurrent.get()
        put(ContextSpecificTag(TAG_ULTIMATE_MAX_CURRENT), optultimateMaxCurrent)
      }
      if (serviceMaxCurrent.isPresent) {
        val optserviceMaxCurrent = serviceMaxCurrent.get()
        put(ContextSpecificTag(TAG_SERVICE_MAX_CURRENT), optserviceMaxCurrent)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_CURRENT_SENSITIVITY = 0
    private const val TAG_TRIP_MECHANISM = 1
    private const val TAG_VOLTAGE_DEPENDENT = 2
    private const val TAG_GROUND_FAULT_CLASS = 3
    private const val TAG_WAVEFORM = 4
    private const val TAG_TRIPPING_CHARACTERISTIC = 5
    private const val TAG_ULTIMATE_MAX_CURRENT = 6
    private const val TAG_SERVICE_MAX_CURRENT = 7

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): ElectricalProtectionAlarmClusterResidualCurrentFaultRatingsStruct {
      tlvReader.enterStructure(tlvTag)
      val currentSensitivity =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_CURRENT_SENSITIVITY))) {
          Optional.of(tlvReader.getLong(ContextSpecificTag(TAG_CURRENT_SENSITIVITY)))
        } else {
          Optional.empty()
        }
      val tripMechanism =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_TRIP_MECHANISM))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_TRIP_MECHANISM)))
        } else {
          Optional.empty()
        }
      val voltageDependent =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_VOLTAGE_DEPENDENT))) {
          Optional.of(tlvReader.getBoolean(ContextSpecificTag(TAG_VOLTAGE_DEPENDENT)))
        } else {
          Optional.empty()
        }
      val groundFaultClass =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_GROUND_FAULT_CLASS))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_GROUND_FAULT_CLASS)))
        } else {
          Optional.empty()
        }
      val waveform =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_WAVEFORM))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_WAVEFORM)))
        } else {
          Optional.empty()
        }
      val trippingCharacteristic =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_TRIPPING_CHARACTERISTIC))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_TRIPPING_CHARACTERISTIC)))
        } else {
          Optional.empty()
        }
      val ultimateMaxCurrent =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_ULTIMATE_MAX_CURRENT))) {
          Optional.of(tlvReader.getLong(ContextSpecificTag(TAG_ULTIMATE_MAX_CURRENT)))
        } else {
          Optional.empty()
        }
      val serviceMaxCurrent =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_SERVICE_MAX_CURRENT))) {
          Optional.of(tlvReader.getLong(ContextSpecificTag(TAG_SERVICE_MAX_CURRENT)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return ElectricalProtectionAlarmClusterResidualCurrentFaultRatingsStruct(
        currentSensitivity,
        tripMechanism,
        voltageDependent,
        groundFaultClass,
        waveform,
        trippingCharacteristic,
        ultimateMaxCurrent,
        serviceMaxCurrent,
      )
    }
  }
}
