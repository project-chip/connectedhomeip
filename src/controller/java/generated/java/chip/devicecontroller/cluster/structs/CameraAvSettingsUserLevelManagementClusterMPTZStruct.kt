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

class CameraAvSettingsUserLevelManagementClusterMPTZStruct(
  val pan: Optional<Int>,
  val tilt: Optional<Int>,
  val zoom: Optional<UInt>,
) {
  override fun toString(): String = buildString {
    append("CameraAvSettingsUserLevelManagementClusterMPTZStruct {\n")
    append("\tpan : $pan\n")
    append("\ttilt : $tilt\n")
    append("\tzoom : $zoom\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (pan.isPresent) {
        val optpan = pan.get()
        put(ContextSpecificTag(TAG_PAN), optpan)
      }
      if (tilt.isPresent) {
        val opttilt = tilt.get()
        put(ContextSpecificTag(TAG_TILT), opttilt)
      }
      if (zoom.isPresent) {
        val optzoom = zoom.get()
        put(ContextSpecificTag(TAG_ZOOM), optzoom)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_PAN = 0
    private const val TAG_TILT = 1
    private const val TAG_ZOOM = 2

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): CameraAvSettingsUserLevelManagementClusterMPTZStruct {
      tlvReader.enterStructure(tlvTag)
      val pan =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_PAN))) {
          Optional.of(tlvReader.getInt(ContextSpecificTag(TAG_PAN)))
        } else {
          Optional.empty()
        }
      val tilt =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_TILT))) {
          Optional.of(tlvReader.getInt(ContextSpecificTag(TAG_TILT)))
        } else {
          Optional.empty()
        }
      val zoom =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_ZOOM))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_ZOOM)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return CameraAvSettingsUserLevelManagementClusterMPTZStruct(pan, tilt, zoom)
    }
  }
}
