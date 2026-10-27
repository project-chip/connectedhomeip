/*
 *   Copyright (c) 2026 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
package com.google.chip.chiptool.devicemanagement

import chip.clusterinfo.CommandParameterInfo
import chip.clusterinfo.InteractionInfo
import chip.devicecontroller.ClusterIDMapping
import chip.devicecontroller.ClusterInfoMapping
import chip.devicecontroller.ClusterWriteMapping
import com.google.chip.chiptool.devicemanagement.MatterDataType.ARRAY
import com.google.chip.chiptool.devicemanagement.MatterDataType.BITMAP32
import com.google.chip.chiptool.devicemanagement.MatterDataType.BITMAP8
import com.google.chip.chiptool.devicemanagement.MatterDataType.BOOLEAN
import com.google.chip.chiptool.devicemanagement.MatterDataType.ENUM8
import com.google.chip.chiptool.devicemanagement.MatterDataType.FLOAT32
import com.google.chip.chiptool.devicemanagement.MatterDataType.FLOAT64
import com.google.chip.chiptool.devicemanagement.MatterDataType.INT16
import com.google.chip.chiptool.devicemanagement.MatterDataType.INT64
import com.google.chip.chiptool.devicemanagement.MatterDataType.INT8
import com.google.chip.chiptool.devicemanagement.MatterDataType.OCTET_STRING
import com.google.chip.chiptool.devicemanagement.MatterDataType.UINT16
import com.google.chip.chiptool.devicemanagement.MatterDataType.UINT32
import com.google.chip.chiptool.devicemanagement.MatterDataType.UINT8
import com.google.chip.chiptool.devicemanagement.MatterDataType.UTF8_STRING
import java.util.Locale
import java.util.Optional
import kotlin.math.roundToInt

/** Supported Matter Data Types in Interaction Model and TLV. */
enum class MatterDataType {
  BOOLEAN,
  INT8,
  INT16,
  INT32,
  INT64,
  UINT8,
  UINT16,
  UINT32,
  UINT64,
  FLOAT32,
  FLOAT64,
  UTF8_STRING,
  OCTET_STRING,
  ENUM8,
  ENUM16,
  BITMAP8,
  BITMAP16,
  BITMAP32,
  STRUCT,
  ARRAY,
  NULL,
}

/** Cluster functional categories for grouping and dynamic discovery. */
enum class ClusterCategory(val displayName: String) {
  LIGHTING_AND_POWER("Lighting & Power"),
  CLOSURES("Closures"),
  HVAC("HVAC & Climate"),
  MEDIA("Media & Entertainment"),
  APPLIANCES_AND_ROBOTICS("Appliances & Robotics"),
  ENERGY_MANAGEMENT("Energy Management"),
  SENSORS_AND_ALARMS("Sensors & Alarms"),
  GENERAL_AND_SYSTEM("General & System"),
}

/** Direction of command communication. */
enum class CommandDirection {
  CLIENT_TO_SERVER,
  SERVER_TO_CLIENT,
}

/** Metadata for a field inside a command or struct. */
data class MatterFieldMeta(
  val tagId: Int,
  val name: String,
  val type: MatterDataType,
  val isOptional: Boolean = false,
  val isNullable: Boolean = false,
  val description: String = "",
  val defaultValue: Any? = null,
  val enumValues: Map<String, Long> = emptyMap(),
  val minVal: Double? = null,
  val maxVal: Double? = null,
  val unit: String? = null,
)

/** Metadata for a Matter cluster command. */
data class MatterCommandMeta(
  val commandId: Long,
  val name: String,
  val description: String,
  val direction: CommandDirection = CommandDirection.CLIENT_TO_SERVER,
  val requestFields: List<MatterFieldMeta> = emptyList(),
  val responseCommandId: Long? = null,
  val isTimed: Boolean = false,
)

/** Metadata for a Matter cluster attribute. */
data class MatterAttributeMeta(
  val attributeId: Long,
  val name: String,
  val type: MatterDataType,
  val isWritable: Boolean = false,
  val isReportable: Boolean = true,
  val isNullable: Boolean = false,
  val description: String = "",
  val defaultValue: Any? = null,
  val enumValues: Map<String, Long> = emptyMap(),
  val unit: String? = null,
)

