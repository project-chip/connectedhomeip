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

class ContentLauncherClusterStyleInformationStruct (
    val imageURL: Optional<String>,
    val color: Optional<String>,
    val size: Optional<ContentLauncherClusterDimensionStruct>) {
  override fun toString() : String {
    val builder: StringBuilder = StringBuilder()
    builder.append("ContentLauncherClusterStyleInformationStruct {\n")
    builder.append("\timageURL : $imageURL\n")
    builder.append("\tcolor : $color\n")
    builder.append("\tsize : $size\n")
    builder.append("}\n")
    return builder.toString()
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.startStructure(tag)
    if (imageURL.isPresent) {
      val optimageURL = imageURL.get()
      tlvWriter.put(ContextSpecificTag(0), optimageURL)
    }
    if (color.isPresent) {
      val optcolor = color.get()
      tlvWriter.put(ContextSpecificTag(1), optcolor)
    }
    if (size.isPresent) {
      val optsize = size.get()
      optsize.toTlv(ContextSpecificTag(2), tlvWriter)
    }
    tlvWriter.endStructure()
  }

  companion object {
    fun fromTlv(tag: Tag, tlvReader: TlvReader) : ContentLauncherClusterStyleInformationStruct {
      tlvReader.enterStructure(tag)
      val imageURL: Optional<String> = try {
      Optional.of(tlvReader.getString(ContextSpecificTag(0)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
      val color: Optional<String> = try {
      Optional.of(tlvReader.getString(ContextSpecificTag(1)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
      val size: Optional<ContentLauncherClusterDimensionStruct> = try {
      Optional.of(ContentLauncherClusterDimensionStruct.fromTlv(ContextSpecificTag(2), tlvReader))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
      
      tlvReader.exitContainer()

      return ContentLauncherClusterStyleInformationStruct(imageURL, color, size)
    }
  }
}
