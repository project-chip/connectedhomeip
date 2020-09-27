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
import androidx.fragment.app.Fragment;
import com.google.chip.chiptool.R;
import com.google.chip.chiptool.commissioner.thread.internal.SelectNetworkFragment;
import com.google.chip.chiptool.setuppayloadscanner.BarcodeFragment;
import com.google.chip.chiptool.setuppayloadscanner.CHIPDeviceInfo;

public class CommissionerActivity extends AppCompatActivity implements BarcodeFragment.Callback {

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.commissioner_activity);

    if (savedInstanceState == null) {
      showFragment(new BarcodeFragment());
    }
  }

  @Override
  public void onActivityResult(int requestCode, int resultCode, Intent data) {
    super.onActivityResult(requestCode, resultCode, data);
  }

  @Override
  public void onCHIPDeviceInfoReceived(@NonNull CHIPDeviceInfo deviceInfo) {
    showFragment(new SelectNetworkFragment(deviceInfo));
  }

  public void finishCommissioning(int resultCode) {
    Intent resultIntent = new Intent();
    setResult(resultCode, resultIntent);
    finish();
  }

  public void showFragment(Fragment fragment) {
    getSupportFragmentManager()
        .beginTransaction()
        .replace(R.id.commissioner_service_activity, fragment, fragment.getClass().getSimpleName())
        .addToBackStack(null)
        .commit();
  }
}
