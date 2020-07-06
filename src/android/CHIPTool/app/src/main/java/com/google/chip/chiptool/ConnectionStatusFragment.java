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

package com.google.chip.chiptool;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import androidx.appcompat.app.AppCompatActivity;
import androidx.fragment.app.Fragment;

// A fragment included at the top of activities to show the connection status
// with the CHIP device.
public class ConnectionStatusFragment extends Fragment {

  public ConnectionStatusFragment() {}

  @Override
  public View onCreateView(
      LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
    View inflated = inflater.inflate(R.layout.connection_status_fragment, container, false);

    TextView statusView = (TextView) inflated.findViewById(R.id.connection_status);
    // statusView.setText(CHIPNativeBridge.getInstance().getConnectionInfo());

    return inflated;
  }

  @Override
  public void onViewCreated(View view, Bundle savedInstanceState) {
    super.onViewCreated(view, savedInstanceState);
  }

  private void updateStatusInternal() {
    TextView statusView = (TextView) getView().findViewById(R.id.connection_status);
    // statusView.setText(CHIPNativeBridge.getInstance().getConnectionInfo());
  }

  // Should be called by an activity after status connection has changed or when the
  // activity's view is created.
  public static void updateStatus(AppCompatActivity activity) {
    ConnectionStatusFragment statusFragment =
        (ConnectionStatusFragment)
            activity.getSupportFragmentManager().findFragmentById(R.id.connection_status_fragment);
    statusFragment.updateStatusInternal();
  }
}
