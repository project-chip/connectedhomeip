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

class ElectricalProtectionAlarmClusterSurgeProtectionRatingsStruct(
  val tripMechanism: Optional<UInt>,
  val protectionClass: Optional<UInt>,
  val protectionType: Optional<UInt>,
  val maxContinuousOperatingVoltage: Optional<Long>,
  val maxVoltageProtection: Optional<Long>,
  val maxTemporaryVoltage: Optional<Long>,
  val nominalDischargeCurrent: Optional<Long>,
  val maximumDishargeCurrent: Optional<Long>,
  val ratedShortCircuitCurrent: Optional<Long>,
  val ratedShortTimeWithstandCurrent: Optional<Long>,
  val energyAbsorptionCapability: Optional<ULong>,
  val responseTime: Optional<ULong>,
) {
  override fun toString(): String = buildString {
    append("ElectricalProtectionAlarmClusterSurgeProtectionRatingsStruct {\n")
    append("\ttripMechanism : $tripMechanism\n")
    append("\tprotectionClass : $protectionClass\n")
    append("\tprotectionType : $protectionType\n")
    append("\tmaxContinuousOperatingVoltage : $maxContinuousOperatingVoltage\n")
    append("\tmaxVoltageProtection : $maxVoltageProtection\n")
    append("\tmaxTemporaryVoltage : $maxTemporaryVoltage\n")
    append("\tnominalDischargeCurrent : $nominalDischargeCurrent\n")
    append("\tmaximumDishargeCurrent : $maximumDishargeCurrent\n")
    append("\tratedShortCircuitCurrent : $ratedShortCircuitCurrent\n")
    append("\tratedShortTimeWithstandCurrent : $ratedShortTimeWithstandCurrent\n")
    append("\tenergyAbsorptionCapability : $energyAbsorptionCapability\n")
    append("\tresponseTime : $responseTime\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (tripMechanism.isPresent) {
        val opttripMechanism = tripMechanism.get()
        put(ContextSpecificTag(TAG_TRIP_MECHANISM), opttripMechanism)
      }
      if (protectionClass.isPresent) {
        val optprotectionClass = protectionClass.get()
        put(ContextSpecificTag(TAG_PROTECTION_CLASS), optprotectionClass)
      }
      if (protectionType.isPresent) {
        val optprotectionType = protectionType.get()
        put(ContextSpecificTag(TAG_PROTECTION_TYPE), optprotectionType)
      }
      if (maxContinuousOperatingVoltage.isPresent) {
        val optmaxContinuousOperatingVoltage = maxContinuousOperatingVoltage.get()
        put(
          ContextSpecificTag(TAG_MAX_CONTINUOUS_OPERATING_VOLTAGE),
          optmaxContinuousOperatingVoltage,
        )
      }
      if (maxVoltageProtection.isPresent) {
        val optmaxVoltageProtection = maxVoltageProtection.get()
        put(ContextSpecificTag(TAG_MAX_VOLTAGE_PROTECTION), optmaxVoltageProtection)
      }
      if (maxTemporaryVoltage.isPresent) {
        val optmaxTemporaryVoltage = maxTemporaryVoltage.get()
        put(ContextSpecificTag(TAG_MAX_TEMPORARY_VOLTAGE), optmaxTemporaryVoltage)
      }
      if (nominalDischargeCurrent.isPresent) {
        val optnominalDischargeCurrent = nominalDischargeCurrent.get()
        put(ContextSpecificTag(TAG_NOMINAL_DISCHARGE_CURRENT), optnominalDischargeCurrent)
      }
      if (maximumDishargeCurrent.isPresent) {
        val optmaximumDishargeCurrent = maximumDishargeCurrent.get()
        put(ContextSpecificTag(TAG_MAXIMUM_DISHARGE_CURRENT), optmaximumDishargeCurrent)
      }
      if (ratedShortCircuitCurrent.isPresent) {
        val optratedShortCircuitCurrent = ratedShortCircuitCurrent.get()
        put(ContextSpecificTag(TAG_RATED_SHORT_CIRCUIT_CURRENT), optratedShortCircuitCurrent)
      }
      if (ratedShortTimeWithstandCurrent.isPresent) {
        val optratedShortTimeWithstandCurrent = ratedShortTimeWithstandCurrent.get()
        put(
          ContextSpecificTag(TAG_RATED_SHORT_TIME_WITHSTAND_CURRENT),
          optratedShortTimeWithstandCurrent,
        )
      }
      if (energyAbsorptionCapability.isPresent) {
        val optenergyAbsorptionCapability = energyAbsorptionCapability.get()
        put(ContextSpecificTag(TAG_ENERGY_ABSORPTION_CAPABILITY), optenergyAbsorptionCapability)
      }
      if (responseTime.isPresent) {
        val optresponseTime = responseTime.get()
        put(ContextSpecificTag(TAG_RESPONSE_TIME), optresponseTime)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_TRIP_MECHANISM = 0
    private const val TAG_PROTECTION_CLASS = 1
    private const val TAG_PROTECTION_TYPE = 2
    private const val TAG_MAX_CONTINUOUS_OPERATING_VOLTAGE = 3
    private const val TAG_MAX_VOLTAGE_PROTECTION = 4
    private const val TAG_MAX_TEMPORARY_VOLTAGE = 5
    private const val TAG_NOMINAL_DISCHARGE_CURRENT = 6
    private const val TAG_MAXIMUM_DISHARGE_CURRENT = 7
    private const val TAG_RATED_SHORT_CIRCUIT_CURRENT = 8
    private const val TAG_RATED_SHORT_TIME_WITHSTAND_CURRENT = 9
    private const val TAG_ENERGY_ABSORPTION_CAPABILITY = 10
    private const val TAG_RESPONSE_TIME = 11

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): ElectricalProtectionAlarmClusterSurgeProtectionRatingsStruct {
      tlvReader.enterStructure(tlvTag)
      val tripMechanism =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_TRIP_MECHANISM))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_TRIP_MECHANISM)))
        } else {
          Optional.empty()
        }
      val protectionClass =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_PROTECTION_CLASS))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_PROTECTION_CLASS)))
        } else {
          Optional.empty()
        }
      val protectionType =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_PROTECTION_TYPE))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_PROTECTION_TYPE)))
        } else {
          Optional.empty()
        }
      val maxContinuousOperatingVoltage =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_MAX_CONTINUOUS_OPERATING_VOLTAGE))) {
          Optional.of(tlvReader.getLong(ContextSpecificTag(TAG_MAX_CONTINUOUS_OPERATING_VOLTAGE)))
        } else {
          Optional.empty()
        }
      val maxVoltageProtection =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_MAX_VOLTAGE_PROTECTION))) {
          Optional.of(tlvReader.getLong(ContextSpecificTag(TAG_MAX_VOLTAGE_PROTECTION)))
        } else {
          Optional.empty()
        }
      val maxTemporaryVoltage =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_MAX_TEMPORARY_VOLTAGE))) {
          Optional.of(tlvReader.getLong(ContextSpecificTag(TAG_MAX_TEMPORARY_VOLTAGE)))
        } else {
          Optional.empty()
        }
      val nominalDischargeCurrent =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_NOMINAL_DISCHARGE_CURRENT))) {
          Optional.of(tlvReader.getLong(ContextSpecificTag(TAG_NOMINAL_DISCHARGE_CURRENT)))
        } else {
          Optional.empty()
        }
      val maximumDishargeCurrent =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_MAXIMUM_DISHARGE_CURRENT))) {
          Optional.of(tlvReader.getLong(ContextSpecificTag(TAG_MAXIMUM_DISHARGE_CURRENT)))
        } else {
          Optional.empty()
        }
      val ratedShortCircuitCurrent =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_RATED_SHORT_CIRCUIT_CURRENT))) {
          Optional.of(tlvReader.getLong(ContextSpecificTag(TAG_RATED_SHORT_CIRCUIT_CURRENT)))
        } else {
          Optional.empty()
        }
      val ratedShortTimeWithstandCurrent =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_RATED_SHORT_TIME_WITHSTAND_CURRENT))) {
          Optional.of(tlvReader.getLong(ContextSpecificTag(TAG_RATED_SHORT_TIME_WITHSTAND_CURRENT)))
        } else {
          Optional.empty()
        }
      val energyAbsorptionCapability =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_ENERGY_ABSORPTION_CAPABILITY))) {
          Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_ENERGY_ABSORPTION_CAPABILITY)))
        } else {
          Optional.empty()
        }
      val responseTime =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_RESPONSE_TIME))) {
          Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_RESPONSE_TIME)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return ElectricalProtectionAlarmClusterSurgeProtectionRatingsStruct(
        tripMechanism,
        protectionClass,
        protectionType,
        maxContinuousOperatingVoltage,
        maxVoltageProtection,
        maxTemporaryVoltage,
        nominalDischargeCurrent,
        maximumDishargeCurrent,
        ratedShortCircuitCurrent,
        ratedShortTimeWithstandCurrent,
        energyAbsorptionCapability,
        responseTime,
      )
    }
  }
}
