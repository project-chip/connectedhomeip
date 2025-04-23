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

class PushAvStreamTransportClusterTransportZoneOptionsStruct(
  val zone: UShort?,
  val sensitivity: Optional<UByte>,
) {
  override fun toString(): String = buildString {
    append("PushAvStreamTransportClusterTransportZoneOptionsStruct {\n")
    append("\tzone : $zone\n")
    append("\tsensitivity : $sensitivity\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (zone != null) {
        put(ContextSpecificTag(TAG_ZONE), zone)
      } else {
        putNull(ContextSpecificTag(TAG_ZONE))
      }
      if (sensitivity.isPresent) {
        val optsensitivity = sensitivity.get()
        put(ContextSpecificTag(TAG_SENSITIVITY), optsensitivity)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_ZONE = 1
    private const val TAG_SENSITIVITY = 2

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): PushAvStreamTransportClusterTransportZoneOptionsStruct {
      tlvReader.enterStructure(tlvTag)
      val zone =
        if (!tlvReader.isNull()) {
          tlvReader.getUShort(ContextSpecificTag(TAG_ZONE))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_ZONE))
          null
        }
      val sensitivity =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_SENSITIVITY))) {
          Optional.of(tlvReader.getUByte(ContextSpecificTag(TAG_SENSITIVITY)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return PushAvStreamTransportClusterTransportZoneOptionsStruct(zone, sensitivity)
    }
  }
}
