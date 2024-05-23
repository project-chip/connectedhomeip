package com.google.chip.chiptool.clusterclient.clusterinteraction

import chip.clusterinfo.CommandResponseInfo
import java.io.Serializable

/**
 * HistoryCommand stores information about each executed command and display necessary information
 * for each item in [ClusterInteractionHistoryFragment]
 */
data class HistoryCommand(
  val clusterName: String,
  val commandName: String,
  val parameterList: MutableList<HistoryParameterInfo>,
  var responseValue: Map<CommandResponseInfo, Any>?,
  var status: String?,
  var endpointId: Int,
  var deviceId: Long,
) : Serializable

data class HistoryParameterInfo(
  val parameterName: String,
  val parameterData: String,
  val parameterType: Class<*>
)
