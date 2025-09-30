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

class CommodityTariffClusterTariffInformationStruct (
    val tariffLabel: String?,
    val providerName: String?,
    val currency: Optional<CommodityTariffClusterCurrencyStruct>?,
    val blockMode: UInt?) {
  override fun toString(): String  = buildString {
    append("CommodityTariffClusterTariffInformationStruct {\n")
    append("\ttariffLabel : $tariffLabel\n")
    append("\tproviderName : $providerName\n")
    append("\tcurrency : $currency\n")
    append("\tblockMode : $blockMode\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (tariffLabel != null) {
      put(ContextSpecificTag(TAG_TARIFF_LABEL), tariffLabel)
    } else {
      putNull(ContextSpecificTag(TAG_TARIFF_LABEL))
    }
      if (providerName != null) {
      put(ContextSpecificTag(TAG_PROVIDER_NAME), providerName)
    } else {
      putNull(ContextSpecificTag(TAG_PROVIDER_NAME))
    }
      if (currency != null) {
      if (currency.isPresent) {
      val optcurrency = currency.get()
      optcurrency.toTlv(ContextSpecificTag(TAG_CURRENCY), this)
    }
    } else {
      putNull(ContextSpecificTag(TAG_CURRENCY))
    }
      if (blockMode != null) {
      put(ContextSpecificTag(TAG_BLOCK_MODE), blockMode)
    } else {
      putNull(ContextSpecificTag(TAG_BLOCK_MODE))
    }
      endStructure()
    }
  }

  companion object {
    private const val TAG_TARIFF_LABEL = 0
    private const val TAG_PROVIDER_NAME = 1
    private const val TAG_CURRENCY = 2
    private const val TAG_BLOCK_MODE = 3

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader) : CommodityTariffClusterTariffInformationStruct {
      tlvReader.enterStructure(tlvTag)
      val tariffLabel = if (!tlvReader.isNull()) {
      tlvReader.getString(ContextSpecificTag(TAG_TARIFF_LABEL))
    } else {
      tlvReader.getNull(ContextSpecificTag(TAG_TARIFF_LABEL))
      null
    }
      val providerName = if (!tlvReader.isNull()) {
      tlvReader.getString(ContextSpecificTag(TAG_PROVIDER_NAME))
    } else {
      tlvReader.getNull(ContextSpecificTag(TAG_PROVIDER_NAME))
      null
    }
      val currency = if (!tlvReader.isNull()) {
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_CURRENCY))) {
      Optional.of(CommodityTariffClusterCurrencyStruct.fromTlv(ContextSpecificTag(TAG_CURRENCY), tlvReader))
    } else {
      Optional.empty()
    }
    } else {
      tlvReader.getNull(ContextSpecificTag(TAG_CURRENCY))
      null
    }
      val blockMode = if (!tlvReader.isNull()) {
      tlvReader.getUInt(ContextSpecificTag(TAG_BLOCK_MODE))
    } else {
      tlvReader.getNull(ContextSpecificTag(TAG_BLOCK_MODE))
      null
    }
      
      tlvReader.exitContainer()

      return CommodityTariffClusterTariffInformationStruct(tariffLabel, providerName, currency, blockMode)
    }
  }
}
