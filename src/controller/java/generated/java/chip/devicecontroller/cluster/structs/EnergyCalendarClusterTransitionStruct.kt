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

class EnergyCalendarClusterTransitionStruct(
  val transitionTime: UInt,
  val priceTier: Optional<ULong>,
  val friendlyCredit: Optional<Boolean>,
  val auxiliaryLoad: Optional<UInt>
) {
  override fun toString(): String = buildString {
    append("EnergyCalendarClusterTransitionStruct {\n")
    append("\ttransitionTime : $transitionTime\n")
    append("\tpriceTier : $priceTier\n")
    append("\tfriendlyCredit : $friendlyCredit\n")
    append("\tauxiliaryLoad : $auxiliaryLoad\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_TRANSITION_TIME), transitionTime)
      if (priceTier.isPresent) {
        val optpriceTier = priceTier.get()
        put(ContextSpecificTag(TAG_PRICE_TIER), optpriceTier)
      }
      if (friendlyCredit.isPresent) {
        val optfriendlyCredit = friendlyCredit.get()
        put(ContextSpecificTag(TAG_FRIENDLY_CREDIT), optfriendlyCredit)
      }
      if (auxiliaryLoad.isPresent) {
        val optauxiliaryLoad = auxiliaryLoad.get()
        put(ContextSpecificTag(TAG_AUXILIARY_LOAD), optauxiliaryLoad)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_TRANSITION_TIME = 0
    private const val TAG_PRICE_TIER = 1
    private const val TAG_FRIENDLY_CREDIT = 2
    private const val TAG_AUXILIARY_LOAD = 3

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): EnergyCalendarClusterTransitionStruct {
      tlvReader.enterStructure(tlvTag)
      val transitionTime = tlvReader.getUInt(ContextSpecificTag(TAG_TRANSITION_TIME))
      val priceTier =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_PRICE_TIER))) {
          Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_PRICE_TIER)))
        } else {
          Optional.empty()
        }
      val friendlyCredit =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_FRIENDLY_CREDIT))) {
          Optional.of(tlvReader.getBoolean(ContextSpecificTag(TAG_FRIENDLY_CREDIT)))
        } else {
          Optional.empty()
        }
      val auxiliaryLoad =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_AUXILIARY_LOAD))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_AUXILIARY_LOAD)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return EnergyCalendarClusterTransitionStruct(
        transitionTime,
        priceTier,
        friendlyCredit,
        auxiliaryLoad
      )
    }
  }
}
