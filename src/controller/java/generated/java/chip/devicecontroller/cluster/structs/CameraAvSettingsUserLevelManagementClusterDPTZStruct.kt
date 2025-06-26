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

class CameraAvSettingsUserLevelManagementClusterDPTZStruct(
  val videoStreamID: UInt,
  val viewport: CameraAvSettingsUserLevelManagementClusterViewportStruct,
) {
  override fun toString(): String = buildString {
    append("CameraAvSettingsUserLevelManagementClusterDPTZStruct {\n")
    append("\tvideoStreamID : $videoStreamID\n")
    append("\tviewport : $viewport\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_VIDEO_STREAM_ID), videoStreamID)
      viewport.toTlv(ContextSpecificTag(TAG_VIEWPORT), this)
      endStructure()
    }
  }

  companion object {
    private const val TAG_VIDEO_STREAM_ID = 0
    private const val TAG_VIEWPORT = 1

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): CameraAvSettingsUserLevelManagementClusterDPTZStruct {
      tlvReader.enterStructure(tlvTag)
      val videoStreamID = tlvReader.getUInt(ContextSpecificTag(TAG_VIDEO_STREAM_ID))
      val viewport =
        CameraAvSettingsUserLevelManagementClusterViewportStruct.fromTlv(
          ContextSpecificTag(TAG_VIEWPORT),
          tlvReader,
        )

      tlvReader.exitContainer()

      return CameraAvSettingsUserLevelManagementClusterDPTZStruct(videoStreamID, viewport)
    }
  }
}