/** Metadata for a Matter cluster event. */
data class MatterEventMeta(
  val eventId: Long,
  val name: String,
  val description: String = "",
  val fields: List<MatterFieldMeta> = emptyList(),
)

/** Metadata for a complete Matter Cluster. */
data class MatterClusterMeta(
  val clusterId: Long,
  val name: String,
  val category: ClusterCategory,
  val description: String,
  val commands: Map<Long, MatterCommandMeta> = emptyMap(),
  val attributes: Map<Long, MatterAttributeMeta> = emptyMap(),
  val events: Map<Long, MatterEventMeta> = emptyMap(),
)

/**
 * Universal Matter Cluster Schema Metamodel Registry. Dynamically introspects the ZAP-generated
 * Matter SDK classes ([ClusterIDMapping], [ClusterInfoMapping], and [ClusterWriteMapping]) packaged
 * in `CHIPClusterID.jar` and `CHIPInteractionModel.jar`, enriched with a compact semantic overlay.
 */
object MatterClusterMetaRegistry {

  const val CLUSTER_IDENTIFY = 0x0003L
  const val CLUSTER_GROUPS = 0x0004L
  const val CLUSTER_ON_OFF = 0x0006L
  const val CLUSTER_LEVEL_CONTROL = 0x0008L
  const val CLUSTER_COLOR_CONTROL = 0x0300L
  const val CLUSTER_DOOR_LOCK = 0x0101L
  const val CLUSTER_WINDOW_COVERING = 0x0102L
  const val CLUSTER_BARRIER_CONTROL = 0x0103L
  const val CLUSTER_THERMOSTAT = 0x0201L
  const val CLUSTER_FAN_CONTROL = 0x0202L
  const val CLUSTER_MEDIA_PLAYBACK = 0x0506L
  const val CLUSTER_KEYPAD_INPUT = 0x0509L
  const val CLUSTER_AUDIO_OUTPUT = 0x050BL
  const val CLUSTER_MODE_SELECT = 0x0050L
  const val CLUSTER_RVC_RUN_MODE = 0x0054L
  const val CLUSTER_RVC_CLEAN_MODE = 0x0055L
  const val CLUSTER_OPERATIONAL_STATE = 0x0060L
  const val CLUSTER_DEVICE_ENERGY_MGMT = 0x0098L
  const val CLUSTER_DEVICE_ENERGY_MANAGEMENT = 0x0098L
  const val CLUSTER_ENERGY_EVSE = 0x0099L
  const val CLUSTER_BOOLEAN_STATE = 0x0045L
  const val CLUSTER_DESCRIPTOR = 0x001DL
  const val CLUSTER_ACCESS_CONTROL = 0x001FL
  const val CLUSTER_BASIC_INFORMATION = 0x0028L
  const val CLUSTER_OTA_REQUESTOR = 0x002AL
  const val CLUSTER_POWER_SOURCE = 0x002FL
  const val CLUSTER_GENERAL_COMMISSIONING = 0x0030L
  const val CLUSTER_NETWORK_COMMISSIONING = 0x0031L
  const val CLUSTER_GENERAL_DIAGNOSTICS = 0x0033L
  const val CLUSTER_SOFTWARE_DIAGNOSTICS = 0x0034L
  const val CLUSTER_THREAD_NETWORK_DIAGNOSTICS = 0x0035L
  const val CLUSTER_WIFI_NETWORK_DIAGNOSTICS = 0x0036L
  const val CLUSTER_SWITCH = 0x003BL
  const val CLUSTER_ADMINISTRATOR_COMMISSIONING = 0x003CL
  const val CLUSTER_OPERATIONAL_CREDENTIALS = 0x003EL
  const val CLUSTER_GROUP_KEY_MANAGEMENT = 0x003FL
  const val CLUSTER_ICD_MANAGEMENT = 0x0046L
  const val CLUSTER_SMOKE_CO_ALARM = 0x005CL
  const val CLUSTER_TEMPERATURE_MEASUREMENT = 0x0402L
  const val CLUSTER_RELATIVE_HUMIDITY_MEASUREMENT = 0x0405L
  const val CLUSTER_OCCUPANCY_SENSING = 0x0406L
  const val CLUSTER_ILLUMINANCE_MEASUREMENT = 0x0400L

