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
package chip.devicecontroller.cluster.eventstructs

import chip.devicecontroller.cluster.*
import chip.tlv.ContextSpecificTag
import chip.tlv.Tag
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

class OtaSoftwareUpdateRequestorClusterDownloadErrorEvent(
  val softwareVersion: Long,
  val bytesDownloaded: Long,
  val progressPercent: Int?,
  val platformCode: Long?
) {
  override fun toString(): String = buildString {
    append("OtaSoftwareUpdateRequestorClusterDownloadErrorEvent {\n")
    append("\tsoftwareVersion : $softwareVersion\n")
    append("\tbytesDownloaded : $bytesDownloaded\n")
    append("\tprogressPercent : $progressPercent\n")
    append("\tplatformCode : $platformCode\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_SOFTWARE_VERSION), softwareVersion)
      put(ContextSpecificTag(TAG_BYTES_DOWNLOADED), bytesDownloaded)
      if (progressPercent != null) {
        put(ContextSpecificTag(TAG_PROGRESS_PERCENT), progressPercent)
      } else {
        putNull(ContextSpecificTag(TAG_PROGRESS_PERCENT))
      }
      if (platformCode != null) {
        put(ContextSpecificTag(TAG_PLATFORM_CODE), platformCode)
      } else {
        putNull(ContextSpecificTag(TAG_PLATFORM_CODE))
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_SOFTWARE_VERSION = 0
    private const val TAG_BYTES_DOWNLOADED = 1
    private const val TAG_PROGRESS_PERCENT = 2
    private const val TAG_PLATFORM_CODE = 3

    fun fromTlv(
      tag: Tag,
      tlvReader: TlvReader
    ): OtaSoftwareUpdateRequestorClusterDownloadErrorEvent {
      tlvReader.enterStructure(tag)
      val softwareVersion = tlvReader.getLong(ContextSpecificTag(TAG_SOFTWARE_VERSION))
      val bytesDownloaded = tlvReader.getLong(ContextSpecificTag(TAG_BYTES_DOWNLOADED))
      val progressPercent =
        if (!tlvReader.isNull()) {
          tlvReader.getInt(ContextSpecificTag(TAG_PROGRESS_PERCENT))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_PROGRESS_PERCENT))
          null
        }
      val platformCode =
        if (!tlvReader.isNull()) {
          tlvReader.getLong(ContextSpecificTag(TAG_PLATFORM_CODE))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_PLATFORM_CODE))
          null
        }

      tlvReader.exitContainer()

      return OtaSoftwareUpdateRequestorClusterDownloadErrorEvent(
        softwareVersion,
        bytesDownloaded,
        progressPercent,
        platformCode
      )
    }
  }
}
