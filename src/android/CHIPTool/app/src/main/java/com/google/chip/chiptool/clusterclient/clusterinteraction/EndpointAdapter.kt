package com.google.chip.chiptool.clusterclient.clusterinteraction

import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.AdapterView
import android.widget.TextView
import androidx.recyclerview.widget.RecyclerView
import com.google.chip.chiptool.R

class EndpointAdapter(private val endpointList: List<EndpointItem>,
                      private val listener: OnItemClickListener

) : RecyclerView.Adapter<EndpointAdapter.EndpointViewHolder>() {

  inner class EndpointViewHolder(itemView: View) : RecyclerView.ViewHolder(itemView), View.OnClickListener {
    val endpointId: TextView = itemView.findViewById(R.id.endpointTv)
    init {
      itemView.setOnClickListener(this)
    }

    override fun onClick(p0: View?) {
      val position = adapterPosition
      if (position != RecyclerView.NO_POSITION) {
        listener.onItemClick(position)
      }
    }

  }

  interface OnItemClickListener {
    fun onItemClick(position: Int)
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