  private val registry = LinkedHashMap<Long, MatterClusterMeta>()

  init {
    loadFromGeneratedMatterSdk()
    applySemanticOverlays()
  }

  fun getCluster(clusterId: Long): MatterClusterMeta? = registry[clusterId]

  fun getClusterOrDefault(clusterId: Long): MatterClusterMeta {
    return registry[clusterId]
      ?: MatterClusterMeta(
        clusterId = clusterId,
        name = "Cluster 0x${clusterId.toString(16).uppercase()}",
        category = ClusterCategory.GENERAL_AND_SYSTEM,
        description = "Unknown / Vendor Matter Cluster 0x${clusterId.toString(16).uppercase()}",
        attributes =
          mapOf(
            0xFFFBL to MatterAttributeMeta(0xFFFBL, "AttributeList", ARRAY),
            0xFFFCL to MatterAttributeMeta(0xFFFCL, "FeatureMap", BITMAP32),
            0xFFFDL to MatterAttributeMeta(0xFFFDL, "ClusterRevision", UINT16),
          ),
      )
  }

  /** Looks up a cluster by its spec name, ignoring case, spaces, and underscores. */
  fun getClusterByName(name: String): MatterClusterMeta? {
    val cleanName = name.replace(" ", "").replace("_", "").lowercase(Locale.US)
    return registry.values.firstOrNull {
      it.name.replace(" ", "").replace("_", "").lowercase(Locale.US) == cleanName
    }
  }

  fun getAllClusters(): List<MatterClusterMeta> = registry.values.toList()

  fun getClustersByCategory(category: ClusterCategory): List<MatterClusterMeta> =
    registry.values.filter { it.category == category }

  fun categoryDisplayName(clusterId: Long): String =
    getCluster(clusterId)?.category?.displayName ?: "System / Custom"

  fun registerCluster(cluster: MatterClusterMeta) {
    registry[cluster.clusterId] = cluster
  }

  fun kelvinToMireds(kelvin: Int): Int {
    require(kelvin > 0) { "Kelvin must be positive" }
    return (1_000_000.0 / kelvin).roundToInt()
  }

  fun miredsToKelvin(mireds: Int): Int {
    require(mireds > 0) { "Mireds must be positive" }
    return (1_000_000.0 / mireds).roundToInt()
  }

  fun celsiusToCentidegrees(celsius: Double): Short = (celsius * 100.0).roundToInt().toShort()

  fun centidegreesToCelsius(centidegrees: Short): Double = centidegrees.toDouble() / 100.0

  fun percentageToLevel(percentage: Double): UByte =
    ((percentage.coerceIn(0.0, 100.0) * 254.0) / 100.0).roundToInt().toUByte()

  fun levelToPercentage(level: UByte): Double =
    (level.toDouble() * 100.0 / 254.0).coerceIn(0.0, 100.0)

  fun batHalfPercentToPercent(rawHalfPercent: Number): Int =
    (rawHalfPercent.toInt() / 2).coerceIn(0, 100)

