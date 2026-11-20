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

import java.util.Optional
import matter.controller.cluster.*
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class ContentLauncherClusterReplicationInfo(
  val launchUrlInfo: Optional<ContentLauncherClusterLaunchUrlInfo>?,
  val contentAppInfo: Optional<ContentLauncherClusterContentAppInfo>?,
) {
  override fun toString(): String = buildString {
    append("ContentLauncherClusterReplicationInfo {\n")
    append("\tlaunchUrlInfo : $launchUrlInfo\n")
    append("\tcontentAppInfo : $contentAppInfo\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (launchUrlInfo != null) {
        if (launchUrlInfo.isPresent) {
          val optlaunchUrlInfo = launchUrlInfo.get()
          optlaunchUrlInfo.toTlv(ContextSpecificTag(TAG_LAUNCH_URL_INFO), this)
        }
      } else {
        putNull(ContextSpecificTag(TAG_LAUNCH_URL_INFO))
      }
      if (contentAppInfo != null) {
        if (contentAppInfo.isPresent) {
          val optcontentAppInfo = contentAppInfo.get()
          optcontentAppInfo.toTlv(ContextSpecificTag(TAG_CONTENT_APP_INFO), this)
        }
      } else {
        putNull(ContextSpecificTag(TAG_CONTENT_APP_INFO))
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_LAUNCH_URL_INFO = 0
    private const val TAG_CONTENT_APP_INFO = 1

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ContentLauncherClusterReplicationInfo {
      tlvReader.enterStructure(tlvTag)
      val launchUrlInfo =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_LAUNCH_URL_INFO))) {
            Optional.of(
              ContentLauncherClusterLaunchUrlInfo.fromTlv(
                ContextSpecificTag(TAG_LAUNCH_URL_INFO),
                tlvReader,
              )
            )
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_LAUNCH_URL_INFO))
          null
        }
      val contentAppInfo =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_CONTENT_APP_INFO))) {
            Optional.of(
              ContentLauncherClusterContentAppInfo.fromTlv(
                ContextSpecificTag(TAG_CONTENT_APP_INFO),
                tlvReader,
              )
            )
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_CONTENT_APP_INFO))
          null
        }

      tlvReader.exitContainer()

      return ContentLauncherClusterReplicationInfo(launchUrlInfo, contentAppInfo)
    }
  }
}
