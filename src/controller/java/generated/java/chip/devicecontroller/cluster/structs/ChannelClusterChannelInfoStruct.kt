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
import java.util.Optional
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class ChannelClusterChannelInfoStruct(
  val majorNumber: UInt,
  val minorNumber: UInt,
  val name: Optional<String>,
  val callSign: Optional<String>,
  val affiliateCallSign: Optional<String>,
  val identifier: Optional<String>,
  val type: Optional<UInt>
) {
  override fun toString(): String = buildString {
    append("ChannelClusterChannelInfoStruct {\n")
    append("\tmajorNumber : $majorNumber\n")
    append("\tminorNumber : $minorNumber\n")
    append("\tname : $name\n")
    append("\tcallSign : $callSign\n")
    append("\taffiliateCallSign : $affiliateCallSign\n")
    append("\tidentifier : $identifier\n")
    append("\ttype : $type\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_MAJOR_NUMBER), majorNumber)
      put(ContextSpecificTag(TAG_MINOR_NUMBER), minorNumber)
      if (name.isPresent) {
        val optname = name.get()
        put(ContextSpecificTag(TAG_NAME), optname)
      }
      if (callSign.isPresent) {
        val optcallSign = callSign.get()
        put(ContextSpecificTag(TAG_CALL_SIGN), optcallSign)
      }
      if (affiliateCallSign.isPresent) {
        val optaffiliateCallSign = affiliateCallSign.get()
        put(ContextSpecificTag(TAG_AFFILIATE_CALL_SIGN), optaffiliateCallSign)
      }
      if (identifier.isPresent) {
        val optidentifier = identifier.get()
        put(ContextSpecificTag(TAG_IDENTIFIER), optidentifier)
      }
      if (type.isPresent) {
        val opttype = type.get()
        put(ContextSpecificTag(TAG_TYPE), opttype)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_MAJOR_NUMBER = 0
    private const val TAG_MINOR_NUMBER = 1
    private const val TAG_NAME = 2
    private const val TAG_CALL_SIGN = 3
    private const val TAG_AFFILIATE_CALL_SIGN = 4
    private const val TAG_IDENTIFIER = 5
    private const val TAG_TYPE = 6

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ChannelClusterChannelInfoStruct {
      tlvReader.enterStructure(tlvTag)
      val majorNumber = tlvReader.getUInt(ContextSpecificTag(TAG_MAJOR_NUMBER))
      val minorNumber = tlvReader.getUInt(ContextSpecificTag(TAG_MINOR_NUMBER))
      val name =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_NAME))) {
          Optional.of(tlvReader.getString(ContextSpecificTag(TAG_NAME)))
        } else {
          Optional.empty()
        }
      val callSign =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_CALL_SIGN))) {
          Optional.of(tlvReader.getString(ContextSpecificTag(TAG_CALL_SIGN)))
        } else {
          Optional.empty()
        }
      val affiliateCallSign =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_AFFILIATE_CALL_SIGN))) {
          Optional.of(tlvReader.getString(ContextSpecificTag(TAG_AFFILIATE_CALL_SIGN)))
        } else {
          Optional.empty()
        }
      val identifier =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_IDENTIFIER))) {
          Optional.of(tlvReader.getString(ContextSpecificTag(TAG_IDENTIFIER)))
        } else {
          Optional.empty()
        }
      val type =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_TYPE))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_TYPE)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return ChannelClusterChannelInfoStruct(
        majorNumber,
        minorNumber,
        name,
        callSign,
        affiliateCallSign,
        identifier,
        type
      )
    }
  }
}