  private fun loadFromGeneratedMatterSdk() {
    val cmdMap = HashMap<String, Map<String, InteractionInfo>>()
    val writeMap = HashMap<String, Map<String, InteractionInfo>>()
    try {
      for ((k, v) in ClusterInfoMapping().commandMap) cmdMap[k.lowercase(Locale.US)] = v
      for ((k, v) in ClusterWriteMapping().writeAttributeMap) writeMap[k.lowercase(Locale.US)] = v
    } catch (_: Throwable) {}

    for (cls in ClusterIDMapping::class.java.declaredClasses) {
      if (!ClusterIDMapping.BaseCluster::class.java.isAssignableFrom(cls) || cls.isInterface) {
        continue
      }
      val cid = runCatching { cls.getDeclaredField("ID").getLong(null) }.getOrNull() ?: continue
      val rawName = cls.simpleName
      val interactions = cmdMap[rawName.lowercase(Locale.US)].orEmpty()
      val writes = writeMap[rawName.lowercase(Locale.US)].orEmpty()
      val nested = cls.declaredClasses.associateBy { it.simpleName }

      val attributes = LinkedHashMap<Long, MatterAttributeMeta>()
      nested["Attribute"]?.enumConstants?.forEach { constant ->
        val attrId = invokeGetId(constant)
        val attrName = (constant as Enum<*>).name
        val readInfo = interactions["read${attrName}Attribute"]
        val writeInfo = writes["write${attrName}Attribute"]
        val paramInfo = writeInfo?.commandParameters?.get("value")
        attributes[attrId] =
          MatterAttributeMeta(
            attributeId = attrId,
            name = attrName,
            type = inferAttributeType(cid, attrId, attrName, readInfo, paramInfo),
            isWritable = writeInfo != null,
            isNullable = paramInfo?.type == Optional::class.java,
            description = "$rawName $attrName",
            unit = inferUnit(cid, attrName),
          )
      }

      val commands = LinkedHashMap<Long, MatterCommandMeta>()
      nested["Command"]?.enumConstants?.forEach { constant ->
        val cmdId = invokeGetId(constant)
        val cmdName = (constant as Enum<*>).name
        val cmdInfo = interactions[cmdName.replaceFirstChar { it.lowercase(Locale.US) }]
        val paramsByLower =
          cmdInfo?.commandParameters?.entries?.associate {
            it.key.lowercase(Locale.US) to (it.key to it.value)
          }
            ?: emptyMap()
        val fields = mutableListOf<MatterFieldMeta>()
        nested["${cmdName}CommandField"]?.enumConstants?.forEach { fieldConst ->
          val fieldEnum = fieldConst as Enum<*>
          val tagId = invokeGetId(fieldConst).toInt()
          val matched = paramsByLower[fieldEnum.name.lowercase(Locale.US)]
          val fName = matched?.first ?: fieldEnum.name.replaceFirstChar { it.lowercase(Locale.US) }
          val isDefOpt = fName in setOf("transitionTime", "optionsMask", "optionsOverride")
          fields.add(
            MatterFieldMeta(
              tagId = tagId,
              name = fName,
              type = inferFieldType(fName, matched?.second),
              isOptional = matched?.second?.type == Optional::class.java || isDefOpt,
              defaultValue = if (isDefOpt) 0 else null,
              description = "$cmdName $fName",
            )
          )
        }
        commands[cmdId] =
          MatterCommandMeta(cmdId, cmdName, "Invoke $cmdName", requestFields = fields)
      }

      val events = LinkedHashMap<Long, MatterEventMeta>()
      nested["Event"]?.enumConstants?.forEach { constant ->
        val eventId = invokeGetId(constant)
        val evName = (constant as Enum<*>).name
        events[eventId] = MatterEventMeta(eventId, evName, evName)
      }

      registry[cid] =
        MatterClusterMeta(
          clusterId = cid,
          name = if (cid == CLUSTER_ENERGY_EVSE) "EnergyEVSE" else rawName,
          category = categorizeById(cid),
          description = "Matter $rawName Cluster",
          commands = commands,
          attributes = attributes,
          events = events,
        )
    }
  }

  private fun invokeGetId(enumConstant: Any): Long =
    (enumConstant.javaClass.getMethod("getID").invoke(enumConstant) as Number).toLong()

  // These are functions rather than stored sets because the object's init block (which loads the
  // metamodel) runs before properties declared further down are initialized.

  /**
   * Attributes shared by the measurement clusters: MeasuredValue (0), MinMeasuredValue (1),
   * MaxMeasuredValue (2) and Tolerance (3). All but Tolerance use the same unit and scale.
   */
  private fun isMeasuredValueFamily(attrName: String) =
    attrName == "MeasuredValue" ||
      attrName == "MinMeasuredValue" ||
      attrName == "MaxMeasuredValue" ||
      attrName == "Tolerance"

  /** Measurement clusters whose Measured/Min/MaxMeasuredValue are int16. */
  private fun isSignedMeasurementCluster(cid: Long) =
    cid == CLUSTER_TEMPERATURE_MEASUREMENT || cid == 0x0403L // PressureMeasurement

  /** Measurement clusters whose Measured/Min/MaxMeasuredValue are uint16. */
  private fun isUnsignedMeasurementCluster(cid: Long) =
    cid == CLUSTER_ILLUMINANCE_MEASUREMENT ||
      cid == 0x0404L || // FlowMeasurement
      cid == CLUSTER_RELATIVE_HUMIDITY_MEASUREMENT

