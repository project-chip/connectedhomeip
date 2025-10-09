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

class ContentLauncherClusterBrandingInformationStruct(
  val providerName: String,
  val background: Optional<ContentLauncherClusterStyleInformationStruct>,
  val logo: Optional<ContentLauncherClusterStyleInformationStruct>,
  val progressBar: Optional<ContentLauncherClusterStyleInformationStruct>,
  val splash: Optional<ContentLauncherClusterStyleInformationStruct>,
  val waterMark: Optional<ContentLauncherClusterStyleInformationStruct>,
) {
  override fun toString(): String = buildString {
    append("ContentLauncherClusterBrandingInformationStruct {\n")
    append("\tproviderName : $providerName\n")
    append("\tbackground : $background\n")
    append("\tlogo : $logo\n")
    append("\tprogressBar : $progressBar\n")
    append("\tsplash : $splash\n")
    append("\twaterMark : $waterMark\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_PROVIDER_NAME), providerName)
      if (background.isPresent) {
        val optbackground = background.get()
        optbackground.toTlv(ContextSpecificTag(TAG_BACKGROUND), this)
      }
      if (logo.isPresent) {
        val optlogo = logo.get()
        optlogo.toTlv(ContextSpecificTag(TAG_LOGO), this)
      }
      if (progressBar.isPresent) {
        val optprogressBar = progressBar.get()
        optprogressBar.toTlv(ContextSpecificTag(TAG_PROGRESS_BAR), this)
      }
      if (splash.isPresent) {
        val optsplash = splash.get()
        optsplash.toTlv(ContextSpecificTag(TAG_SPLASH), this)
      }
      if (waterMark.isPresent) {
        val optwaterMark = waterMark.get()
        optwaterMark.toTlv(ContextSpecificTag(TAG_WATER_MARK), this)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_PROVIDER_NAME = 0
    private const val TAG_BACKGROUND = 1
    private const val TAG_LOGO = 2
    private const val TAG_PROGRESS_BAR = 3
    private const val TAG_SPLASH = 4
    private const val TAG_WATER_MARK = 5

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): ContentLauncherClusterBrandingInformationStruct {
      tlvReader.enterStructure(tlvTag)
      val providerName = tlvReader.getString(ContextSpecificTag(TAG_PROVIDER_NAME))
      val background =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_BACKGROUND))) {
          Optional.of(
            ContentLauncherClusterStyleInformationStruct.fromTlv(
              ContextSpecificTag(TAG_BACKGROUND),
              tlvReader,
            )
          )
        } else {
          Optional.empty()
        }
      val logo =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_LOGO))) {
          Optional.of(
            ContentLauncherClusterStyleInformationStruct.fromTlv(
              ContextSpecificTag(TAG_LOGO),
              tlvReader,
            )
          )
        } else {
          Optional.empty()
        }
      val progressBar =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_PROGRESS_BAR))) {
          Optional.of(
            ContentLauncherClusterStyleInformationStruct.fromTlv(
              ContextSpecificTag(TAG_PROGRESS_BAR),
              tlvReader,
            )
          )
        } else {
          Optional.empty()
        }
      val splash =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_SPLASH))) {
          Optional.of(
            ContentLauncherClusterStyleInformationStruct.fromTlv(
              ContextSpecificTag(TAG_SPLASH),
              tlvReader,
            )
          )
        } else {
          Optional.empty()
        }
      val waterMark =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_WATER_MARK))) {
          Optional.of(
            ContentLauncherClusterStyleInformationStruct.fromTlv(
              ContextSpecificTag(TAG_WATER_MARK),
              tlvReader,
            )
          )
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return ContentLauncherClusterBrandingInformationStruct(
        providerName,
        background,
        logo,
        progressBar,
        splash,
        waterMark,
      )
    }
  }
}
