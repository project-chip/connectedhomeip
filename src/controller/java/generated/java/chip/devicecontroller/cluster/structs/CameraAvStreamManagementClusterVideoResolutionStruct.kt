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
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class CameraAvStreamManagementClusterVideoResolutionStruct(val width: UInt, val height: UInt) {
  override fun toString(): String = buildString {
    append("CameraAvStreamManagementClusterVideoResolutionStruct {\n")
    append("\twidth : $width\n")
    append("\theight : $height\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_WIDTH), width)
      put(ContextSpecificTag(TAG_HEIGHT), height)
      endStructure()
    }
  }

  companion object {
    private const val TAG_WIDTH = 0
    private const val TAG_HEIGHT = 1

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader
    ): CameraAvStreamManagementClusterVideoResolutionStruct {
      tlvReader.enterStructure(tlvTag)
      val width = tlvReader.getUInt(ContextSpecificTag(TAG_WIDTH))
      val height = tlvReader.getUInt(ContextSpecificTag(TAG_HEIGHT))

      tlvReader.exitContainer()

      return CameraAvStreamManagementClusterVideoResolutionStruct(width, height)
    }
  }
}
