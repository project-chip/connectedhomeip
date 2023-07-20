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
import chip.tlv.AnonymousTag
import chip.tlv.ContextSpecificTag
import chip.tlv.Tag
import chip.tlv.TlvParsingException
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

import java.util.Optional

class ContentLauncherClusterBrandingInformationStruct (
    val providerName: String,
    val background: Optional<ContentLauncherClusterStyleInformationStruct>,
    val logo: Optional<ContentLauncherClusterStyleInformationStruct>,
    val progressBar: Optional<ContentLauncherClusterStyleInformationStruct>,
    val splash: Optional<ContentLauncherClusterStyleInformationStruct>,
    val waterMark: Optional<ContentLauncherClusterStyleInformationStruct>) {
  override fun toString() : String {
    val builder: StringBuilder = StringBuilder()
    builder.append("ContentLauncherClusterBrandingInformationStruct {\n")
    builder.append("\tproviderName : $providerName\n")
    builder.append("\tbackground : $background\n")
    builder.append("\tlogo : $logo\n")
    builder.append("\tprogressBar : $progressBar\n")
    builder.append("\tsplash : $splash\n")
    builder.append("\twaterMark : $waterMark\n")
    builder.append("}\n")
    return builder.toString()
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.startStructure(tag)
    tlvWriter.put(ContextSpecificTag(TAG_PROVIDER_NAME), providerName)
    if (background.isPresent) {
      val optbackground = background.get()
      optbackground.toTlv(ContextSpecificTag(TAG_BACKGROUND), tlvWriter)
    }
    if (logo.isPresent) {
      val optlogo = logo.get()
      optlogo.toTlv(ContextSpecificTag(TAG_LOGO), tlvWriter)
    }
    if (progressBar.isPresent) {
      val optprogressBar = progressBar.get()
      optprogressBar.toTlv(ContextSpecificTag(TAG_PROGRESS_BAR), tlvWriter)
    }
    if (splash.isPresent) {
      val optsplash = splash.get()
      optsplash.toTlv(ContextSpecificTag(TAG_SPLASH), tlvWriter)
    }
    if (waterMark.isPresent) {
      val optwaterMark = waterMark.get()
      optwaterMark.toTlv(ContextSpecificTag(TAG_WATER_MARK), tlvWriter)
    }
    tlvWriter.endStructure()
  }

  companion object {
    private const val TAG_PROVIDER_NAME = 0
    private const val TAG_BACKGROUND = 1
    private const val TAG_LOGO = 2
    private const val TAG_PROGRESS_BAR = 3
    private const val TAG_SPLASH = 4
    private const val TAG_WATER_MARK = 5

    fun fromTlv(tag: Tag, tlvReader: TlvReader) : ContentLauncherClusterBrandingInformationStruct {
      tlvReader.enterStructure(tag)
      val providerName: String = tlvReader.getString(ContextSpecificTag(TAG_PROVIDER_NAME))
      val background: Optional<ContentLauncherClusterStyleInformationStruct> = try {
      Optional.of(ContentLauncherClusterStyleInformationStruct.fromTlv(ContextSpecificTag(TAG_BACKGROUND), tlvReader))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
      val logo: Optional<ContentLauncherClusterStyleInformationStruct> = try {
      Optional.of(ContentLauncherClusterStyleInformationStruct.fromTlv(ContextSpecificTag(TAG_LOGO), tlvReader))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
      val progressBar: Optional<ContentLauncherClusterStyleInformationStruct> = try {
      Optional.of(ContentLauncherClusterStyleInformationStruct.fromTlv(ContextSpecificTag(TAG_PROGRESS_BAR), tlvReader))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
      val splash: Optional<ContentLauncherClusterStyleInformationStruct> = try {
      Optional.of(ContentLauncherClusterStyleInformationStruct.fromTlv(ContextSpecificTag(TAG_SPLASH), tlvReader))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
      val waterMark: Optional<ContentLauncherClusterStyleInformationStruct> = try {
      Optional.of(ContentLauncherClusterStyleInformationStruct.fromTlv(ContextSpecificTag(TAG_WATER_MARK), tlvReader))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
      
      tlvReader.exitContainer()

      return ContentLauncherClusterBrandingInformationStruct(providerName, background, logo, progressBar, splash, waterMark)
    }
  }
}
