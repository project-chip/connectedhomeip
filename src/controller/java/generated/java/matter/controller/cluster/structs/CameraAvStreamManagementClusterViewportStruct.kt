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

class CameraAvStreamManagementClusterViewportStruct(
  val x1: UShort,
  val y1: UShort,
  val x2: UShort,
  val y2: UShort,
) {
  override fun toString(): String = buildString {
    append("CameraAvStreamManagementClusterViewportStruct {\n")
    append("\tx1 : $x1\n")
    append("\ty1 : $y1\n")
    append("\tx2 : $x2\n")
    append("\ty2 : $y2\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_X1), x1)
      put(ContextSpecificTag(TAG_Y1), y1)
      put(ContextSpecificTag(TAG_X2), x2)
      put(ContextSpecificTag(TAG_Y2), y2)
      endStructure()
    }
  }

  companion object {
    private const val TAG_X1 = 0
    private const val TAG_Y1 = 1
    private const val TAG_X2 = 2
    private const val TAG_Y2 = 3

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): CameraAvStreamManagementClusterViewportStruct {
      tlvReader.enterStructure(tlvTag)
      val x1 = tlvReader.getUShort(ContextSpecificTag(TAG_X1))
      val y1 = tlvReader.getUShort(ContextSpecificTag(TAG_Y1))
      val x2 = tlvReader.getUShort(ContextSpecificTag(TAG_X2))
      val y2 = tlvReader.getUShort(ContextSpecificTag(TAG_Y2))

      tlvReader.exitContainer()

      return CameraAvStreamManagementClusterViewportStruct(x1, y1, x2, y2)
    }
  }
}