  private fun inferUnit(cid: Long, attrName: String): String? =
    when {
      cid == CLUSTER_TEMPERATURE_MEASUREMENT && isMeasuredValueFamily(attrName) -> "°C"
      cid == CLUSTER_RELATIVE_HUMIDITY_MEASUREMENT && isMeasuredValueFamily(attrName) -> "%"
      // Illuminance Tolerance is a delta on the log scale, so it has no lux unit.
      cid == CLUSTER_ILLUMINANCE_MEASUREMENT &&
        isMeasuredValueFamily(attrName) &&
        attrName != "Tolerance" -> "lx"
      cid == CLUSTER_THERMOSTAT &&
        (attrName.contains("Temperature") || attrName.contains("Setpoint")) &&
        !attrName.contains("Source") &&
        !attrName.contains("Timestamp") &&
        !attrName.contains("Calibration") -> "°C"
      cid == CLUSTER_FAN_CONTROL && attrName.startsWith("Percent") -> "%"
      cid == CLUSTER_POWER_SOURCE && attrName == "BatPercentRemaining" -> "%"
      else -> null
    }

  private fun inferAttributeType(
    cid: Long,
    attrId: Long,
    attrName: String,
    readInfo: InteractionInfo?,
    writeParam: CommandParameterInfo?,
  ): MatterDataType {
    if (attrId == 0xFFFBL || attrId == 0xFFF8L || attrId == 0xFFF9L) return ARRAY
    if (attrId == 0xFFFCL) return BITMAP32
    if (attrId == 0xFFFDL) return UINT16
    if (isSignedMeasurementCluster(cid) || isUnsignedMeasurementCluster(cid)) {
      when (attrId) {
        0L,
        1L,
        2L -> return if (isSignedMeasurementCluster(cid)) INT16 else UINT16
        3L -> return UINT16 // Tolerance
      }
    }
    if (cid == CLUSTER_OCCUPANCY_SENSING && attrId == 0L) return BITMAP8
    if ((cid == CLUSTER_ON_OFF || cid == CLUSTER_BOOLEAN_STATE) && attrId == 0L) return BOOLEAN
    val cbName =
      runCatching {
          readInfo?.commandCallbackSupplier?.get()?.javaClass?.interfaces?.firstOrNull()?.simpleName
        }
        .getOrNull()
        ?: ""
    val underlying = writeParam?.underlyingType ?: writeParam?.type
    return when {
      cbName.contains("Boolean") || underlying == Boolean::class.javaObjectType -> BOOLEAN
      cbName.contains("CharString") || underlying == String::class.java -> UTF8_STRING
      cbName.contains("OctetString") || underlying == ByteArray::class.java -> OCTET_STRING
      cbName.contains("Float") || underlying == Float::class.javaObjectType -> FLOAT32
      cbName.contains("Double") || underlying == Double::class.javaObjectType -> FLOAT64
      cbName.contains("List") || attrName.endsWith("List") -> ARRAY
      cid == CLUSTER_THERMOSTAT &&
        (attrName.contains("Temperature") || attrName.contains("Setpoint")) -> INT16
      (cid == CLUSTER_ENERGY_EVSE || cid == CLUSTER_DEVICE_ENERGY_MGMT) &&
        (attrName.contains("Current") ||
          attrName.contains("Power") ||
          attrName.contains("Capacity")) -> INT64
      attrName.contains("Mireds") ||
        attrName.contains("Percent100ths") ||
        attrName.contains("Time") -> UINT16
      attrName.contains("State") || attrName.contains("Status") || attrName.contains("Type") ->
        ENUM8
      else -> UINT8
    }
  }

  private fun inferFieldType(fieldName: String, paramInfo: CommandParameterInfo?): MatterDataType {
    val underlying = paramInfo?.underlyingType ?: paramInfo?.type
    val lower = fieldName.lowercase(Locale.US)
    return when {
      underlying == Boolean::class.javaObjectType -> BOOLEAN
      underlying == String::class.java -> UTF8_STRING
      underlying == ByteArray::class.java -> OCTET_STRING
      underlying == Float::class.javaObjectType -> FLOAT32
      underlying == Double::class.javaObjectType -> FLOAT64
      lower == "amount" -> INT8
      lower.contains("current") || lower.contains("power") -> INT64
      lower.contains("time") ||
        lower.contains("mireds") ||
        lower.contains("100ths") ||
        lower.contains("colorx") ||
        lower.contains("colory") ||
        lower.contains("tiltvalue") ||
        lower.contains("liftvalue") -> UINT16
      lower.contains("mask") || lower.contains("override") || lower.contains("control") -> BITMAP8
      lower.contains("mode") ||
        lower.contains("direction") ||
        lower.contains("effectidentifier") ||
        lower.contains("keycode") -> ENUM8
      underlying == Long::class.javaObjectType -> UINT32
      else -> UINT8
    }
  }

