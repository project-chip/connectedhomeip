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
package matter.controller.cluster.eventstructs

import matter.controller.cluster.*
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class OtaSoftwareUpdateRequestorClusterDownloadErrorEvent(
  val softwareVersion: UInt,
  val bytesDownloaded: ULong,
  val progressPercent: UByte?,
  val platformCode: Long?,
) {
  override fun toString(): String = buildString {
    append("OtaSoftwareUpdateRequestorClusterDownloadErrorEvent {\n")
    append("\tsoftwareVersion : $softwareVersion\n")
    append("\tbytesDownloaded : $bytesDownloaded\n")
    append("\tprogressPercent : $progressPercent\n")
    append("\tplatformCode : $platformCode\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
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
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): OtaSoftwareUpdateRequestorClusterDownloadErrorEvent {
      tlvReader.enterStructure(tlvTag)
      val softwareVersion = tlvReader.getUInt(ContextSpecificTag(TAG_SOFTWARE_VERSION))
      val bytesDownloaded = tlvReader.getULong(ContextSpecificTag(TAG_BYTES_DOWNLOADED))
      val progressPercent =
        if (!tlvReader.isNull()) {
          tlvReader.getUByte(ContextSpecificTag(TAG_PROGRESS_PERCENT))
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
        platformCode,
      )
    }
  }
}
