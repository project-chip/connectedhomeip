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
import com.matter.casting.core.CastingPlayer;
import com.matter.casting.core.Endpoint;

/**
 * A {@link Fragment} to read the VendorID (from ApplicationBasic cluster) using the TV Casting App.
 */
public class ApplicationBasicReadVendorIDExampleFragment extends Fragment {
  private static final String TAG =
      ApplicationBasicReadVendorIDExampleFragment.class.getSimpleName();

  private final CastingPlayer selectedCastingPlayer;

  private View.OnClickListener readButtonClickListener;

  public ApplicationBasicReadVendorIDExampleFragment(CastingPlayer selectedCastingPlayer) {
    this.selectedCastingPlayer = selectedCastingPlayer;
  }

  /**
   * Use this factory method to create a new instance of this fragment using the provided
   * parameters.
   *
   * @param selectedCastingPlayer CastingPlayer that the casting app connected to
   * @return A new instance of fragment ApplicationBasicReadVendorIDExampleFragment.
   */
  public static ApplicationBasicReadVendorIDExampleFragment newInstance(
      CastingPlayer selectedCastingPlayer) {
    return new ApplicationBasicReadVendorIDExampleFragment(selectedCastingPlayer);
  }

  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
  }

  @Override
  public View onCreateView(
      LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
    this.readButtonClickListener =
        v -> {
          Endpoint endpoint =
              EndpointSelectorExample.selectFirstEndpointByVID(selectedCastingPlayer);
          if (endpoint == null) {
            Log.e(TAG, "No Endpoint with sample vendorID found on CastingPlayer");
            return;
          }

          // get ChipClusters.ApplicationBasic from the endpoint
          ChipClusters.ApplicationBasicCluster cluster =
              endpoint.getCluster(ChipClusters.ApplicationBasicCluster.class);
          if (cluster == null) {
            Log.e(
                TAG,
                "Could not get ApplicationBasicCluster for endpoint with ID: " + endpoint.getId());
            return;
          }

          // call readVendorIDAttribute on the cluster object while passing in a
          // ChipClusters.IntegerAttributeCallback
          cluster.readVendorIDAttribute(
              new ChipClusters.IntegerAttributeCallback() {
                @Override
                public void onSuccess(int value) {
                  Log.d(TAG, "ReadVendorID success. Value: " + value);
                  new Handler(Looper.getMainLooper())
                      .post(
                          () -> {
                            TextView vendorIdResult = getView().findViewById(R.id.vendorIdResult);
                            vendorIdResult.setText("Read VendorID result\nValue: " + value);
                          });
                }

                @Override
                public void onError(Exception error) {
                  Log.e(TAG, "ReadVendorID failure " + error);
                  new Handler(Looper.getMainLooper())
                      .post(
                          () -> {
                            TextView vendorIdResult = getView().findViewById(R.id.vendorIdResult);
                            vendorIdResult.setText("Read VendorID result\nError: " + error);
                          });
                }
              });
        };
    return inflater.inflate(
        R.layout.fragment_matter_application_basic_read_vendor_id, container, false);
  }

  @Override
  public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
    super.onViewCreated(view, savedInstanceState);
    Log.d(TAG, "ApplicationBasicReadVendorIDExampleFragment.onViewCreated called");
    getView().findViewById(R.id.readVendorIdButton).setOnClickListener(readButtonClickListener);
  }
}
