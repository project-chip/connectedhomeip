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

class ChannelClusterChannelInfoStruct (
    val majorNumber: Int,
    val minorNumber: Int,
    val name: Optional<String>,
    val callSign: Optional<String>,
    val affiliateCallSign: Optional<String>) {
  override fun toString() : String {
    val builder: StringBuilder = StringBuilder()
    builder.append("ChannelClusterChannelInfoStruct {\n")
    builder.append("\tmajorNumber : $majorNumber\n")
    builder.append("\tminorNumber : $minorNumber\n")
    builder.append("\tname : $name\n")
    builder.append("\tcallSign : $callSign\n")
    builder.append("\taffiliateCallSign : $affiliateCallSign\n")
    builder.append("}\n")
    return builder.toString()
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.startStructure(tag)
    tlvWriter.put(ContextSpecificTag(0), majorNumber)
    tlvWriter.put(ContextSpecificTag(1), minorNumber)
    if (name.isPresent) {
      val optname = name.get()
      tlvWriter.put(ContextSpecificTag(2), optname)
    }
    if (callSign.isPresent) {
      val optcallSign = callSign.get()
      tlvWriter.put(ContextSpecificTag(3), optcallSign)
    }
    if (affiliateCallSign.isPresent) {
      val optaffiliateCallSign = affiliateCallSign.get()
      tlvWriter.put(ContextSpecificTag(4), optaffiliateCallSign)
    }
    tlvWriter.endStructure()
  }

  companion object {
    fun fromTlv(tag: Tag, tlvReader: TlvReader) : ChannelClusterChannelInfoStruct {
      tlvReader.enterStructure(tag)
      val majorNumber: Int = tlvReader.getInt(ContextSpecificTag(0))
      val minorNumber: Int = tlvReader.getInt(ContextSpecificTag(1))
      val name: Optional<String> = try {
      Optional.of(tlvReader.getString(ContextSpecificTag(2)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
      val callSign: Optional<String> = try {
      Optional.of(tlvReader.getString(ContextSpecificTag(3)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
      val affiliateCallSign: Optional<String> = try {
      Optional.of(tlvReader.getString(ContextSpecificTag(4)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
      
      tlvReader.exitContainer()

      return ChannelClusterChannelInfoStruct(majorNumber, minorNumber, name, callSign, affiliateCallSign)
    }
  }
}
