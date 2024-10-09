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

class CameraAvStreamManagementClusterSnapshotParamsStruct(
  val resolution: CameraAvStreamManagementClusterVideoResolutionStruct,
  val maxFrameRate: UShort,
  val imageCodec: UByte,
) {
  override fun toString(): String = buildString {
    append("CameraAvStreamManagementClusterSnapshotParamsStruct {\n")
    append("\tresolution : $resolution\n")
    append("\tmaxFrameRate : $maxFrameRate\n")
    append("\timageCodec : $imageCodec\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      resolution.toTlv(ContextSpecificTag(TAG_RESOLUTION), this)
      put(ContextSpecificTag(TAG_MAX_FRAME_RATE), maxFrameRate)
      put(ContextSpecificTag(TAG_IMAGE_CODEC), imageCodec)
      endStructure()
    }
  }

  companion object {
    private const val TAG_RESOLUTION = 0
    private const val TAG_MAX_FRAME_RATE = 1
    private const val TAG_IMAGE_CODEC = 2

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): CameraAvStreamManagementClusterSnapshotParamsStruct {
      tlvReader.enterStructure(tlvTag)
      val resolution =
        CameraAvStreamManagementClusterVideoResolutionStruct.fromTlv(
          ContextSpecificTag(TAG_RESOLUTION),
          tlvReader,
        )
      val maxFrameRate = tlvReader.getUShort(ContextSpecificTag(TAG_MAX_FRAME_RATE))
      val imageCodec = tlvReader.getUByte(ContextSpecificTag(TAG_IMAGE_CODEC))

      tlvReader.exitContainer()

      return CameraAvStreamManagementClusterSnapshotParamsStruct(
        resolution,
        maxFrameRate,
        imageCodec,
      )
    }
  }
}
