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

import matter.controller.cluster.*
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class EcosystemInformationClusterEcosystemLocationStruct(
  val uniqueLocationID: String?,
  val homeLocation: EcosystemInformationClusterHomeLocationStruct?,
  val homeLocationLastEdit: ULong?,
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
      if (uniqueLocationID != null) {
        put(ContextSpecificTag(TAG_UNIQUE_LOCATION_I_D), uniqueLocationID)
      } else {
        putNull(ContextSpecificTag(TAG_UNIQUE_LOCATION_I_D))
      }
      if (homeLocation != null) {
        homeLocation.toTlv(ContextSpecificTag(TAG_HOME_LOCATION), this)
      } else {
        putNull(ContextSpecificTag(TAG_HOME_LOCATION))
      }
      if (homeLocationLastEdit != null) {
        put(ContextSpecificTag(TAG_HOME_LOCATION_LAST_EDIT), homeLocationLastEdit)
      } else {
        putNull(ContextSpecificTag(TAG_HOME_LOCATION_LAST_EDIT))
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
      val uniqueLocationID =
        if (!tlvReader.isNull()) {
          tlvReader.getString(ContextSpecificTag(TAG_UNIQUE_LOCATION_I_D))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_UNIQUE_LOCATION_I_D))
          null
        }
      val homeLocation =
        if (!tlvReader.isNull()) {
          EcosystemInformationClusterHomeLocationStruct.fromTlv(
            ContextSpecificTag(TAG_HOME_LOCATION),
            tlvReader,
          )
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_HOME_LOCATION))
          null
        }
      val homeLocationLastEdit =
        if (!tlvReader.isNull()) {
          tlvReader.getULong(ContextSpecificTag(TAG_HOME_LOCATION_LAST_EDIT))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_HOME_LOCATION_LAST_EDIT))
          null
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
