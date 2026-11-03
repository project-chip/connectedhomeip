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

class ElectricalProtectionAlarmClusterOverLoadRatingsStruct(
  val tripCurrent: Optional<Long>,
  val tripCurve: Optional<UInt>,
  val tripMechanism: Optional<UInt>,
  val ultimateMaxCurrent: Optional<Long>,
  val serviceMaxCurrent: Optional<Long>,
) {
  override fun toString(): String = buildString {
    append("ElectricalProtectionAlarmClusterOverLoadRatingsStruct {\n")
    append("\ttripCurrent : $tripCurrent\n")
    append("\ttripCurve : $tripCurve\n")
    append("\ttripMechanism : $tripMechanism\n")
    append("\tultimateMaxCurrent : $ultimateMaxCurrent\n")
    append("\tserviceMaxCurrent : $serviceMaxCurrent\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (tripCurrent.isPresent) {
        val opttripCurrent = tripCurrent.get()
        put(ContextSpecificTag(TAG_TRIP_CURRENT), opttripCurrent)
      }
      if (tripCurve.isPresent) {
        val opttripCurve = tripCurve.get()
        put(ContextSpecificTag(TAG_TRIP_CURVE), opttripCurve)
      }
      if (tripMechanism.isPresent) {
        val opttripMechanism = tripMechanism.get()
        put(ContextSpecificTag(TAG_TRIP_MECHANISM), opttripMechanism)
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
    private const val TAG_TRIP_CURRENT = 0
    private const val TAG_TRIP_CURVE = 1
    private const val TAG_TRIP_MECHANISM = 2
    private const val TAG_ULTIMATE_MAX_CURRENT = 3
    private const val TAG_SERVICE_MAX_CURRENT = 4

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): ElectricalProtectionAlarmClusterOverLoadRatingsStruct {
      tlvReader.enterStructure(tlvTag)
      val tripCurrent =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_TRIP_CURRENT))) {
          Optional.of(tlvReader.getLong(ContextSpecificTag(TAG_TRIP_CURRENT)))
        } else {
          Optional.empty()
        }
      val tripCurve =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_TRIP_CURVE))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_TRIP_CURVE)))
        } else {
          Optional.empty()
        }
      val tripMechanism =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_TRIP_MECHANISM))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_TRIP_MECHANISM)))
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

      return ElectricalProtectionAlarmClusterOverLoadRatingsStruct(
        tripCurrent,
        tripCurve,
        tripMechanism,
        ultimateMaxCurrent,
        serviceMaxCurrent,
      )
    }
  }
}
