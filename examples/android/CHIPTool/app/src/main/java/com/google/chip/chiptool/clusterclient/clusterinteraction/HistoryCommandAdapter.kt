package com.google.chip.chiptool.clusterclient.clusterinteraction

import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.EditText
import android.widget.LinearLayout
import android.widget.TextView
import androidx.constraintlayout.widget.ConstraintLayout
import androidx.recyclerview.widget.RecyclerView
import chip.clusterinfo.CommandResponseInfo
import com.google.chip.chiptool.R

/**
 * HistoryCommandAdapter implements the historyCommandList(RecycleView) Adapter and associates
 * different history command with the same onClick function provided in
 * [ClusterInteractionHistoryFragment.HistoryCommandListener]
 */
class HistoryCommandAdapter(
  private val HistoryCommandList: List<HistoryCommand>,
  private val listener: ClusterInteractionHistoryFragment.HistoryCommandListener,
  private val inflater: LayoutInflater,
) : RecyclerView.Adapter<HistoryCommandAdapter.HistoryCommandViewHolder>() {

  inner class HistoryCommandViewHolder(itemView: View) :
    RecyclerView.ViewHolder(itemView), View.OnClickListener {
    var historyInfo: LinearLayout = itemView.findViewById(R.id.historyBasicInfo)
    var parameterList: LinearLayout = itemView.findViewById(R.id.historyParameterList)
    var responseValueList: LinearLayout = itemView.findViewById(R.id.historyResponseValueList)
    var statusCode: LinearLayout = itemView.findViewById(R.id.historyItemStatus)

    init {
      itemView.setOnClickListener(this)
    }

    override fun onClick(endpointItem: View) {
      val position = this.adapterPosition
      if (position != RecyclerView.NO_POSITION) {
        listener.onItemClick(position)
      }
    }
  }

  interface OnItemClickListener {
    fun onItemClick(position: Int)
  }

  override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): HistoryCommandViewHolder {
    val itemView =
      LayoutInflater.from(parent.context)
        .inflate(R.layout.cluster_interaction_history_item, parent, false)
    return HistoryCommandViewHolder(itemView)
  }

  override fun getItemCount(): Int {
    return HistoryCommandList.size
  }

  override fun getItemId(position: Int): Long {
    return position.toLong()
  }

  override fun getItemViewType(position: Int): Int {
    return position
  }

  override fun onBindViewHolder(
    holder: HistoryCommandAdapter.HistoryCommandViewHolder,
    position: Int
  ) {
    // go through each element and fill the data
    // fill out cluster name and command name
    clearPreviousReview(holder)
    val info =
      inflater.inflate(R.layout.cluster_interaction_history_item_info, null, false)
        as ConstraintLayout
    val historyClusterNameTv: TextView = info.findViewById(R.id.historyClusterNameTv)
    val historyCommandNameTv: TextView = info.findViewById(R.id.historyCommandNameTv)

    historyClusterNameTv.text = HistoryCommandList[position].clusterName
    historyCommandNameTv.text = HistoryCommandList[position].commandName
    holder.historyInfo.addView(info)
    // fill out parameterList
    if (HistoryCommandList[position].parameterList.isEmpty()) {
      val emptyParameterList =
        inflater.inflate(android.R.layout.simple_list_item_1, null, false) as TextView
      emptyParameterList.text = "No parameter"
      holder.parameterList.addView(emptyParameterList)
    } else {
      HistoryCommandList[position].parameterList.forEach {
        val param =
          inflater.inflate(R.layout.cluster_parameter_item, null, false) as ConstraintLayout
        val clusterParameterNameTv: TextView = info.findViewById(R.id.clusterParameterNameTv)
        val clusterParameterTypeTv: TextView = info.findViewById(R.id.clusterParameterTypeTv)
        val clusterParameterData: EditText = info.findViewById(R.id.clusterParameterData)

        clusterParameterData.setText(it.parameterData)
        clusterParameterNameTv.text = it.parameterName
        clusterParameterTypeTv.text = formatParameterType(it.parameterType)
        holder.parameterList.addView(param)
      }
    }
    // fill out responseList
    if (
      HistoryCommandList[position].responseValue == null ||
        HistoryCommandList[position].responseValue!!.isEmpty()
    ) {
      val emptyResponseInfo =
        inflater.inflate(android.R.layout.simple_list_item_1, null, false) as TextView
      emptyResponseInfo.text = "No response"
      holder.responseValueList.addView(emptyResponseInfo)
    } else {
      populateCallbackResult(
        HistoryCommandList[position].responseValue!!,
        inflater,
        holder.responseValueList
      )
    }
    // fill out status
    val statusInfo = inflater.inflate(android.R.layout.simple_list_item_1, null, false) as TextView
    statusInfo.text = "Status: " + HistoryCommandList[position].status
    holder.statusCode.addView(statusInfo)
  }

  private fun populateCallbackResult(
    responseValues: Map<CommandResponseInfo, Any>,
    inflater: LayoutInflater,
    callbackList: LinearLayout
  ) {
    responseValues.forEach { (variableNameType, response) ->
      if (response is List<*>) {
        createListResponseView(response, inflater, callbackList, variableNameType)
      } else {
        createBasicResponseView(response, inflater, callbackList, variableNameType)
      }
    }
  }

  private fun createBasicResponseView(
    response: Any,
    inflater: LayoutInflater,
    callbackList: LinearLayout,
    variableNameType: CommandResponseInfo
  ) {
    val callbackItem =
      inflater.inflate(R.layout.cluster_callback_item, null, false) as ConstraintLayout
    val clusterCallbackNameTv: TextView = callbackItem.findViewById(R.id.clusterCallbackNameTv)
    val clusterCallbackDataTv: TextView = callbackItem.findViewById(R.id.clusterCallbackDataTv)
    val clusterCallbackTypeTv: TextView = callbackItem.findViewById(R.id.clusterCallbackTypeTv)

    clusterCallbackNameTv.text = variableNameType.name
    clusterCallbackDataTv.text =
      if (response.javaClass == ByteArray::class.java) {
        (response as ByteArray).decodeToString()
      } else {
        response.toString()
      }
    clusterCallbackTypeTv.text = variableNameType.type
    callbackList.addView(callbackItem)
  }

  private fun createListResponseView(
    response: List<*>,
    inflater: LayoutInflater,
    callbackList: LinearLayout,
    variableNameType: CommandResponseInfo
  ) {
    if (response.isEmpty()) {
      val emptyCallback =
        inflater.inflate(R.layout.cluster_callback_item, null, false) as ConstraintLayout
      val clusterCallbackNameTv: TextView = emptyCallback.findViewById(R.id.clusterCallbackNameTv)

      clusterCallbackNameTv.text = "Result is empty"
      callbackList.addView(emptyCallback)
    } else {
      response.forEachIndexed { index, it ->
        val attributeCallbackItem =
          inflater.inflate(R.layout.cluster_callback_item, null, false) as ConstraintLayout
        val clusterCallbackNameTv: TextView =
          attributeCallbackItem.findViewById(R.id.clusterCallbackNameTv)
        val clusterCallbackDataTv: TextView =
          attributeCallbackItem.findViewById(R.id.clusterCallbackDataTv)
        val clusterCallbackTypeTv: TextView =
          attributeCallbackItem.findViewById(R.id.clusterCallbackTypeTv)

        clusterCallbackNameTv.text = variableNameType.name + "[$index]"
        val objectString =
          if (it!!.javaClass == ByteArray::class.java) {
            (it as ByteArray).contentToString()
          } else {
            it.toString()
          }
        var callbackClassName =
          if (it!!.javaClass == ByteArray::class.java) {
            "Byte[]"
          } else {
            it!!.javaClass.toString().split('$').last()
          }
        clusterCallbackDataTv.text = objectString
        clusterCallbackTypeTv.text = "List<$callbackClassName>"
        callbackList.addView(attributeCallbackItem)
      }
    }
  }

  private fun formatParameterType(castType: Class<*>): String {
    return if (castType == ByteArray::class.java) {
      "Byte[]"
    } else {
      castType.toString()
    }
  }

  private fun clearPreviousReview(holder: HistoryCommandAdapter.HistoryCommandViewHolder) {
    holder.historyInfo.removeAllViews()
    holder.parameterList.removeAllViews()
    holder.responseValueList.removeAllViews()
    holder.statusCode.removeAllViews()
  }
}
