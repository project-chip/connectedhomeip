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

package com.google.chip.chiptool.setuppayloadscanner;

import android.os.Bundle;
import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AppCompatActivity;
import androidx.fragment.app.Fragment;
import com.google.chip.chiptool.R;
import com.google.chip.chiptool.setuppayloadscanner.BarcodeFragment.Callback;

/** Launches the QR code scanner and shows the details of any detected CHIP device QR codes. */
public class BarcodeReaderActivity extends AppCompatActivity implements Callback {

  private static final String TAG = BarcodeReaderActivity.class.getSimpleName();

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.barcode_activity);

    ActionBar actionBar = getSupportActionBar();
    if (actionBar != null) {
      actionBar.setTitle(R.string.barcode_scanner_activity_toolbar_title);
    }

    if (savedInstanceState == null) {
      addFragment(BarcodeFragment.newInstance());
    }
  }

  @Override
  public void onCHIPDeviceInfoReceived(CHIPDeviceInfo deviceInfo) {
    showFragment(CHIPDeviceDetailsFragment.newInstance(deviceInfo));
  }

  private void addFragment(Fragment fragment) {
    getSupportFragmentManager()
        .beginTransaction()
        .add(R.id.fragment_container, fragment, fragment.getClass().getSimpleName())
        .commit();
  }

  private void showFragment(Fragment fragment) {
    getSupportFragmentManager()
        .beginTransaction()
        .replace(R.id.fragment_container, fragment, fragment.getClass().getSimpleName())
        .addToBackStack(null)
        .commit();
  }
}
