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

import matter.controller.cluster.*
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class DynamicLightingClusterEffectStruct(
  val effectID: UShort,
  val source: UByte,
  val label: String,
  val maxSpeed: UShort,
  val defaultSpeed: UShort,
  val supportsColorPalette: Boolean,
) {
  override fun toString(): String = buildString {
    append("DynamicLightingClusterEffectStruct {\n")
    append("\teffectID : $effectID\n")
    append("\tsource : $source\n")
    append("\tlabel : $label\n")
    append("\tmaxSpeed : $maxSpeed\n")
    append("\tdefaultSpeed : $defaultSpeed\n")
    append("\tsupportsColorPalette : $supportsColorPalette\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_EFFECT_ID), effectID)
      put(ContextSpecificTag(TAG_SOURCE), source)
      put(ContextSpecificTag(TAG_LABEL), label)
      put(ContextSpecificTag(TAG_MAX_SPEED), maxSpeed)
      put(ContextSpecificTag(TAG_DEFAULT_SPEED), defaultSpeed)
      put(ContextSpecificTag(TAG_SUPPORTS_COLOR_PALETTE), supportsColorPalette)
      endStructure()
    }
  }

  companion object {
    private const val TAG_EFFECT_ID = 0
    private const val TAG_SOURCE = 1
    private const val TAG_LABEL = 2
    private const val TAG_MAX_SPEED = 3
    private const val TAG_DEFAULT_SPEED = 4
    private const val TAG_SUPPORTS_COLOR_PALETTE = 5

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): DynamicLightingClusterEffectStruct {
      tlvReader.enterStructure(tlvTag)
      val effectID = tlvReader.getUShort(ContextSpecificTag(TAG_EFFECT_ID))
      val source = tlvReader.getUByte(ContextSpecificTag(TAG_SOURCE))
      val label = tlvReader.getString(ContextSpecificTag(TAG_LABEL))
      val maxSpeed = tlvReader.getUShort(ContextSpecificTag(TAG_MAX_SPEED))
      val defaultSpeed = tlvReader.getUShort(ContextSpecificTag(TAG_DEFAULT_SPEED))
      val supportsColorPalette =
        tlvReader.getBoolean(ContextSpecificTag(TAG_SUPPORTS_COLOR_PALETTE))

      tlvReader.exitContainer()

      return DynamicLightingClusterEffectStruct(
        effectID,
        source,
        label,
        maxSpeed,
        defaultSpeed,
        supportsColorPalette,
      )
    }
  }
}
