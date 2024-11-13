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
import android.widget.TextView;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import chip.devicecontroller.ChipClusters;
import com.R;
import com.matter.casting.core.CastingApp;
import com.matter.casting.core.CastingPlayer;
import com.matter.casting.core.Endpoint;
import java.util.Date;

/**
 * A {@link Fragment} to subscribe to CurrentState (from MediaPLayback cluster) using the TV Casting
 * App.
 */
public class MediaPlaybackSubscribeToCurrentStateExampleFragment extends Fragment {
  private static final String TAG =
      MediaPlaybackSubscribeToCurrentStateExampleFragment.class.getSimpleName();
  private static final int DEFAULT_ENDPOINT_ID_FOR_CGP_FLOW = 1;

  private final CastingPlayer selectedCastingPlayer;
  private final boolean useCommissionerGeneratedPasscode;

  private View.OnClickListener subscribeButtonClickListener;
  private View.OnClickListener shutdownSubscriptionsButtonClickListener;

  public MediaPlaybackSubscribeToCurrentStateExampleFragment(
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
   * @return A new instance of fragment MediaPlaybackSubscribeToCurrentStateExampleFragment.
   */
  public static MediaPlaybackSubscribeToCurrentStateExampleFragment newInstance(
      CastingPlayer selectedCastingPlayer, boolean useCommissionerGeneratedPasscode) {
    return new MediaPlaybackSubscribeToCurrentStateExampleFragment(
        selectedCastingPlayer, useCommissionerGeneratedPasscode);
  }

  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
  }

  @Override
  public View onCreateView(
      LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
    Endpoint endpoint;
    if (useCommissionerGeneratedPasscode) {
      // For the example Commissioner-Generated passcode commissioning flow, run demo interactions
      // with the Endpoint with ID DEFAULT_ENDPOINT_ID_FOR_CGP_FLOW = 1. For this flow, we
      // commissioned with the Target Content Application with Vendor ID 1111. Since this target
      // content application does not report its Endpoint's Vendor IDs, we find the desired endpoint
      // based on the Endpoint ID. See connectedhomeip/examples/tv-app/tv-common/include/AppTv.h.
      endpoint =
          EndpointSelectorExample.selectEndpointById(
              selectedCastingPlayer, DEFAULT_ENDPOINT_ID_FOR_CGP_FLOW);
    } else {
      endpoint = EndpointSelectorExample.selectFirstEndpointByVID(selectedCastingPlayer);
    }
    if (endpoint == null) {
      Log.e(TAG, "No Endpoint with sample vendorID found on CastingPlayer");
      return inflater.inflate(
          R.layout.fragment_matter_media_playback_subscribe_current_state, container, false);
    }

    this.subscribeButtonClickListener =
        v -> {
          // get ChipClusters.MediaPlaybackCluster from the endpoint
          ChipClusters.MediaPlaybackCluster cluster =
              endpoint.getCluster(ChipClusters.MediaPlaybackCluster.class);
          if (cluster == null) {
            Log.e(
                TAG,
                "Could not get ApplicationBasicCluster for endpoint with ID: " + endpoint.getId());
            return;
          }

          // call subscribeCurrentStateAttribute on the cluster object while passing in a
          // ChipClusters.IntegerAttributeCallback and [0, 1] for min and max interval params
          cluster.subscribeCurrentStateAttribute(
              new ChipClusters.IntegerAttributeCallback() {
                @Override
                public void onSuccess(int value) {
                  Log.d(TAG, "Read success on subscription. Value: " + value + " @ " + new Date());
                  new Handler(Looper.getMainLooper())
                      .post(
                          () -> {
                            TextView currentStateResult =
                                getView().findViewById(R.id.currentStateResult);
                            currentStateResult.setText("Current State result\nValue: " + value);
                          });
                }

                @Override
                public void onError(Exception error) {
                  Log.e(TAG, "Read failure on subscription: " + error);
                  new Handler(Looper.getMainLooper())
                      .post(
                          () -> {
                            TextView currentStateResult =
                                getView().findViewById(R.id.currentStateResult);
                            currentStateResult.setText("Current State result\nError: " + error);
                          });
                }
              },
              0,
              1);
        };

    this.shutdownSubscriptionsButtonClickListener =
        new View.OnClickListener() {
          @Override
          public void onClick(View v) {
            Log.d(TAG, "Shutting down subscriptions");
            CastingApp.getInstance().shutdownAllSubscriptions();
          }
        };

    return inflater.inflate(
        R.layout.fragment_matter_media_playback_subscribe_current_state, container, false);
  }

  @Override
  public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
    super.onViewCreated(view, savedInstanceState);
    Log.d(TAG, "MediaPlaybackSubscribeToCurrentStateExampleFragment.onViewCreated called");
    getView()
        .findViewById(R.id.subscribeToCurrentStateButton)
        .setOnClickListener(subscribeButtonClickListener);
    getView()
        .findViewById(R.id.shutdownSubscriptionsButton)
        .setOnClickListener(shutdownSubscriptionsButtonClickListener);
  }
}
