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
package matter.controller.cluster.structs

import java.util.Optional
import matter.controller.cluster.*
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class ElectricalProtectionAlarmClusterShortCircuitRatingsStruct(
  val tripCurrent: Optional<Long>,
  val tripMechanism: Optional<UByte>,
  val tripCurve: Optional<UByte>,
  val ultimateMaxCurrent: Optional<Long>,
  val serviceMaxCurrent: Optional<Long>,
  val maxCurrent: Optional<Long>,
) {
  override fun toString(): String = buildString {
    append("ElectricalProtectionAlarmClusterShortCircuitRatingsStruct {\n")
    append("\ttripCurrent : $tripCurrent\n")
    append("\ttripMechanism : $tripMechanism\n")
    append("\ttripCurve : $tripCurve\n")
    append("\tultimateMaxCurrent : $ultimateMaxCurrent\n")
    append("\tserviceMaxCurrent : $serviceMaxCurrent\n")
    append("\tmaxCurrent : $maxCurrent\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (tripCurrent.isPresent) {
        val opttripCurrent = tripCurrent.get()
        put(ContextSpecificTag(TAG_TRIP_CURRENT), opttripCurrent)
      }
      if (tripMechanism.isPresent) {
        val opttripMechanism = tripMechanism.get()
        put(ContextSpecificTag(TAG_TRIP_MECHANISM), opttripMechanism)
      }
      if (tripCurve.isPresent) {
        val opttripCurve = tripCurve.get()
        put(ContextSpecificTag(TAG_TRIP_CURVE), opttripCurve)
      }
      if (ultimateMaxCurrent.isPresent) {
        val optultimateMaxCurrent = ultimateMaxCurrent.get()
        put(ContextSpecificTag(TAG_ULTIMATE_MAX_CURRENT), optultimateMaxCurrent)
      }
      if (serviceMaxCurrent.isPresent) {
        val optserviceMaxCurrent = serviceMaxCurrent.get()
        put(ContextSpecificTag(TAG_SERVICE_MAX_CURRENT), optserviceMaxCurrent)
      }
      if (maxCurrent.isPresent) {
        val optmaxCurrent = maxCurrent.get()
        put(ContextSpecificTag(TAG_MAX_CURRENT), optmaxCurrent)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_TRIP_CURRENT = 0
    private const val TAG_TRIP_MECHANISM = 1
    private const val TAG_TRIP_CURVE = 2
    private const val TAG_ULTIMATE_MAX_CURRENT = 3
    private const val TAG_SERVICE_MAX_CURRENT = 4
    private const val TAG_MAX_CURRENT = 5

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): ElectricalProtectionAlarmClusterShortCircuitRatingsStruct {
      tlvReader.enterStructure(tlvTag)
      val tripCurrent =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_TRIP_CURRENT))) {
          Optional.of(tlvReader.getLong(ContextSpecificTag(TAG_TRIP_CURRENT)))
        } else {
          Optional.empty()
        }
      val tripMechanism =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_TRIP_MECHANISM))) {
          Optional.of(tlvReader.getUByte(ContextSpecificTag(TAG_TRIP_MECHANISM)))
        } else {
          Optional.empty()
        }
      val tripCurve =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_TRIP_CURVE))) {
          Optional.of(tlvReader.getUByte(ContextSpecificTag(TAG_TRIP_CURVE)))
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
      val maxCurrent =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_MAX_CURRENT))) {
          Optional.of(tlvReader.getLong(ContextSpecificTag(TAG_MAX_CURRENT)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return ElectricalProtectionAlarmClusterShortCircuitRatingsStruct(
        tripCurrent,
        tripMechanism,
        tripCurve,
        ultimateMaxCurrent,
        serviceMaxCurrent,
        maxCurrent,
      )
    }
  }
}
