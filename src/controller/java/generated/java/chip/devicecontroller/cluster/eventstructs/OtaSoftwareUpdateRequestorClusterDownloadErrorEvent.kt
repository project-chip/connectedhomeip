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
import chip.tlv.Tag
import chip.tlv.AnonymousTag
import chip.tlv.ContextSpecificTag
import chip.tlv.TlvParsingException
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

import java.util.Optional

class OtaSoftwareUpdateRequestorClusterDownloadErrorEvent (
    val softwareVersion: Long,
    val bytesDownloaded: Long,
    val progressPercent: Int?,
    val platformCode: Long?) {
  override fun toString() : String {
    val builder: StringBuilder = StringBuilder()
    builder.append("OtaSoftwareUpdateRequestorClusterDownloadErrorEvent {\n")
    builder.append("\tsoftwareVersion : $softwareVersion\n")
    builder.append("\tbytesDownloaded : $bytesDownloaded\n")
    builder.append("\tprogressPercent : $progressPercent\n")
    builder.append("\tplatformCode : $platformCode\n")
    builder.append("}\n")
    return builder.toString()
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.startStructure(tag)
    tlvWriter.put(ContextSpecificTag(0), softwareVersion)
    tlvWriter.put(ContextSpecificTag(1), bytesDownloaded)
    if (progressPercent == null) { tlvWriter.putNull(ContextSpecificTag(2)) }
    else {
      tlvWriter.put(ContextSpecificTag(2), progressPercent)
    }
    if (platformCode == null) { tlvWriter.putNull(ContextSpecificTag(3)) }
    else {
      tlvWriter.put(ContextSpecificTag(3), platformCode)
    }
    tlvWriter.endStructure()
  }

  companion object {
    fun fromTlv(tag: Tag, tlvReader: TlvReader) : OtaSoftwareUpdateRequestorClusterDownloadErrorEvent {
      tlvReader.enterStructure(tag)
      val softwareVersion: Long = tlvReader.getLong(ContextSpecificTag(0))
      val bytesDownloaded: Long = tlvReader.getLong(ContextSpecificTag(1))
      val progressPercent: Int? = try {
      tlvReader.getInt(ContextSpecificTag(2))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(2))
      null
    }
      val platformCode: Long? = try {
      tlvReader.getLong(ContextSpecificTag(3))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(3))
      null
    }
      
      tlvReader.exitContainer()

      return OtaSoftwareUpdateRequestorClusterDownloadErrorEvent(softwareVersion, bytesDownloaded, progressPercent, platformCode)
    }
  }
}
