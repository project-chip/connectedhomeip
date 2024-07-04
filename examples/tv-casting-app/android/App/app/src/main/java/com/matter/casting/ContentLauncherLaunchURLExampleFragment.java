/*
 *   Copyright (c) 2024 Project CHIP Authors
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
 */
package com.matter.casting;

import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.EditText;
import android.widget.TextView;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import chip.devicecontroller.ChipClusters;
import com.R;
import com.matter.casting.core.CastingPlayer;
import com.matter.casting.core.Endpoint;
import java.util.Optional;

/** A {@link Fragment} to send Content Launcher LaunchURL command using the TV Casting App. */
public class ContentLauncherLaunchURLExampleFragment extends Fragment {
  private static final String TAG = ContentLauncherLaunchURLExampleFragment.class.getSimpleName();
  private static final Integer SAMPLE_ENDPOINT_VID = 65521;
  private static final int DEFAULT_ENDPOINT_ID_FOR_CGP_FLOW = 1;

  private final CastingPlayer selectedCastingPlayer;
  private final boolean useCommissionerGeneratedPasscode;

  private View.OnClickListener launchUrlButtonClickListener;

  public ContentLauncherLaunchURLExampleFragment(
      CastingPlayer selectedCastingPlayer, boolean useCommissionerGeneratedPasscode) {
    this.selectedCastingPlayer = selectedCastingPlayer;
    this.useCommissionerGeneratedPasscode = useCommissionerGeneratedPasscode;
  }

  /**
   * Use this factory method to create a new instance of this fragment using the provided
   * parameters.
   *
   * @param selectedCastingPlayer CastingPlayer that the casting app connected to.
   * @param useCommissionerGeneratedPasscode Boolean indicating whether this CastingPlayer was
   *     commissioned using the Commissioner-Generated Passcode (CGP) commissioning flow.
   * @return A new instance of fragment ContentLauncherLaunchURLExampleFragment.
   */
  public static ContentLauncherLaunchURLExampleFragment newInstance(
      CastingPlayer selectedCastingPlayer, Boolean useCommissionerGeneratedPasscode) {
    return new ContentLauncherLaunchURLExampleFragment(
        selectedCastingPlayer, useCommissionerGeneratedPasscode);
  }

  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
  }

  @Override
  public View onCreateView(
      LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
    this.launchUrlButtonClickListener =
        v -> {
          Endpoint endpoint;
          if (useCommissionerGeneratedPasscode) {
            // For the example Commissioner-Generated passcode commissioning flow, run demo
            // interactions with the Endpoint with ID DEFAULT_ENDPOINT_ID_FOR_CGP_FLOW = 1. For this
            // flow, we commissioned with the Target Content Application with Vendor ID 1111. Since
            // this target content application does not report its Endpoint's Vendor IDs, we find
            // the desired endpoint based on the Endpoint ID. See
            // connectedhomeip/examples/tv-app/tv-common/include/AppTv.h.
            endpoint =
                EndpointSelectorExample.selectEndpointById(
                    selectedCastingPlayer, DEFAULT_ENDPOINT_ID_FOR_CGP_FLOW);
          } else {
            endpoint = EndpointSelectorExample.selectFirstEndpointByVID(selectedCastingPlayer);
          }
          if (endpoint == null) {
            Log.e(TAG, "No Endpoint with sample vendorID found on CastingPlayer");
            return;
          }

          EditText contentUrlEditText = getView().findViewById(R.id.contentUrlEditText);
          String contentUrl = contentUrlEditText.getText().toString();
          EditText contentDisplayStringEditText =
              getView().findViewById(R.id.contentDisplayStringEditText);
          String contentDisplayString = contentDisplayStringEditText.getText().toString();

          // get ChipClusters.ContentLauncherCluster from the endpoint
          ChipClusters.ContentLauncherCluster cluster =
              endpoint.getCluster(ChipClusters.ContentLauncherCluster.class);
          if (cluster == null) {
            Log.e(
                TAG,
                "Could not get ContentLauncherCluster for endpoint with ID: " + endpoint.getId());
            return;
          }

          // call launchURL on the cluster object while passing in a
          // ChipClusters.ContentLauncherCluster.LauncherResponseCallback and request parameters
          cluster.launchURL(
              new ChipClusters.ContentLauncherCluster.LauncherResponseCallback() {
                @Override
                public void onSuccess(Integer status, Optional<String> data) {
                  Log.d(TAG, "LaunchURL success. Status: " + status + ", Data: " + data);
                  new Handler(Looper.getMainLooper())
                      .post(
                          () -> {
                            TextView launcherResult = getView().findViewById(R.id.launcherResult);
                            launcherResult.setText(
                                "LaunchURL result\nStatus: " + status + ", Data: " + data);
                          });
                }

                @Override
                public void onError(Exception error) {
                  Log.e(TAG, "LaunchURL failure " + error);
                  new Handler(Looper.getMainLooper())
                      .post(
                          () -> {
                            TextView launcherResult = getView().findViewById(R.id.launcherResult);
                            launcherResult.setText("LaunchURL result\nError: " + error);
                          });
                }
              },
              contentUrl,
              Optional.of(contentDisplayString),
              Optional.empty());
        };
    return inflater.inflate(R.layout.fragment_matter_content_launcher_launch_url, container, false);
  }

  @Override
  public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
    super.onViewCreated(view, savedInstanceState);
    Log.d(TAG, "ContentLauncherLaunchURLExampleFragment.onViewCreated called");
    getView().findViewById(R.id.launchUrlButton).setOnClickListener(launchUrlButtonClickListener);
  }
}
