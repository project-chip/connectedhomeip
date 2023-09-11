/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
package chip.devicecontroller.cluster.structs

import chip.devicecontroller.cluster.*
import chip.tlv.ContextSpecificTag
import chip.tlv.Tag
import chip.tlv.TlvReader
import chip.tlv.TlvWriter
import java.util.Optional

class ChannelClusterChannelInfoStruct(
  val majorNumber: Int,
  val minorNumber: Int,
  val name: Optional<String>,
  val callSign: Optional<String>,
  val affiliateCallSign: Optional<String>
) {
  override fun toString(): String = buildString {
    append("ChannelClusterChannelInfoStruct {\n")
    append("\tmajorNumber : $majorNumber\n")
    append("\tminorNumber : $minorNumber\n")
    append("\tname : $name\n")
    append("\tcallSign : $callSign\n")
    append("\taffiliateCallSign : $affiliateCallSign\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
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
      endStructure()
    }
  }

  companion object {
    private const val TAG_MAJOR_NUMBER = 0
    private const val TAG_MINOR_NUMBER = 1
    private const val TAG_NAME = 2
    private const val TAG_CALL_SIGN = 3
    private const val TAG_AFFILIATE_CALL_SIGN = 4

    fun fromTlv(tag: Tag, tlvReader: TlvReader): ChannelClusterChannelInfoStruct {
      tlvReader.enterStructure(tag)
      val majorNumber = tlvReader.getInt(ContextSpecificTag(TAG_MAJOR_NUMBER))
      val minorNumber = tlvReader.getInt(ContextSpecificTag(TAG_MINOR_NUMBER))
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

      tlvReader.exitContainer()

      return ChannelClusterChannelInfoStruct(
        majorNumber,
        minorNumber,
        name,
        callSign,
        affiliateCallSign
      )
    }
  }
}
