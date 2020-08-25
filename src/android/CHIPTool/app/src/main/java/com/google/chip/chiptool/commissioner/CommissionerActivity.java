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

import android.content.Intent;
import android.os.Bundle;
import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.navigation.NavController;
import androidx.navigation.Navigation;
import com.google.chip.chiptool.R;
import com.google.chip.chiptool.setuppayloadscanner.BarcodeFragment;
import com.google.chip.chiptool.setuppayloadscanner.CHIPDeviceInfo;

public class CommissionerActivity extends AppCompatActivity implements BarcodeFragment.Callback {

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.commissioner_activity);
  }

  public void onCHIPDeviceInfoReceived(@NonNull CHIPDeviceInfo deviceInfo) {
    NavController controller = Navigation.findNavController(this, R.id.nav_host_fragment);

    if (controller.getCurrentDestination().getId() == R.id.commissioner_scan_qr_code_fragment) {
      Bundle bundle = new Bundle();
      bundle.putParcelable(Constants.KEY_DEVICE_INFO, deviceInfo);
      controller.navigate(R.id.action_scan_qr_code_to_select_network, bundle);
    }
  }

  public void finishCommissioning(int resultCode) {
    Intent resultIntent = new Intent();
    setResult(resultCode, resultIntent);
    finish();
  }
}
