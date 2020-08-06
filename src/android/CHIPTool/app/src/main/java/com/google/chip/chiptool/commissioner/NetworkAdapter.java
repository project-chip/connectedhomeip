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

package com.google.chip.chiptool.commissioner;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;
import com.google.chip.chiptool.R;
import java.util.Arrays;
import java.util.Vector;

public class NetworkAdapter extends BaseAdapter {
  private Vector<NetworkInfo> networks;

  private LayoutInflater inflater;

  NetworkAdapter(Context context) {
    inflater = (LayoutInflater)context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
    networks = new Vector<>();
  }

  public boolean addNetwork(NetworkInfo newNetwork) {
    for (NetworkInfo network : networks) {
      if (network.getNetworkName().equals(newNetwork.getNetworkName()) &&
              Arrays.equals(network.getExtendedPanId(), newNetwork.getExtendedPanId())) {
        network.merge(newNetwork);
        return false;
      }
    }

    networks.add(newNetwork);
    notifyDataSetChanged();
    return true;
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
    networkNameText.setText(networks.get(position).getNetworkName());
    return convertView;
  }
}
