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

class EcosystemInformationClusterEcosystemLocationStruct(
  val uniqueLocationID: String,
  val homeLocation: Optional<EcosystemInformationClusterHomeLocationStruct>,
  val homeLocationLastEdit: Optional<ULong>,
  val fabricIndex: UByte,
) {
  override fun toString(): String = buildString {
    append("EcosystemInformationClusterEcosystemLocationStruct {\n")
    append("\tuniqueLocationID : $uniqueLocationID\n")
    append("\thomeLocation : $homeLocation\n")
    append("\thomeLocationLastEdit : $homeLocationLastEdit\n")
    append("\tfabricIndex : $fabricIndex\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_UNIQUE_LOCATION_I_D), uniqueLocationID)
      if (homeLocation.isPresent) {
        val opthomeLocation = homeLocation.get()
        opthomeLocation.toTlv(ContextSpecificTag(TAG_HOME_LOCATION), this)
      }
      if (homeLocationLastEdit.isPresent) {
        val opthomeLocationLastEdit = homeLocationLastEdit.get()
        put(ContextSpecificTag(TAG_HOME_LOCATION_LAST_EDIT), opthomeLocationLastEdit)
      }
      put(ContextSpecificTag(TAG_FABRIC_INDEX), fabricIndex)
      endStructure()
    }
  }

  companion object {
    private const val TAG_UNIQUE_LOCATION_I_D = 0
    private const val TAG_HOME_LOCATION = 1
    private const val TAG_HOME_LOCATION_LAST_EDIT = 2
    private const val TAG_FABRIC_INDEX = 254

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): EcosystemInformationClusterEcosystemLocationStruct {
      tlvReader.enterStructure(tlvTag)
      val uniqueLocationID = tlvReader.getString(ContextSpecificTag(TAG_UNIQUE_LOCATION_I_D))
      val homeLocation =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_HOME_LOCATION))) {
          Optional.of(
            EcosystemInformationClusterHomeLocationStruct.fromTlv(
              ContextSpecificTag(TAG_HOME_LOCATION),
              tlvReader,
            )
          )
        } else {
          Optional.empty()
        }
      val homeLocationLastEdit =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_HOME_LOCATION_LAST_EDIT))) {
          Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_HOME_LOCATION_LAST_EDIT)))
        } else {
          Optional.empty()
        }
      val fabricIndex = tlvReader.getUByte(ContextSpecificTag(TAG_FABRIC_INDEX))

      tlvReader.exitContainer()

      return EcosystemInformationClusterEcosystemLocationStruct(
        uniqueLocationID,
        homeLocation,
        homeLocationLastEdit,
        fabricIndex,
      )
    }
  }
}
