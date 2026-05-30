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

class ElectricalProtectionAlarmClusterOverVoltageRatingsStruct(
  val tripMechanism: Optional<UInt>,
  val tripVoltage: Optional<Long>,
  val maxContinuousOperatingVoltage: Optional<Long>,
  val responseTime: Optional<ULong>,
) {
  override fun toString(): String = buildString {
    append("ElectricalProtectionAlarmClusterOverVoltageRatingsStruct {\n")
    append("\ttripMechanism : $tripMechanism\n")
    append("\ttripVoltage : $tripVoltage\n")
    append("\tmaxContinuousOperatingVoltage : $maxContinuousOperatingVoltage\n")
    append("\tresponseTime : $responseTime\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (tripMechanism.isPresent) {
        val opttripMechanism = tripMechanism.get()
        put(ContextSpecificTag(TAG_TRIP_MECHANISM), opttripMechanism)
      }
      if (tripVoltage.isPresent) {
        val opttripVoltage = tripVoltage.get()
        put(ContextSpecificTag(TAG_TRIP_VOLTAGE), opttripVoltage)
      }
      if (maxContinuousOperatingVoltage.isPresent) {
        val optmaxContinuousOperatingVoltage = maxContinuousOperatingVoltage.get()
        put(
          ContextSpecificTag(TAG_MAX_CONTINUOUS_OPERATING_VOLTAGE),
          optmaxContinuousOperatingVoltage,
        )
      }
      if (responseTime.isPresent) {
        val optresponseTime = responseTime.get()
        put(ContextSpecificTag(TAG_RESPONSE_TIME), optresponseTime)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_TRIP_MECHANISM = 0
    private const val TAG_TRIP_VOLTAGE = 1
    private const val TAG_MAX_CONTINUOUS_OPERATING_VOLTAGE = 2
    private const val TAG_RESPONSE_TIME = 3

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): ElectricalProtectionAlarmClusterOverVoltageRatingsStruct {
      tlvReader.enterStructure(tlvTag)
      val tripMechanism =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_TRIP_MECHANISM))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_TRIP_MECHANISM)))
        } else {
          Optional.empty()
        }
      val tripVoltage =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_TRIP_VOLTAGE))) {
          Optional.of(tlvReader.getLong(ContextSpecificTag(TAG_TRIP_VOLTAGE)))
        } else {
          Optional.empty()
        }
      val maxContinuousOperatingVoltage =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_MAX_CONTINUOUS_OPERATING_VOLTAGE))) {
          Optional.of(tlvReader.getLong(ContextSpecificTag(TAG_MAX_CONTINUOUS_OPERATING_VOLTAGE)))
        } else {
          Optional.empty()
        }
      val responseTime =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_RESPONSE_TIME))) {
          Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_RESPONSE_TIME)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return ElectricalProtectionAlarmClusterOverVoltageRatingsStruct(
        tripMechanism,
        tripVoltage,
        maxContinuousOperatingVoltage,
        responseTime,
      )
    }
  }
}
