package com.google.chip.chiptool.clusterclient.clusterinteraction

import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import androidx.recyclerview.widget.RecyclerView
import com.google.chip.chiptool.R

class EndpointAdapter(private val endpointList: List<EndpointItem>) : RecyclerView.Adapter<EndpointAdapter.EndpointViewHolder>() {

  class EndpointViewHolder(itemView: View) : RecyclerView.ViewHolder(itemView) {
    val endpointId: TextView = itemView.findViewById(R.id.endpoint_id)

  }

  override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): EndpointViewHolder {
    val itemView = LayoutInflater.from(parent.context).inflate(R.layout.endpoint_item, parent, false)
    return EndpointViewHolder(itemView)
  }

  override fun onBindViewHolder(holder: EndpointViewHolder, position: Int) {
    val currentItem = endpointList[position]

    holder.endpointId.text = currentItem.endpointId.toString()
  }

  override fun getItemCount(): Int {
    return endpointList.size
  }
}