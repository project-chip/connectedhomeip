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

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.ListView;
import android.widget.TextView;
import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;
import androidx.navigation.NavController;
import androidx.navigation.fragment.NavHostFragment;
import com.google.chip.chiptool.R;
import com.google.chip.chiptool.setuppayloadscanner.CHIPDeviceInfo;

public class SelectNetworkFragment extends Fragment {

  private CHIPDeviceInfo deviceInfo;

  private NetworkAdapter networksAdapter;

  private NetworkInfo selectedNetwork;
  private Button addDeviceButton;


  private BorderAgentDiscoverer borderAgentDiscoverer;


  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);

    networksAdapter = new NetworkAdapter(getContext());
    borderAgentDiscoverer = new BorderAgentDiscoverer(getContext(), networksAdapter);
    borderAgentDiscoverer.start();
  }

  @Override
  public void onDestroy() {
    super.onDestroy();

    borderAgentDiscoverer.stop();
  }

  @Override
  public View onCreateView(
      LayoutInflater inflater, ViewGroup container,
      Bundle savedInstanceState
  ) {
    // Inflate the layout for this fragment
    return inflater.inflate(R.layout.commissioner_select_network_fragment, container, false);
  }

  public void onViewCreated(@NonNull View view, Bundle savedInstanceState) {
    super.onViewCreated(view, savedInstanceState);

    // Hide the button
    addDeviceButton = view.findViewById(R.id.add_device_button);
    addDeviceButton.setVisibility(View.GONE);

    deviceInfo = getArguments().getParcelable(Constants.KEY_DEVICE_INFO);

    String deviceInfoString = String.format("version: %d\nvendorId: %d\nproductId: %d\nsetupPinCode: %d",
            deviceInfo.getVersion(), deviceInfo.getVendorId(), deviceInfo.getProductId(), deviceInfo.getSetupPinCode());
    TextView deviceInfoView = view.findViewById(R.id.device_info);
    deviceInfoView.setText(deviceInfoString);

    final ListView networkListView = view.findViewById(R.id.networks);
    networkListView.setAdapter(networksAdapter);

    networkListView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
      @Override
      public void onItemClick(AdapterView<?> adapterView, View view, int position, long id) {
        selectedNetwork = (NetworkInfo)adapterView.getItemAtPosition(position);
        addDeviceButton.setVisibility(View.VISIBLE);
      }
    });

    view.findViewById(R.id.add_device_button).setOnClickListener(new View.OnClickListener() {
      @Override
      public void onClick(View view) {
        NavController controller = NavHostFragment.findNavController(SelectNetworkFragment.this);

        if (controller.getCurrentDestination().getId() == R.id.commissioner_select_network_fragment) {
          Bundle bundle = new Bundle();

          assert(deviceInfo != null);
          assert(selectedNetwork != null);
          bundle.putParcelable(Constants.KEY_DEVICE_INFO, deviceInfo);
          bundle.putParcelable(Constants.KEY_NETWORK_INFO, selectedNetwork);

          controller.navigate(R.id.action_select_network_to_commissioning, bundle);
        }
      }
    });
  }

  // TODO: may add network if it is new.
  private void onFoundBorderAgent() {

  }
}
