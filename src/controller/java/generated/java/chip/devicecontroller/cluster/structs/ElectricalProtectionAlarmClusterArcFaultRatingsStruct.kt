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

class ElectricalProtectionAlarmClusterArcFaultRatingsStruct(
  val seriesArcCurrentSensitivity: Optional<Long>,
  val parallelArcCurrentSensitivity: Optional<Long>,
  val supportedArcCauses: Optional<UInt>,
) {
  override fun toString(): String = buildString {
    append("ElectricalProtectionAlarmClusterArcFaultRatingsStruct {\n")
    append("\tseriesArcCurrentSensitivity : $seriesArcCurrentSensitivity\n")
    append("\tparallelArcCurrentSensitivity : $parallelArcCurrentSensitivity\n")
    append("\tsupportedArcCauses : $supportedArcCauses\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (seriesArcCurrentSensitivity.isPresent) {
        val optseriesArcCurrentSensitivity = seriesArcCurrentSensitivity.get()
        put(ContextSpecificTag(TAG_SERIES_ARC_CURRENT_SENSITIVITY), optseriesArcCurrentSensitivity)
      }
      if (parallelArcCurrentSensitivity.isPresent) {
        val optparallelArcCurrentSensitivity = parallelArcCurrentSensitivity.get()
        put(
          ContextSpecificTag(TAG_PARALLEL_ARC_CURRENT_SENSITIVITY),
          optparallelArcCurrentSensitivity,
        )
      }
      if (supportedArcCauses.isPresent) {
        val optsupportedArcCauses = supportedArcCauses.get()
        put(ContextSpecificTag(TAG_SUPPORTED_ARC_CAUSES), optsupportedArcCauses)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_SERIES_ARC_CURRENT_SENSITIVITY = 0
    private const val TAG_PARALLEL_ARC_CURRENT_SENSITIVITY = 1
    private const val TAG_SUPPORTED_ARC_CAUSES = 2

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): ElectricalProtectionAlarmClusterArcFaultRatingsStruct {
      tlvReader.enterStructure(tlvTag)
      val seriesArcCurrentSensitivity =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_SERIES_ARC_CURRENT_SENSITIVITY))) {
          Optional.of(tlvReader.getLong(ContextSpecificTag(TAG_SERIES_ARC_CURRENT_SENSITIVITY)))
        } else {
          Optional.empty()
        }
      val parallelArcCurrentSensitivity =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_PARALLEL_ARC_CURRENT_SENSITIVITY))) {
          Optional.of(tlvReader.getLong(ContextSpecificTag(TAG_PARALLEL_ARC_CURRENT_SENSITIVITY)))
        } else {
          Optional.empty()
        }
      val supportedArcCauses =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_SUPPORTED_ARC_CAUSES))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_SUPPORTED_ARC_CAUSES)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return ElectricalProtectionAlarmClusterArcFaultRatingsStruct(
        seriesArcCurrentSensitivity,
        parallelArcCurrentSensitivity,
        supportedArcCauses,
      )
    }
  }
}
