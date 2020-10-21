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

import android.app.Activity;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.TextView;
import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;
import com.google.chip.chiptool.R;
import com.google.chip.chiptool.commissioner.CommissionerActivity;
import com.google.chip.chiptool.commissioner.thread.ThreadNetworkCredential;

public class CommissioningFragment extends Fragment {

  private static final String TAG = CommissioningFragment.class.getSimpleName();

  ThreadNetworkCredential networkCredential;

  CommissionerActivity commissionerActivity;

  TextView statusText;
  ProgressBar progressBar;
  Button cancelButton;
  Button doneButton;
  ImageView doneImage;
  ImageView errorImage;

  public CommissioningFragment(@NonNull ThreadNetworkCredential networkCredential) {
    this.networkCredential = networkCredential;
  }

  @Override
  public View onCreateView(
      LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
    // Inflate the layout for this fragment
    return inflater.inflate(R.layout.commissioner_commissioning_fragment, container, false);
  }

  @Override
  public void onViewCreated(@NonNull View view, Bundle savedInstanceState) {
    super.onViewCreated(view, savedInstanceState);

    commissionerActivity = (CommissionerActivity) getActivity();

    cancelButton = view.findViewById(R.id.cancel_button);
    doneButton = view.findViewById(R.id.done_button);
    doneImage = view.findViewById(R.id.done_image);
    errorImage = view.findViewById(R.id.error_image);
    statusText = view.findViewById(R.id.status_text);
    progressBar = view.findViewById(R.id.commissioning_progress);

    view.findViewById(R.id.cancel_button)
        .setOnClickListener(
            v -> {
              commissionerActivity.finishCommissioning(Activity.RESULT_CANCELED);
            });

    view.findViewById(R.id.done_button)
        .setOnClickListener(
            v -> {
              commissionerActivity.finishCommissioning(Activity.RESULT_OK);
            });

    // TODO: commissioning over BLE.
    showCommissionDone(false, "Commissioning over BLE not implemented yet!");
  }

  private void showInProgress(String status) {
    if (status != null) {
      statusText.setText(status);
    }

    progressBar.setVisibility(View.VISIBLE);

    cancelButton.setVisibility(View.VISIBLE);
    doneImage.setVisibility(View.GONE);
    errorImage.setVisibility(View.GONE);
    doneButton.setVisibility(View.GONE);
  }

  private void showCommissionDone(Boolean success, String status) {
    if (status != null) {
      statusText.setText(status);
    }

    progressBar.setVisibility(View.GONE);
    cancelButton.setVisibility(View.GONE);
    doneButton.setVisibility(View.VISIBLE);

    if (success) {
      doneImage.setVisibility(View.VISIBLE);
      errorImage.setVisibility(View.GONE);
    } else {
      doneImage.setVisibility(View.GONE);
      errorImage.setVisibility(View.VISIBLE);
    }
  }
}
