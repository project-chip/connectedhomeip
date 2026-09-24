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

class DynamicLightingClusterEffectColorStruct(
  val level: UByte?,
  val x: UShort?,
  val y: UShort?,
  val hue: UByte?,
  val enhancedHue: UShort?,
  val saturation: UByte?,
) {
  override fun toString(): String = buildString {
    append("DynamicLightingClusterEffectColorStruct {\n")
    append("\tlevel : $level\n")
    append("\tx : $x\n")
    append("\ty : $y\n")
    append("\thue : $hue\n")
    append("\tenhancedHue : $enhancedHue\n")
    append("\tsaturation : $saturation\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (level != null) {
        put(ContextSpecificTag(TAG_LEVEL), level)
      } else {
        putNull(ContextSpecificTag(TAG_LEVEL))
      }
      if (x != null) {
        put(ContextSpecificTag(TAG_X), x)
      } else {
        putNull(ContextSpecificTag(TAG_X))
      }
      if (y != null) {
        put(ContextSpecificTag(TAG_Y), y)
      } else {
        putNull(ContextSpecificTag(TAG_Y))
      }
      if (hue != null) {
        put(ContextSpecificTag(TAG_HUE), hue)
      } else {
        putNull(ContextSpecificTag(TAG_HUE))
      }
      if (enhancedHue != null) {
        put(ContextSpecificTag(TAG_ENHANCED_HUE), enhancedHue)
      } else {
        putNull(ContextSpecificTag(TAG_ENHANCED_HUE))
      }
      if (saturation != null) {
        put(ContextSpecificTag(TAG_SATURATION), saturation)
      } else {
        putNull(ContextSpecificTag(TAG_SATURATION))
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_LEVEL = 0
    private const val TAG_X = 1
    private const val TAG_Y = 2
    private const val TAG_HUE = 3
    private const val TAG_ENHANCED_HUE = 4
    private const val TAG_SATURATION = 5

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): DynamicLightingClusterEffectColorStruct {
      tlvReader.enterStructure(tlvTag)
      val level =
        if (!tlvReader.isNull()) {
          tlvReader.getUByte(ContextSpecificTag(TAG_LEVEL))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_LEVEL))
          null
        }
      val x =
        if (!tlvReader.isNull()) {
          tlvReader.getUShort(ContextSpecificTag(TAG_X))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_X))
          null
        }
      val y =
        if (!tlvReader.isNull()) {
          tlvReader.getUShort(ContextSpecificTag(TAG_Y))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_Y))
          null
        }
      val hue =
        if (!tlvReader.isNull()) {
          tlvReader.getUByte(ContextSpecificTag(TAG_HUE))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_HUE))
          null
        }
      val enhancedHue =
        if (!tlvReader.isNull()) {
          tlvReader.getUShort(ContextSpecificTag(TAG_ENHANCED_HUE))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_ENHANCED_HUE))
          null
        }
      val saturation =
        if (!tlvReader.isNull()) {
          tlvReader.getUByte(ContextSpecificTag(TAG_SATURATION))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_SATURATION))
          null
        }

      tlvReader.exitContainer()

      return DynamicLightingClusterEffectColorStruct(level, x, y, hue, enhancedHue, saturation)
    }
  }
}