  private fun categorizeById(clusterId: Long): ClusterCategory =
    when (clusterId) {
      CLUSTER_ON_OFF,
      CLUSTER_LEVEL_CONTROL,
      CLUSTER_COLOR_CONTROL -> ClusterCategory.LIGHTING_AND_POWER
      CLUSTER_DOOR_LOCK,
      CLUSTER_WINDOW_COVERING,
      CLUSTER_BARRIER_CONTROL -> ClusterCategory.CLOSURES
      CLUSTER_THERMOSTAT,
      CLUSTER_FAN_CONTROL -> ClusterCategory.HVAC
      CLUSTER_MEDIA_PLAYBACK,
      CLUSTER_KEYPAD_INPUT,
      CLUSTER_AUDIO_OUTPUT -> ClusterCategory.MEDIA
      CLUSTER_MODE_SELECT,
      CLUSTER_RVC_RUN_MODE,
      CLUSTER_RVC_CLEAN_MODE,
      CLUSTER_OPERATIONAL_STATE -> ClusterCategory.APPLIANCES_AND_ROBOTICS
      CLUSTER_DEVICE_ENERGY_MGMT,
      CLUSTER_ENERGY_EVSE -> ClusterCategory.ENERGY_MANAGEMENT
      CLUSTER_BOOLEAN_STATE,
      CLUSTER_SMOKE_CO_ALARM,
      CLUSTER_SWITCH,
      CLUSTER_TEMPERATURE_MEASUREMENT,
      CLUSTER_RELATIVE_HUMIDITY_MEASUREMENT,
      CLUSTER_OCCUPANCY_SENSING,
      CLUSTER_ILLUMINANCE_MEASUREMENT -> ClusterCategory.SENSORS_AND_ALARMS
      else -> ClusterCategory.GENERAL_AND_SYSTEM
    }

