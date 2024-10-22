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
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class CameraAvStreamManagementClusterAudioCapabilitiesStruct(
  val maxNumberOfChannels: UInt,
  val supportedCodecs: List<UInt>,
  val supportedSampleRates: List<ULong>,
  val supportedBitDepths: List<UInt>,
) {
  override fun toString(): String = buildString {
    append("CameraAvStreamManagementClusterAudioCapabilitiesStruct {\n")
    append("\tmaxNumberOfChannels : $maxNumberOfChannels\n")
    append("\tsupportedCodecs : $supportedCodecs\n")
    append("\tsupportedSampleRates : $supportedSampleRates\n")
    append("\tsupportedBitDepths : $supportedBitDepths\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_MAX_NUMBER_OF_CHANNELS), maxNumberOfChannels)
      startArray(ContextSpecificTag(TAG_SUPPORTED_CODECS))
      for (item in supportedCodecs.iterator()) {
        put(AnonymousTag, item)
      }
      endArray()
      startArray(ContextSpecificTag(TAG_SUPPORTED_SAMPLE_RATES))
      for (item in supportedSampleRates.iterator()) {
        put(AnonymousTag, item)
      }
      endArray()
      startArray(ContextSpecificTag(TAG_SUPPORTED_BIT_DEPTHS))
      for (item in supportedBitDepths.iterator()) {
        put(AnonymousTag, item)
      }
      endArray()
      endStructure()
    }
  }

  companion object {
    private const val TAG_MAX_NUMBER_OF_CHANNELS = 0
    private const val TAG_SUPPORTED_CODECS = 1
    private const val TAG_SUPPORTED_SAMPLE_RATES = 2
    private const val TAG_SUPPORTED_BIT_DEPTHS = 3

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): CameraAvStreamManagementClusterAudioCapabilitiesStruct {
      tlvReader.enterStructure(tlvTag)
      val maxNumberOfChannels = tlvReader.getUInt(ContextSpecificTag(TAG_MAX_NUMBER_OF_CHANNELS))
      val supportedCodecs =
        buildList<UInt> {
          tlvReader.enterArray(ContextSpecificTag(TAG_SUPPORTED_CODECS))
          while (!tlvReader.isEndOfContainer()) {
            add(tlvReader.getUInt(AnonymousTag))
          }
          tlvReader.exitContainer()
        }
      val supportedSampleRates =
        buildList<ULong> {
          tlvReader.enterArray(ContextSpecificTag(TAG_SUPPORTED_SAMPLE_RATES))
          while (!tlvReader.isEndOfContainer()) {
            add(tlvReader.getULong(AnonymousTag))
          }
          tlvReader.exitContainer()
        }
      val supportedBitDepths =
        buildList<UInt> {
          tlvReader.enterArray(ContextSpecificTag(TAG_SUPPORTED_BIT_DEPTHS))
          while (!tlvReader.isEndOfContainer()) {
            add(tlvReader.getUInt(AnonymousTag))
          }
          tlvReader.exitContainer()
        }

      tlvReader.exitContainer()

      return CameraAvStreamManagementClusterAudioCapabilitiesStruct(
        maxNumberOfChannels,
        supportedCodecs,
        supportedSampleRates,
        supportedBitDepths,
      )
    }
  }
}
