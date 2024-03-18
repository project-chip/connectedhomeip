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
import com.matter.casting.support.EndpointFilter;
import com.matter.casting.support.MatterCallback;
import com.matter.casting.support.MatterError;
import java.util.concurrent.Executors;

/** A {@link Fragment} to Verify or establish a connection with a selected Casting Player. */
public class ConnectionExampleFragment extends Fragment {
  private static final String TAG = ConnectionExampleFragment.class.getSimpleName();
  // Time (in sec) to keep the commissioning window open, if commissioning is required.
  // Must be >= 3 minutes.
  private static final long MIN_CONNECTION_TIMEOUT_SEC = 3 * 60;
  private static final Integer DESIRED_ENDPOINT_VENDOR_ID = 65521;
  private final CastingPlayer targetCastingPlayer;
  private TextView connectionFragmentStatusTextView;
  private Button connectionFragmentNextButton;

  public ConnectionExampleFragment(CastingPlayer targetCastingPlayer) {
    Log.i(
        TAG,
        "ConnectionExampleFragment() called with target CastingPlayer ID: "
            + targetCastingPlayer.getDeviceId());
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
            + targetCastingPlayer.getDeviceName()
            + "\nSetup Passcode: "
            + InitializationExample.commissionableDataProvider.get().getSetupPasscode()
            + "\nDiscriminator: "
            + InitializationExample.commissionableDataProvider.get().getDiscriminator());

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
              Log.d(TAG, "onViewCreated() calling CastingPlayer.verifyOrEstablishConnection()");

              EndpointFilter desiredEndpointFilter = new EndpointFilter();
              desiredEndpointFilter.vendorId = DESIRED_ENDPOINT_VENDOR_ID;

              MatterError err =
                  targetCastingPlayer.verifyOrEstablishConnection(
                      MIN_CONNECTION_TIMEOUT_SEC,
                      desiredEndpointFilter,
                      new MatterCallback<Void>() {
                        @Override
                        public void handle(Void v) {
                          Log.i(
                              TAG,
                              "Connected to CastingPlayer with deviceId: "
                                  + targetCastingPlayer.getDeviceId());
                          getActivity()
                              .runOnUiThread(
                                  () -> {
                                    connectionFragmentStatusTextView.setText(
                                        "Connected to Casting Player with device name: "
                                            + targetCastingPlayer.getDeviceName()
                                            + "\n\n");
                                    connectionFragmentNextButton.setEnabled(true);
                                  });
                        }
                      },
                      new MatterCallback<MatterError>() {
                        @Override
                        public void handle(MatterError err) {
                          Log.e(TAG, "CastingPlayer connection failed: " + err);
                          getActivity()
                              .runOnUiThread(
                                  () -> {
                                    connectionFragmentStatusTextView.setText(
                                        "Casting Player connection failed due to: " + err + "\n\n");
                                  });
                        }
                      });

              if (err.hasError()) {
                getActivity()
                    .runOnUiThread(
                        () -> {
                          connectionFragmentStatusTextView.setText(
                              "Casting Player connection failed due to: " + err + "\n\n");
                        });
              }
            });
  }

  /** Interface for notifying the host. */
  public interface Callback {
    /** Notifies listener to trigger transition on completion of connection */
    void handleConnectionComplete(CastingPlayer castingPlayer);
  }
}