  private fun applySemanticOverlays() {
    val alarm = "0:OK,1:Warning,2:Critical"
    val rvcRun = "0:Idle,1:Cleaning,2:Mapping,3:ReturningToDock"
    val rvcClean = "0:Vacuum,1:Mop,2:VacuumAndMop,3:DeepClean"
    val pinOpt = field(0, isOpt = true, isNull = true, def = null)

    overlay(
      CLUSTER_LEVEL_CONTROL,
      cmd(
        0x05L,
        "StepWithOnOff",
        field(0, name = "stepMode", enums = "0:Up,1:Down"),
        field(1, name = "stepSize"),
        field(2, name = "transitionTime", isOpt = true, def = 0),
        field(3, name = "optionsMask", isOpt = true, def = 0),
        field(4, name = "optionsOverride", isOpt = true, def = 0),
      ),
    )
    overlay(
      CLUSTER_DOOR_LOCK,
      attr(0x0000L, enums = "0:NotFullyLocked,1:Locked,2:Unlocked"),
      timedCmd(0x00L, pinOpt),
      timedCmd(0x01L, pinOpt),
      timedCmd(
        0x03L,
        field(0, name = "timeoutSeconds", isOpt = false),
        field(1, name = "pinCode", isOpt = true, isNull = true, def = null),
      ),
    )
    registerCluster(
      MatterClusterMeta(
        CLUSTER_BARRIER_CONTROL,
        "BarrierControl",
        ClusterCategory.CLOSURES,
        "Controls motorized barrier gates and garage doors.",
        mapOf(
          0x00L to
            MatterCommandMeta(
              0x00L,
              "BarrierControlGoToPercent",
              "BarrierControlGoToPercent",
              requestFields = listOf(MatterFieldMeta(0, "percentOpen", MatterDataType.UINT8)),
            ),
          0x01L to MatterCommandMeta(0x01L, "BarrierControlStop", "BarrierControlStop"),
        ),
        mapOf(
          0x0001L to MatterAttributeMeta(0x0001L, "BarrierMovingState", MatterDataType.ENUM8),
          0x000AL to MatterAttributeMeta(0x000AL, "BarrierPosition", MatterDataType.UINT8),
        ),
      )
    )
    overlay(
      CLUSTER_THERMOSTAT,
      writableAttr(
        0x001CL,
        enums = "0:Off,1:Auto,3:Cool,4:Heat,5:EmergencyHeat,6:Precooling,7:FanOnly",
      ),
    )
    overlay(
      CLUSTER_FAN_CONTROL,
      writableAttr(0x0000L, enums = "0:Off,1:Low,2:Medium,3:High,4:On,5:Auto,6:Smart"),
    )
    overlay(
      CLUSTER_MEDIA_PLAYBACK,
      attr(0x0000L, enums = "0:Playing,1:Paused,2:NotPlaying,3:Buffering"),
    )
    overlay(
      CLUSTER_KEYPAD_INPUT,
      cmd(
        0x00L,
        field(
          0,
          enums =
            "0:Select,1:Up,2:Down,3:Left,4:Right,9:RootMenu,10:SetupMenu,11:ContentsMenu," +
              "12:FavoriteMenu,13:Exit",
        ),
      ),
    )
    overlay(
      CLUSTER_AUDIO_OUTPUT,
      attr(0x0000L),
      attr(0x0001L),
      cmd(0x00L, field(0, def = 0)),
      cmd(0x01L, field(0, def = 0), field(1, def = "")),
    )
    overlay(
      CLUSTER_MODE_SELECT,
      cmd(0x00L, field(0, def = 1, enums = "0:Normal,1:Eco,2:Quick,3:Delicate,4:Heavy")),
    )
    overlay(
      CLUSTER_RVC_RUN_MODE,
      attr(0x0001L, enums = rvcRun),
      cmd(0x00L, field(0, def = 1, enums = rvcRun)),
    )
    overlay(
      CLUSTER_RVC_CLEAN_MODE,
      attr(0x0001L, enums = rvcClean),
      cmd(0x00L, field(0, def = 1, enums = rvcClean)),
    )
    overlay(
      CLUSTER_OPERATIONAL_STATE,
      attr(0x0004L, enums = "0:Stopped,1:Running,2:Paused,3:Error"),
    )
    overlay(
      CLUSTER_ENERGY_EVSE,
      attr(
        0x0000L,
        enums =
          "0:NotPluggedIn,1:PluggedInNoDemand,2:PluggedInDemand,3:PluggedInCharging," +
            "4:PluggedInDischarging,5:SessionEnding,6:Fault",
      ),
      timedCmd(0x01L),
      timedCmd(
        0x02L,
        field(0, isOpt = false, isNull = true, def = null),
        field(1, def = 6000L),
        field(2, def = 32000L),
      ),
      timedCmd(0x03L, field(0, isOpt = false, isNull = true, def = null), field(1, def = 32000L)),
      timedCmd(0x04L),
      timedCmd(0x06L),
      timedCmd(0x07L),
    )
    overlay(
      CLUSTER_SMOKE_CO_ALARM,
      attr(
        0x0000L,
        enums =
          "0:Normal,1:SmokeAlarm,2:COAlarm,3:BatteryAlert,4:Testing,5:HardwareFault," +
            "6:EndOfService,7:InterconnectSmoke,8:InterconnectCO",
      ),
      attr(0x0001L, enums = alarm),
      attr(0x0002L, enums = alarm),
      attr(0x0003L, enums = alarm),
      attr(0x0004L, enums = "0:NotMuted,1:Muted"),
    )
    overlay(
      CLUSTER_POWER_SOURCE,
      attr(0x0000L, enums = "0:Unspecified,1:Active,2:Standby,3:Unavailable"),
      attr(0x000CL, unit = "%"),
      attr(0x000EL, enums = alarm),
    )
  }

  private fun parseEnums(spec: String): Map<String, Long> =
    if (spec.isEmpty()) emptyMap()
    else
      spec.split(",").associate {
        val (id, label) = it.split(":")
        label to id.toLong()
      }

