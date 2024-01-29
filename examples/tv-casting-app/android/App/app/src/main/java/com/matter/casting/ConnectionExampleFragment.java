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
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.TextView;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import com.R;
import com.matter.casting.core.CastingPlayer;
import com.matter.casting.support.DeviceTypeStruct;
import com.matter.casting.support.EndpointFilter;
import java.util.ArrayList;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.Executors;

/** A {@link Fragment} to Verify or establish a connection with a selected Casting Player. */
public class ConnectionExampleFragment extends Fragment {
  private static final String TAG = ConnectionExampleFragment.class.getSimpleName();
  // Time (in sec) to keep the commissioning window open, if commissioning is required.
  // Must be >= 3 minutes.
  private static final long MIN_CONNECTION_TIMEOUT_SEC = 3 * 60;
  private final CastingPlayer targetCastingPlayer;
  private TextView connectionFragmentStatusTextView;
  private Button connectionFragmentNextButton;

  public ConnectionExampleFragment(CastingPlayer targetCastingPlayer) {
    Log.i(TAG, "ConnectionExampleFragment() called with target CastingPlayer");
    this.targetCastingPlayer = targetCastingPlayer;
  }

  /**
   * Use this factory method to create a new instance of this fragment using the provided
   * parameters.
   *
   * @return A new instance of fragment ConnectionExampleFragment.
   */
  public static ConnectionExampleFragment newInstance(CastingPlayer castingPlayer) {
    Log.i(TAG, "newInstance() called");
    return new ConnectionExampleFragment(castingPlayer);
  }

  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    Log.i(TAG, "onCreate() called");
  }

  @Override
  public View onCreateView(
      LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
    Log.i(TAG, "onCreateView() called");
    // Inflate the layout for this fragment
    return inflater.inflate(R.layout.fragment_matter_connection_example, container, false);
  }

  @Override
  public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
    super.onViewCreated(view, savedInstanceState);
    Log.i(TAG, "onViewCreated() called");

    connectionFragmentStatusTextView = getView().findViewById(R.id.connectionFragmentStatusText);
    connectionFragmentStatusTextView.setText(
        "Verifying or establishing connection with Casting Player with device name: "
            + targetCastingPlayer.getDeviceName());

    connectionFragmentNextButton = getView().findViewById(R.id.connectionFragmentNextButton);
    Callback callback = (ConnectionExampleFragment.Callback) this.getActivity();
    connectionFragmentNextButton.setOnClickListener(
        v -> {
          Log.i(TAG, "onViewCreated() NEXT clicked. Calling handleConnectionComplete()");
          callback.handleConnectionComplete(targetCastingPlayer);
        });

    Executors.newSingleThreadExecutor()
        .submit(
            () -> {
              Log.d(TAG, "onViewCreated() calling verifyOrEstablishConnection()");

              EndpointFilter desiredEndpointFilter =
                  new EndpointFilter(null, 65521, new ArrayList<DeviceTypeStruct>());
              // The desired commissioning window timeout and EndpointFilter are optional.
              CompletableFuture<Void> completableFuture =
                  targetCastingPlayer.VerifyOrEstablishConnection(
                      MIN_CONNECTION_TIMEOUT_SEC, desiredEndpointFilter);

              Log.d(TAG, "onViewCreated() verifyOrEstablishConnection() called");

              completableFuture
                  .thenRun(
                      () -> {
                        Log.i(
                            TAG,
                            "CompletableFuture.thenRun(), connected to CastingPlayer with deviceId: "
                                + targetCastingPlayer.getDeviceId());
                        getActivity()
                            .runOnUiThread(
                                () -> {
                                  connectionFragmentStatusTextView.setText(
                                      "Connected to Casting Player with device name: "
                                          + targetCastingPlayer.getDeviceName());
                                  connectionFragmentNextButton.setEnabled(true);
                                });
                      })
                  .exceptionally(
                      exc -> {
                        Log.e(
                            TAG,
                            "CompletableFuture.exceptionally(), CastingPLayer connection failed: "
                                + exc.getMessage());
                        getActivity()
                            .runOnUiThread(
                                () -> {
                                  connectionFragmentStatusTextView.setText(
                                      "Casting Player connection failed due to: "
                                          + exc.getMessage());
                                });
                        return null;
                      });
            });
  }

  /** Interface for notifying the host. */
  public interface Callback {
    /** Notifies listener to trigger transition on completion of connection */
    void handleConnectionComplete(CastingPlayer castingPlayer);
  }
}
