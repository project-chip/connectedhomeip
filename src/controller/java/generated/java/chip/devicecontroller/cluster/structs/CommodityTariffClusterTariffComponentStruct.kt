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
import matter.tlv.TlvParsingException
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

import java.util.Optional

class CommodityTariffClusterTariffComponentStruct (
    val tariffComponentID: ULong,
    val price: Optional<CommodityTariffClusterTariffPriceStruct>?,
    val friendlyCredit: Optional<Boolean>,
    val auxiliaryLoad: Optional<CommodityTariffClusterAuxiliaryLoadSwitchSettingsStruct>,
    val peakPeriod: Optional<CommodityTariffClusterPeakPeriodStruct>,
    val powerThreshold: Optional<CommodityTariffClusterPowerThresholdStruct>,
    val threshold: ULong?,
    val label: Optional<String>?,
    val predicted: Optional<Boolean>) {
  override fun toString(): String  = buildString {
    append("CommodityTariffClusterTariffComponentStruct {\n")
    append("\ttariffComponentID : $tariffComponentID\n")
    append("\tprice : $price\n")
    append("\tfriendlyCredit : $friendlyCredit\n")
    append("\tauxiliaryLoad : $auxiliaryLoad\n")
    append("\tpeakPeriod : $peakPeriod\n")
    append("\tpowerThreshold : $powerThreshold\n")
    append("\tthreshold : $threshold\n")
    append("\tlabel : $label\n")
    append("\tpredicted : $predicted\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_TARIFF_COMPONENT_ID), tariffComponentID)
      if (price != null) {
      if (price.isPresent) {
      val optprice = price.get()
      optprice.toTlv(ContextSpecificTag(TAG_PRICE), this)
    }
    } else {
      putNull(ContextSpecificTag(TAG_PRICE))
    }
      if (friendlyCredit.isPresent) {
      val optfriendlyCredit = friendlyCredit.get()
      put(ContextSpecificTag(TAG_FRIENDLY_CREDIT), optfriendlyCredit)
    }
      if (auxiliaryLoad.isPresent) {
      val optauxiliaryLoad = auxiliaryLoad.get()
      optauxiliaryLoad.toTlv(ContextSpecificTag(TAG_AUXILIARY_LOAD), this)
    }
      if (peakPeriod.isPresent) {
      val optpeakPeriod = peakPeriod.get()
      optpeakPeriod.toTlv(ContextSpecificTag(TAG_PEAK_PERIOD), this)
    }
      if (powerThreshold.isPresent) {
      val optpowerThreshold = powerThreshold.get()
      optpowerThreshold.toTlv(ContextSpecificTag(TAG_POWER_THRESHOLD), this)
    }
      if (threshold != null) {
      put(ContextSpecificTag(TAG_THRESHOLD), threshold)
    } else {
      putNull(ContextSpecificTag(TAG_THRESHOLD))
    }
      if (label != null) {
      if (label.isPresent) {
      val optlabel = label.get()
      put(ContextSpecificTag(TAG_LABEL), optlabel)
    }
    } else {
      putNull(ContextSpecificTag(TAG_LABEL))
    }
      if (predicted.isPresent) {
      val optpredicted = predicted.get()
      put(ContextSpecificTag(TAG_PREDICTED), optpredicted)
    }
      endStructure()
    }
  }

  companion object {
    private const val TAG_TARIFF_COMPONENT_ID = 0
    private const val TAG_PRICE = 1
    private const val TAG_FRIENDLY_CREDIT = 2
    private const val TAG_AUXILIARY_LOAD = 3
    private const val TAG_PEAK_PERIOD = 4
    private const val TAG_POWER_THRESHOLD = 5
    private const val TAG_THRESHOLD = 6
    private const val TAG_LABEL = 7
    private const val TAG_PREDICTED = 8

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader) : CommodityTariffClusterTariffComponentStruct {
      tlvReader.enterStructure(tlvTag)
      val tariffComponentID = tlvReader.getULong(ContextSpecificTag(TAG_TARIFF_COMPONENT_ID))
      val price = if (!tlvReader.isNull()) {
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_PRICE))) {
      Optional.of(CommodityTariffClusterTariffPriceStruct.fromTlv(ContextSpecificTag(TAG_PRICE), tlvReader))
    } else {
      Optional.empty()
    }
    } else {
      tlvReader.getNull(ContextSpecificTag(TAG_PRICE))
      null
    }
      val friendlyCredit = if (tlvReader.isNextTag(ContextSpecificTag(TAG_FRIENDLY_CREDIT))) {
      Optional.of(tlvReader.getBoolean(ContextSpecificTag(TAG_FRIENDLY_CREDIT)))
    } else {
      Optional.empty()
    }
      val auxiliaryLoad = if (tlvReader.isNextTag(ContextSpecificTag(TAG_AUXILIARY_LOAD))) {
      Optional.of(CommodityTariffClusterAuxiliaryLoadSwitchSettingsStruct.fromTlv(ContextSpecificTag(TAG_AUXILIARY_LOAD), tlvReader))
    } else {
      Optional.empty()
    }
      val peakPeriod = if (tlvReader.isNextTag(ContextSpecificTag(TAG_PEAK_PERIOD))) {
      Optional.of(CommodityTariffClusterPeakPeriodStruct.fromTlv(ContextSpecificTag(TAG_PEAK_PERIOD), tlvReader))
    } else {
      Optional.empty()
    }
      val powerThreshold = if (tlvReader.isNextTag(ContextSpecificTag(TAG_POWER_THRESHOLD))) {
      Optional.of(CommodityTariffClusterPowerThresholdStruct.fromTlv(ContextSpecificTag(TAG_POWER_THRESHOLD), tlvReader))
    } else {
      Optional.empty()
    }
      val threshold = if (!tlvReader.isNull()) {
      tlvReader.getULong(ContextSpecificTag(TAG_THRESHOLD))
    } else {
      tlvReader.getNull(ContextSpecificTag(TAG_THRESHOLD))
      null
    }
      val label = if (!tlvReader.isNull()) {
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_LABEL))) {
      Optional.of(tlvReader.getString(ContextSpecificTag(TAG_LABEL)))
    } else {
      Optional.empty()
    }
    } else {
      tlvReader.getNull(ContextSpecificTag(TAG_LABEL))
      null
    }
      val predicted = if (tlvReader.isNextTag(ContextSpecificTag(TAG_PREDICTED))) {
      Optional.of(tlvReader.getBoolean(ContextSpecificTag(TAG_PREDICTED)))
    } else {
      Optional.empty()
    }
      
      tlvReader.exitContainer()

      return CommodityTariffClusterTariffComponentStruct(tariffComponentID, price, friendlyCredit, auxiliaryLoad, peakPeriod, powerThreshold, threshold, label, predicted)
    }
  }
}