  private fun attr(id: Long, unit: String? = null, enums: String = "") =
    MatterAttributeMeta(
      id,
      "",
      MatterDataType.UINT8,
      isWritable = false,
      enumValues = parseEnums(enums),
      unit = unit,
    )

  private fun writableAttr(id: Long, unit: String? = null, enums: String = "") =
    MatterAttributeMeta(
      id,
      "",
      MatterDataType.UINT8,
      isWritable = true,
      enumValues = parseEnums(enums),
      unit = unit,
    )

  private fun field(
    tag: Int,
    name: String = "",
    isOpt: Boolean? = null,
    isNull: Boolean? = null,
    def: Any? = null,
    enums: String = "",
  ) =
    MatterFieldMeta(
      tag,
      name,
      if (name.isNotEmpty()) inferFieldType(name, null) else MatterDataType.UINT8,
      isOptional = isOpt ?: false,
      isNullable = isNull ?: false,
      description = "${isOpt ?: ""}:${isNull ?: ""}",
      defaultValue = def,
      enumValues = parseEnums(enums),
    )

  private fun cmd(id: Long, vararg fields: MatterFieldMeta) =
    MatterCommandMeta(id, "", "", requestFields = fields.toList(), isTimed = false)

  private fun cmd(id: Long, name: String, vararg fields: MatterFieldMeta) =
    MatterCommandMeta(id, name, "Invoke $name", requestFields = fields.toList(), isTimed = false)

  private fun timedCmd(id: Long, vararg fields: MatterFieldMeta) =
    MatterCommandMeta(id, "", "", requestFields = fields.toList(), isTimed = true)

  private fun overlay(clusterId: Long, vararg items: Any) {
    val existing = registry[clusterId] ?: return
    val mergedAttrs = LinkedHashMap(existing.attributes)
    val mergedCmds = LinkedHashMap(existing.commands)
    for (item in items) {
      when (item) {
        is MatterAttributeMeta -> {
          mergedAttrs[item.attributeId] = mergeAttributeOverlay(mergedAttrs[item.attributeId], item)
        }
        is MatterCommandMeta -> {
          mergedCmds[item.commandId] = mergeCommandOverlay(mergedCmds[item.commandId], item)
        }
      }
    }
    registry[clusterId] = existing.copy(attributes = mergedAttrs, commands = mergedCmds)
  }

  private fun mergeAttributeOverlay(
    old: MatterAttributeMeta?,
    item: MatterAttributeMeta,
  ): MatterAttributeMeta {
    if (old == null) return item
    return old.copy(
      name = item.name.ifEmpty { old.name },
      type = if (item.name.isNotEmpty()) item.type else old.type,
      isWritable = item.isWritable || old.isWritable,
      isNullable = item.isNullable || old.isNullable,
      unit = item.unit ?: old.unit,
      enumValues = if (item.enumValues.isNotEmpty()) item.enumValues else old.enumValues,
    )
  }

  private fun mergeCommandOverlay(
    old: MatterCommandMeta?,
    item: MatterCommandMeta,
  ): MatterCommandMeta {
    if (old == null) return item
    val newFields = old.requestFields.map { oldField -> mergeFieldOverlay(oldField, item) }
    return old.copy(
      name = item.name.ifEmpty { old.name },
      isTimed = item.isTimed || old.isTimed,
      requestFields = newFields,
    )
  }

  private fun mergeFieldOverlay(
    oldField: MatterFieldMeta,
    item: MatterCommandMeta,
  ): MatterFieldMeta {
    val overrideField = item.requestFields.find { it.tagId == oldField.tagId } ?: return oldField
    val parts = overrideField.description.split(":", limit = 2)
    val optOverride = parts.getOrNull(0)?.toBooleanStrictOrNull()
    val nullOverride = parts.getOrNull(1)?.toBooleanStrictOrNull()
    val mergedEnums =
      if (overrideField.enumValues.isNotEmpty()) overrideField.enumValues else oldField.enumValues
    return oldField.copy(
      name = overrideField.name.ifEmpty { oldField.name },
      isOptional = optOverride ?: oldField.isOptional,
      isNullable = nullOverride ?: oldField.isNullable,
      enumValues = mergedEnums,
      defaultValue = overrideField.defaultValue ?: oldField.defaultValue,
    )
  }
}
