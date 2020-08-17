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
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import androidx.lifecycle.Observer;
import androidx.work.Data;
import androidx.work.OneTimeWorkRequest;
import androidx.work.WorkInfo;
import androidx.work.WorkManager;
import androidx.work.WorkRequest;
import com.google.chip.chiptool.R;
import com.google.chip.chiptool.setuppayloadscanner.CHIPDeviceInfo;
import com.google.gson.Gson;

public class CommissioningFragment extends Fragment implements Observer<WorkInfo> {

  private static final String TAG = CommissioningFragment.class.getSimpleName();

  private CHIPDeviceInfo deviceInfo;
  private NetworkInfo networkInfo;

  WorkRequest commssionerWorkRequest;

  TextView statusText;
  ProgressBar progressBar;
  Button cancelButton;
  Button doneButton;
  ImageView doneImage;
  ImageView errorImage;

  @Override
  public View onCreateView(
      LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
    // Inflate the layout for this fragment
    return inflater.inflate(R.layout.commissioner_commissioning_fragment, container, false);
  }

  public void onViewCreated(@NonNull View view, Bundle savedInstanceState) {
    super.onViewCreated(view, savedInstanceState);

    cancelButton = view.findViewById(R.id.cancel_button);
    doneButton = view.findViewById(R.id.done_button);
    doneImage = view.findViewById(R.id.done_image);
    errorImage = view.findViewById(R.id.error_image);
    statusText = view.findViewById(R.id.status_text);
    progressBar = view.findViewById(R.id.commissioning_progress);
    progressBar.setMin(0);
    progressBar.setMax(100);

    deviceInfo = getArguments().getParcelable(Constants.KEY_DEVICE_INFO);
    networkInfo = getArguments().getParcelable(Constants.KEY_NETWORK_INFO);

    Data arguments =
        new Data.Builder()
            .putString(Constants.KEY_DEVICE_INFO, new Gson().toJson(deviceInfo))
            .putString(Constants.KEY_NETWORK_INFO, new Gson().toJson(networkInfo))
            .build();
    commssionerWorkRequest =
        new OneTimeWorkRequest.Builder(CommissionerWorker.class).setInputData(arguments).build();

    WorkManager.getInstance(getActivity()).enqueue(commssionerWorkRequest);

    WorkManager.getInstance(getActivity())
        .getWorkInfoByIdLiveData(commssionerWorkRequest.getId())
        .observe(getViewLifecycleOwner(), this);

    view.findViewById(R.id.cancel_button)
        .setOnClickListener(
            new View.OnClickListener() {
              @Override
              public void onClick(View view) {
                WorkManager.getInstance(getActivity())
                    .cancelWorkById(commssionerWorkRequest.getId());

                CommissionerActivity commissionerActivity = (CommissionerActivity) getActivity();
                commissionerActivity.finishCommissioning(Activity.RESULT_CANCELED);
              }
            });

    view.findViewById(R.id.done_button)
        .setOnClickListener(
            new View.OnClickListener() {
              @Override
              public void onClick(View view) {
                CommissionerActivity commissionerActivity = (CommissionerActivity) getActivity();
                commissionerActivity.finishCommissioning(Activity.RESULT_OK);
              }
            });
  }

  @Override
  public void onChanged(@Nullable WorkInfo workInfo) {
    if (workInfo != null) {
      if (workInfo.getState().isFinished()) {

        showCommissionDone(
            workInfo.getOutputData().getBoolean(Constants.KEY_SUCCESS, false),
            workInfo.getOutputData().getString(Constants.KEY_COMMISSIONING_STATUS));
      } else {
        showInProgress(workInfo.getProgress().getString(Constants.KEY_COMMISSIONING_STATUS));
      }
    }
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
