/*
 *   Copyright (c) 2020 Project CHIP Authors
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

package com.google.chip.chiptool.commissioner.thread.internal;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;
import com.google.chip.chiptool.R;
import com.google.chip.chiptool.commissioner.thread.BorderAgentInfo;
import com.google.chip.chiptool.commissioner.thread.ThreadNetworkInfo;
import com.google.chip.chiptool.commissioner.thread.internal.BorderAgentDiscoverer.BorderAgentListener;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Vector;

class NetworkAdapter extends BaseAdapter implements BorderAgentListener {
  private Vector<ThreadNetworkInfoHolder> networks;

  private LayoutInflater inflater;

  NetworkAdapter(Context context) {
    inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
    networks = new Vector<>();
  }

  public void addBorderAgent(BorderAgentInfo borderAgent) {
    boolean hasExistingNetwork = false;
    for (ThreadNetworkInfoHolder networkInfoHolder : networks) {
      if (networkInfoHolder.networkInfo.networkName.equals(borderAgent.networkName)
          && Arrays.equals(networkInfoHolder.networkInfo.extendedPanId, borderAgent.extendedPanId)) {
        networkInfoHolder.borderAgents.add(borderAgent);
        hasExistingNetwork = true;
      }
    }

    if (!hasExistingNetwork) {
      networks.add(new ThreadNetworkInfoHolder(borderAgent));
    }

    new Handler(Looper.getMainLooper()).post(() -> notifyDataSetChanged());
  }

  public void removeBorderAgent(String lostBorderAgentDisciminator) {
    for (ThreadNetworkInfoHolder networkInfoHolder : networks) {
      for (BorderAgentInfo borderAgent : networkInfoHolder.borderAgents) {
        if (borderAgent.discriminator.equals(lostBorderAgentDisciminator)) {
          networkInfoHolder.borderAgents.remove(borderAgent);
          if (networkInfoHolder.borderAgents.isEmpty()) {
            networks.remove(networkInfoHolder);
          }

          new Handler(Looper.getMainLooper()).post(() -> notifyDataSetChanged());
          return;
        }
      }
    }
  }

  @Override
  public int getCount() {
    return networks.size();
  }

  @Override
  public Object getItem(int position) {
    return networks.get(position);
  }

  @Override
  public long getItemId(int position) {
    return position;
  }

  @Override
  public View getView(int position, View convertView, ViewGroup container) {
    if (convertView == null) {
      convertView = inflater.inflate(R.layout.commissioner_network_list_item, container, false);
    }
    TextView networkNameText = convertView.findViewById(R.id.network_name);
    networkNameText.setText(networks.get(position).networkInfo.networkName);
    return convertView;
  }

  @Override
  public void onBorderAgentFound(BorderAgentInfo borderAgentInfo) {
    addBorderAgent(borderAgentInfo);
  }

  @Override
  public void onBorderAgentLost(String discriminator) {
    removeBorderAgent(discriminator);
  }
}
